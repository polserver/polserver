"""A raw TCP peer for the game port, for packet-level regression tests (issue #452).

The test needs to put exact bytes on the wire ahead of any login. eScript cannot:
its strings are UTF-8 text, so `CChrZ()` runs the array through utf16to8 +
sanitizeUnicode (bscript/bstring.cpp:1250) and any byte >= 0x80 comes out as two,
while a Packet handed to `transmit()` is stringified as ASCII hex
(pol/packetscrobj.cpp:527). So the bytes are composed in the test script as a hex
string -- which is also how the issue's dumps are written -- and this process turns
them back into bytes.

Started alongside the shard by cmake/core_tests_start.cmake, like deafclient.py.

Control protocol (line based, spoken by testpkgs/rawpacket/rawctl.src):

    SEND <hex> [<hex> ...]   ->  RESULT <verdict> <ms> <bytes-read-back>

Each hex word is written as its own send() with a short gap, so a test can put a
packet boundary where it wants one; a single word is one write. The connection is
opened fresh per SEND and closed at the end.

<verdict> is one of:
    CLOSED    the shard closed the connection    (it processed the bytes and reacted)
    OPEN      still connected when we gave up    (bytes accepted, no disconnect)
    REFUSED   could not connect at all

The distinction matters more than it looks. A test that merely fires bytes and
checks the shard is alive passes just as well when nothing ever connected -- the
failure mode specs/testsuite/05-harness-traps.md is about. CLOSED is positive
evidence that the core read the bytes and decided something about them.

Note the 4-byte crypt seed: a fresh connection is in RECV_STATE_CRYPTSEED_WAIT, so
the first four bytes are consumed as the seed and never parsed as a message
(pol/network/clientthread.cpp). The seed is the caller's business -- it is just the
first hex word -- but a test that forgets it is testing an offset it did not intend.
"""
import os
import socket
import threading
import time

# Ports the test shard and its helpers already own, so a new one has to avoid all of
# them: 5001 debugger, 5002 DAP, 5003 game, 5006 webserver, 5011 deafclient control,
# 5012 the aux listener testpkgs/slowreader asks deafclient to open, 50000 testclient
# control. Note 5012 is bound late (only while that test runs), so a clash with it does
# not show up as a startup error -- it shows up as slowreader failing to get a listener.
CONTROL_PORT = 5013
GAME_PORT = 5003  # testsuite/pol/config/servers.cfg, and testclient.cfg
HARD_DEADLINE_SECS = 540  # backstop if the shard never appears; ctest allows 600s

# How long to wait for the shard to drop the connection before calling it OPEN.
# Generous: the verdict is about whether it *ever* closes, not how fast.
CLOSE_WAIT_SECS = 5.0
# Gap between hex words, so consecutive sends land as separate reads often enough
# to exercise the partial-message path. Not a guarantee -- TCP may coalesce them.
INTER_SEND_GAP = 0.05

logfile = open("rawpeer.log", "w", encoding="utf-8", buffering=1)
START = time.monotonic()


def log(message):
    logfile.write(f"[{time.monotonic() - START:7.2f}s] {message}\n")


def release_stdout():
    """Let POL's stdin reach EOF immediately -- see deafclient.py for the full why."""
    devnull = os.open(os.devnull, os.O_WRONLY)
    os.dup2(devnull, 1)
    os.close(devnull)


def game_port_free():
    """True once nothing is listening on the game port, i.e. the shard is gone.

    The bind must use the wildcard address, not 127.0.0.1: POL's game listener binds
    INADDR_ANY (clib/network/wnsckt.cpp, listen() with loopback_only false), and Windows
    lets a socket bind a specific address while another holds the wildcard on the same
    port. A loopback probe therefore succeeds every time, the shard is never seen, and
    this process runs to its hard deadline instead of exiting with the shard -- which
    pins shard_test_1 at 540s no matter what POLCORE_TEST_FILTER selects. The webserver
    probe in deafclient.py is loopback-bound on both ends, so it does not have this
    problem. Binding rather than connecting is still deliberate: a connection to the game
    port would make POL build a client for it once a second.
    """
    probe = socket.socket()
    try:
        probe.bind(("0.0.0.0", GAME_PORT))
        return True
    except OSError:
        return False
    finally:
        probe.close()


