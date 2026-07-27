#!/usr/bin/env python3
"""Measure UO login-handshake service latency against a running POL shard.

Step 0 of specs/sockets/12-batched-login-poll.md: the claim under test is that
UoClientListener::run() discovers readiness for the listen socket and every
pending login socket with 1+N separate poll() calls, so a login packet waits
up to (listener timeout) + N ms before anyone looks at it.

This speaks only enough of the protocol to time one round trip:

  - "parked" clients connect and send the 4-byte crypt seed, then sit idle.
    They stay in the listener's login_clients list (until LoginServerTimeout,
    10 min by default), inflating N without generating traffic.
  - the "probe" sends a 0x80 account-login for an account that does not exist
    and times the 0x82 reply. That crosses exactly the same sweep a real
    0xA8 server-list would, needs no valid account, and mutates no shard
    state: loginserver_login() rejects an unknown name before touching MD5,
    account records or scripts (pol-core/pol/login.cpp:150-156).

Run against a manual test shard (bin/Release/pol.exe with cwd build/coretest
and POLCORE_TEST_RUN unset), which listens on port 5003 with Encryption=none.

Usage:
  python loginlatency.py --parked 0 --samples 30
  python loginlatency.py --sweep 0,10,25,50 --samples 20
"""

import argparse
import socket
import statistics
import sys
import time

# The client's own IP, the classic seed value. Must not be 0xffffffff (UOKR
# probe) or start with 0xef (6.0.5.0+ seed), which take other branches in
# process_data(); see clientthread.cpp:451-470.
SEED = b"\x7f\x00\x00\x01"

LOGIN_PKT_LEN = 62  # PKTIN_80: u8 msgtype + char[30] name + char[30] pass + u8


def login_packet(name: str, password: str) -> bytes:
    """Build a 0x80 account-login packet (pol-core/pol/network/pktin.h:257)."""
    pkt = bytearray(LOGIN_PKT_LEN)
    pkt[0] = 0x80
    n = name.encode("ascii")[:29]
    p = password.encode("ascii")[:29]
    pkt[1 : 1 + len(n)] = n
    pkt[31 : 31 + len(p)] = p
    return bytes(pkt)


def connect(host: str, port: int, timeout: float) -> socket.socket:
    s = socket.create_connection((host, port), timeout=timeout)
    # Without this the probe measures Nagle, not POL.
    s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    return s


def count_accepts(pol_log: str) -> int:
    """Number of 'connected from' lines POL has logged (uolisten.cpp:88-97)."""
    if not pol_log:
        return 0
    try:
        with open(pol_log, "r", encoding="utf-8", errors="replace") as fh:
            return sum(1 for line in fh if "connected from" in line)
    except OSError:
        return 0


def park(host: str, port: int, count: int, args):
    """Open `count` sockets that sit in login_clients doing nothing.

    Returns (sockets, admit_seconds). The listener accepts at most one
    connection per sweep (uolisten.cpp:121 -- GetConnection does a single
    accept), so a batch of N needs N sweeps to be fully admitted, and the
    sweep itself grows with N. A fixed sleep is not enough: probing while
    the set is still filling measures a moving target.

    With --pol-log we wait for N new accept lines, which is exact and also
    yields the accept-drain time. Without it, fall back to a scaled sleep.
    """
    if not count:
        return [], 0.0

    before = count_accepts(args.pol_log)
    parked = []
    t0 = time.perf_counter()
    for _ in range(count):
        s = connect(host, port, args.timeout)
        s.sendall(SEED)
        parked.append(s)

    if args.pol_log:
        deadline = t0 + args.admit_timeout
        while time.perf_counter() < deadline:
            if count_accepts(args.pol_log) - before >= count:
                break
            time.sleep(0.05)
        else:
            print(f"  WARNING: only {count_accepts(args.pol_log) - before} of "
                  f"{count} parked clients admitted within "
                  f"{args.admit_timeout}s; results are unreliable")
    else:
        time.sleep(args.settle + count * args.settle_per_parked)

    admit = time.perf_counter() - t0
    # Let the seeds be consumed too: admission only means accept() ran.
    time.sleep(args.settle)
    return parked, admit


