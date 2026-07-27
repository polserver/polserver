"""A deliberately deaf HTTP client, for the webserver slow-reader regression test.

The test needs a peer that requests a page and then stops reading; an eScript client
cannot do it, because POL's aux reader thread always drains its socket. Started
alongside the shard by cmake/core_tests_start.cmake, like testsuite/smtpd/smtpd.py.

Control protocol (line based, spoken by testpkgs/slowreader/deafctl.src):

    STALL <hold_secs> <kb> <chunks>   ->  ARMED       once the request is on the wire
                                      ->  BYTES <n>   after it drains the response
    DEAFLISTEN <port> <hold_secs>     ->  LISTENING   ready for os::OpenConnection()

STALL covers the webserver; DEAFLISTEN covers the aux service, where the shard is the one
connecting out and this process is the peer that stops reading.

Two details the test depends on: the small SO_RCVBUF, which makes the server's socket
buffer fill after ~64 KB, and `chunks` -- a body written in one large send is accepted
whole by the OS and never stalls, which would leave the test passing either way.

To notice the shard is gone it binds the webserver port instead of connecting to it: a
connection resets the listener's idle timer, which stops config/www.cfg from being
hot-reloaded and breaks test_www_config.
"""
import socket
import threading
import time

CONTROL_PORT = 5011
WEB_PORT = 5006
WEB_AUTH = "Basic cG9sY29yZTp0ZXN0"  # polcore:test, matches pol.cfg WebServerPassword
HARD_DEADLINE_SECS = 540  # backstop if the shard never appears; ctest allows 600s

logfile = open("deafclient.log", "w", encoding="utf-8", buffering=1)


def log(message):
    logfile.write(message + "\n")


def webserver_port_free():
    """True once nothing is listening on the webserver port, i.e. the shard is gone."""
    probe = socket.socket()  # bind, never connect -- see module docstring
    try:
        probe.bind(("127.0.0.1", WEB_PORT))
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


def stall(control, hold_secs, kb, chunks):
    """Request a large page, read nothing for hold_secs, then drain and count."""
    deaf = socket.socket()
    deaf.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 2048)
    deaf.settimeout(60)
    deaf.connect(("127.0.0.1", WEB_PORT))
    request = (
        f"GET /pkg/webserver/bigpage.ecl?kb={kb}&chunks={chunks} HTTP/1.1\r\n"
        f"Host: 127.0.0.1\r\n"
        f"Authorization: {WEB_AUTH}\r\n"
        f"\r\n"
    )
    deaf.sendall(request.encode())
    control.sendall(b"ARMED\n")
    log(f"armed: requested kb={kb} chunks={chunks}, holding {hold_secs}s without reading")

    time.sleep(hold_secs)

    received = 0
    try:
        while True:
            data = deaf.recv(262144)
            if not data:
                break
            received += len(data)
    except OSError as ex:
        log(f"drain stopped: {type(ex).__name__}: {ex}")
    deaf.close()

    log(f"drained {received} bytes")
    control.sendall(f"BYTES {received}\n".encode())


def deaf_listen(control, port, hold_secs):
    """Accept one connection from the shard and never read it, so its transmit stalls."""
    listener = socket.socket()
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # small receive window, same reason as the STALL case -- it has to be set on the
    # listening socket: the accepted socket inherits it, and on Linux/macOS the window is
    # negotiated during the handshake, so setting it after accept() comes too late to
    # shrink what was already advertised and nothing ever stalls
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 2048)
    listener.bind(("127.0.0.1", port))
    listener.listen(1)
    listener.settimeout(60)
    control.sendall(b"LISTENING\n")
    log(f"deaf listener on {port}, will hold {hold_secs}s without reading")

    try:
        peer, _ = listener.accept()
    except OSError as ex:
        log(f"nothing connected to {port}: {type(ex).__name__}")
        listener.close()
        return

    time.sleep(hold_secs)

    peer.close()
    listener.close()
    log(f"deaf listener on {port} done")


def handle(control):
    try:
        while True:
            line = read_line(control)
            if not line:
                return
            log(f"command: {line}")
            words = line.split()
            if words[0] == "STALL":
                stall(control, float(words[1]), int(words[2]), int(words[3]))
            elif words[0] == "DEAFLISTEN":
                deaf_listen(control, int(words[1]), float(words[2]))
            else:
                log(f"ignoring unknown command: {line}")
    except OSError as ex:
        log(f"control connection lost: {type(ex).__name__}: {ex}")
    finally:
        control.close()


def main():
    listener = socket.socket()
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind(("127.0.0.1", CONTROL_PORT))
    listener.listen(4)
    listener.settimeout(1.0)
    log(f"deaf client helper listening on 127.0.0.1:{CONTROL_PORT}")

    started = time.monotonic()
    shard_seen = False
    while time.monotonic() - started < HARD_DEADLINE_SECS:
        try:
            control, _ = listener.accept()
        except socket.timeout:
            if not webserver_port_free():
                shard_seen = True
            elif shard_seen and threading.active_count() == 1:  # no stall in progress
                log("shard gone, exiting")
                return
            continue
        threading.Thread(target=handle, args=(control,), daemon=True).start()

    log("hard deadline reached, exiting")


if __name__ == "__main__":
    main()
