#!/usr/bin/env python3
import argparse
import ipaddress
import socket
import sys
import threading
import time


PRESETS = {
    "stop": "S\n",
    "f": "V 600 0 0\n",
    "b": "V -600 0 0\n",
    "l": "V 0 -600 0\n",
    "r": "V 0 600 0\n",
    "rl": "V 0 0 -600\n",
    "rr": "V 0 0 600\n",
}


def local_network_candidates(port: int) -> list[str]:
    ips = []
    try:
        probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        probe.connect(("8.8.8.8", 80))
        local_ip = probe.getsockname()[0]
        probe.close()
    except OSError:
        return ips

    network = ipaddress.ip_network(local_ip + "/24", strict=False)
    for addr in network.hosts():
        ip = str(addr)
        if ip != local_ip:
            ips.append(ip)
    return ips


def scan_for_robot(port: int, timeout: float = 0.25) -> str | None:
    found = []
    lock = threading.Lock()
    candidates = local_network_candidates(port)

    def try_one(host: str) -> None:
        if found:
            return
        try:
            with socket.create_connection((host, port), timeout=timeout):
                with lock:
                    if not found:
                        found.append(host)
        except OSError:
            return

    threads = [threading.Thread(target=try_one, args=(host,), daemon=True)
               for host in candidates]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join(timeout + 0.05)
    return found[0] if found else None


def send_and_print(sock: socket.socket, command: str) -> None:
    sock.sendall(command.encode("ascii"))
    sock.settimeout(0.15)
    chunks = []
    while True:
        try:
            data = sock.recv(4096)
        except socket.timeout:
            break
        if not data:
            break
        chunks.append(data)
    if chunks:
        sys.stdout.write(b"".join(chunks).decode("ascii", errors="replace"))
        sys.stdout.flush()


def interactive(sock: socket.socket) -> None:
    print("Connected. Commands:")
    print("  f/b/l/r/rl/rr/stop for presets")
    print("  raw STM32 command, e.g. V 400 0 0 or M 3 700")
    print("  q to quit")
    while True:
        line = input("car> ").strip()
        if not line:
            continue
        if line in {"q", "quit", "exit"}:
            send_and_print(sock, PRESETS["stop"])
            return
        command = PRESETS.get(line, line + "\n")
        send_and_print(sock, command)


def main() -> int:
    parser = argparse.ArgumentParser(description="RobotCar WiFi TCP controller")
    parser.add_argument("--host", default=None)
    parser.add_argument("--port", type=int, default=5000)
    parser.add_argument("--scan", action="store_true",
                        help="scan the local /24 network for the robot TCP server")
    parser.add_argument("--cmd", help="single command or preset to send")
    parser.add_argument("--duration", type=float, default=0.0,
                        help="seconds to wait before sending stop after --cmd")
    args = parser.parse_args()
    host = args.host
    if args.scan or host is None:
        host = scan_for_robot(args.port)
        if host is None:
            raise SystemExit("Robot not found. Pass --host manually if you know its IP.")
        print(f"Robot found at {host}:{args.port}")

    with socket.create_connection((host, args.port), timeout=5.0) as sock:
        if args.cmd:
            command = PRESETS.get(args.cmd, args.cmd + "\n")
            send_and_print(sock, command)
            if args.duration > 0:
                time.sleep(args.duration)
                send_and_print(sock, PRESETS["stop"])
        else:
            interactive(sock)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