def read_line(sock):
    data = b""
    while not data.endswith(b"\n"):
        chunk = sock.recv(1)
        if not chunk:
            break
        data += chunk
    return data.decode(errors="replace").strip()


def send_raw(words):
    """Connect, write each hex word, then wait to see whether the shard hangs up.

    Returns (verdict, elapsed_ms, bytes_read_back).
    """
    payloads = [bytes.fromhex(w) for w in words]
    total = sum(len(p) for p in payloads)

    peer = socket.socket()
    peer.settimeout(10)
    started = time.monotonic()
    try:
        peer.connect(("127.0.0.1", GAME_PORT))
    except OSError as ex:
        log(f"connect failed: {type(ex).__name__}: {ex}")
        peer.close()
        return "REFUSED", 0, 0

    try:
        for i, payload in enumerate(payloads):
            peer.sendall(payload)
            if i + 1 < len(payloads):
                time.sleep(INTER_SEND_GAP)
    except OSError as ex:
        # A reset while writing still means the shard read and rejected the bytes.
        log(f"send stopped early: {type(ex).__name__}: {ex}")
        elapsed = (time.monotonic() - started) * 1000
        peer.close()
        return "CLOSED", int(elapsed), 0

    log(f"sent {total} bytes in {len(payloads)} write(s), waiting for close")

    # Read until EOF or the wait budget runs out. Anything the shard sends back
    # (a login-error packet, say) is counted but not interpreted -- what the test
    # asserts on is whether the connection ends, not what came down it.
    peer.settimeout(CLOSE_WAIT_SECS)
    read_back = 0
    verdict = "OPEN"
    deadline = time.monotonic() + CLOSE_WAIT_SECS
    try:
        while time.monotonic() < deadline:
            data = peer.recv(4096)
            if not data:
                verdict = "CLOSED"
                break
            read_back += len(data)
    except socket.timeout:
        pass
    except OSError as ex:
        log(f"recv ended: {type(ex).__name__}: {ex}")
        verdict = "CLOSED"

    elapsed = (time.monotonic() - started) * 1000
    peer.close()
    log(f"verdict {verdict} after {elapsed:.0f}ms, read back {read_back} bytes")
    return verdict, int(elapsed), read_back


def handle(control):
    try:
        while True:
            line = read_line(control)
            if not line:
                return
            log(f"command: {line}")
            words = line.split()
            if words[0] == "SEND" and len(words) > 1:
                verdict, ms, read_back = send_raw(words[1:])
                control.sendall(f"RESULT {verdict} {ms} {read_back}\n".encode())
            else:
                log(f"ignoring unknown command: {line}")
                control.sendall(b"RESULT REFUSED 0 0\n")
    except (OSError, ValueError) as ex:
        log(f"control connection lost: {type(ex).__name__}: {ex}")
    finally:
        control.close()


def main():
    release_stdout()
    listener = socket.socket()
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind(("127.0.0.1", CONTROL_PORT))
    listener.listen(4)
    listener.settimeout(1.0)
    log(f"raw peer helper listening on 127.0.0.1:{CONTROL_PORT}")

    started = time.monotonic()
    shard_seen = False
    while time.monotonic() - started < HARD_DEADLINE_SECS:
        try:
            control, _ = listener.accept()
        except socket.timeout:
            if not game_port_free():
                shard_seen = True
            elif shard_seen and threading.active_count() == 1:
                log("shard gone, exiting")
                return
            continue
        threading.Thread(target=handle, args=(control,), daemon=True).start()

    log("hard deadline reached, exiting")


if __name__ == "__main__":
    main()