def probe(host: str, port: int, pkt: bytes, timeout: float, settle: float,
          isolate: bool):
    """Return milliseconds from sending 0x80 to the first response byte.

    isolate=True first sends the seed alone and waits for the shard to accept
    the connection and consume it, so the timed window is one sweep. Otherwise
    the seed and 0x80 go together and the window also covers the accept plus
    the extra sweep the seed costs -- process_data() returns after consuming
    the crypt seed (clientthread.cpp:473-475 falling through to :604), so the
    0x80 is necessarily handled on a later sweep either way.
    """
    s = connect(host, port, timeout)
    try:
        if isolate:
            s.sendall(SEED)
            time.sleep(settle)
            payload = pkt
        else:
            payload = SEED + pkt

        s.settimeout(timeout)
        t0 = time.perf_counter()
        s.sendall(payload)
        data = s.recv(64)
        t1 = time.perf_counter()

        if not data:
            return None, "connection closed with no reply"
        if data[0] != 0x82:
            return None, f"unexpected reply {data[0]:#04x} (expected 0x82)"
        return (t1 - t0) * 1000.0, None
    except socket.timeout:
        return None, "timed out waiting for reply"
    finally:
        s.close()


def pct(values, p):
    """Nearest-rank percentile; avoids numpy for a script this small."""
    if not values:
        return float("nan")
    ordered = sorted(values)
    k = max(0, min(len(ordered) - 1, int(round(p / 100.0 * len(ordered))) - 1))
    return ordered[k]


def run_one(args, parked_count):
    parked = []
    try:
        parked, admit = park(args.host, args.port, parked_count, args)
        pkt = login_packet(args.account, args.password)

        samples, errors = [], []
        for i in range(args.samples):
            ms, err = probe(args.host, args.port, pkt, args.timeout,
                            args.settle, not args.full)
            if err:
                errors.append(f"sample {i}: {err}")
            else:
                samples.append(ms)
            time.sleep(args.gap)

        return samples, errors, admit
    finally:
        for s in parked:
            try:
                s.close()
            except OSError:
                pass


def main():
    ap = argparse.ArgumentParser(
        description="Measure POL login round-trip latency",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__.split("Usage:")[-1])
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=5003,
                    help="login listener port (test shard: 5003)")
    ap.add_argument("--parked", type=int, default=0,
                    help="idle login clients to hold open during the run")
    ap.add_argument("--sweep", default=None,
                    help="comma-separated parked counts, e.g. 0,10,25,50")
    ap.add_argument("--samples", type=int, default=30)
    ap.add_argument("--gap", type=float, default=0.05,
                    help="seconds between probes; keep above 0 so consecutive "
                         "probes do not share a sweep")
    ap.add_argument("--settle", type=float, default=1.5,
                    help="seconds to wait for the shard to accept and consume "
                         "seeds before timing")
    ap.add_argument("--settle-per-parked", type=float, default=1.0,
                    help="extra settle seconds per parked client when "
                         "--pol-log is not given (accept is 1 per sweep)")
    ap.add_argument("--pol-log", default=None,
                    help="path to the shard's pol.log; makes parking wait for "
                         "actual admission instead of guessing, and reports "
                         "accept-drain time (e.g. build/coretest/log/pol.log)")
    ap.add_argument("--admit-timeout", type=float, default=180.0,
                    help="max seconds to wait for parked clients to be admitted")
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--account", default="__probe_no_such_account__",
                    help="must NOT exist on the shard: the run relies on the "
                         "unknown-account rejection path")
    ap.add_argument("--password", default="x")
    ap.add_argument("--full", action="store_true",
                    help="time seed+0x80 together (includes accept latency) "
                         "instead of isolating a single sweep")
    args = ap.parse_args()

    counts = ([int(x) for x in args.sweep.split(",")] if args.sweep
              else [args.parked])

    mode = "seed+0x80 (accept included)" if args.full else "0x80 only (one sweep)"
    print(f"target   {args.host}:{args.port}")
    print(f"mode     {mode}")
    print(f"samples  {args.samples} per point\n")
    print(f"{'parked':>7} {'n':>4} {'min':>9} {'median':>9} {'p90':>9} "
          f"{'max':>9} {'admit':>9}")
    print("-" * 62)

    rows = []
    for n in counts:
        samples, errors, admit = run_one(args, n)
        if not samples:
            print(f"{n:>7} {0:>4}  no successful samples")
            for e in errors[:3]:
                print(f"          {e}")
            continue
        row = (n, len(samples), min(samples), statistics.median(samples),
               pct(samples, 90), max(samples), admit)
        rows.append(row)
        print(f"{row[0]:>7} {row[1]:>4} {row[2]:>8.2f}ms {row[3]:>8.2f}ms "
              f"{row[4]:>8.2f}ms {row[5]:>8.2f}ms {row[6]:>8.2f}s")
        for e in errors[:3]:
            print(f"          note: {e}")

    if len(rows) > 1:
        base, last = rows[0], rows[-1]
        dn = last[0] - base[0]
        if dn > 0:
            slope = (last[3] - base[3]) / dn
            print(f"\nmedian slope {slope:+.3f} ms per parked client "
                  f"({base[0]} -> {last[0]})")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.exit(130)
