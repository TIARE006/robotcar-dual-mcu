#!/usr/bin/env python3
"""WiFi keyboard controller for the dual-MCU mecanum robot.

The controller sends velocity vectors periodically while a non-zero command is
active. The F407 watchdog stops the chassis if packets stop arriving.
"""

from __future__ import annotations

import argparse
import select
import socket
import sys
import termios
import time
import tty


DEFAULT_HOST = "192.168.1.73"
DEFAULT_PORT = 5000
DEFAULT_SPEED = 300
DEFAULT_RATE_HZ = 20.0


def key_to_velocity(key: str, speed: int) -> tuple[int, int, int] | None:
    key = key.lower()
    if key == "w":
        return speed, 0, 0
    if key == "s":
        return -speed, 0, 0
    if key == "a":
        return 0, -speed, 0
    if key == "d":
        return 0, speed, 0
    if key == "q":
        return 0, 0, -speed
    if key == "e":
        return 0, 0, speed
    if key == " ":
        return 0, 0, 0
    return None


def velocity_command(velocity: tuple[int, int, int]) -> str:
    x, y, z = velocity
    if velocity == (0, 0, 0):
        return "S\n"
    return f"V {x} {y} {z}\n"


def send_command(sock: socket.socket, command: str) -> None:
    sock.sendall(command.encode("ascii"))


def drain_socket(sock: socket.socket) -> None:
    while True:
        readable, _, _ = select.select([sock], [], [], 0)
        if sock not in readable:
            return
        try:
            data = sock.recv(4096)
        except BlockingIOError:
            return
        if not data:
            return
        sys.stdout.write(data.decode("ascii", errors="replace"))
        sys.stdout.flush()


def print_help(host: str, port: int, speed: int, rate_hz: float) -> None:
    print(f"Connected to {host}:{port}")
    print(f"speed={speed}, rate={rate_hz:.1f} Hz")
    print("Controls:")
    print("  W/S  forward/back")
    print("  A/D  left/right")
    print("  Q/E  rotate left/right")
    print("  Space stop")
    print("  +/-  speed up/down")
    print("  X    stop and quit")
    print("Tap a key to change velocity; the current velocity is resent periodically.")


def run(host: str, port: int, speed: int, rate_hz: float) -> None:
    stdin_fd = sys.stdin.fileno()
    old_tty = termios.tcgetattr(stdin_fd)
    period_s = 1.0 / rate_hz
    velocity = (0, 0, 0)
    last_send = 0.0

    with socket.create_connection((host, port), timeout=5.0) as sock:
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        sock.setblocking(False)
        print_help(host, port, speed, rate_hz)

        try:
            tty.setcbreak(stdin_fd)
            send_command(sock, "S\n")
            last_send = time.monotonic()

            while True:
                readable, _, _ = select.select([stdin_fd, sock], [], [], 0.02)

                if sock in readable:
                    drain_socket(sock)

                if stdin_fd in readable:
                    key = sys.stdin.read(1)
                    if key.lower() == "x" or key == "\x03":
                        break
                    if key in {"+", "="}:
                        speed = min(speed + 50, 1000)
                        print(f"speed={speed}")
                    elif key in {"-", "_"}:
                        speed = max(speed - 50, 50)
                        print(f"speed={speed}")
                    else:
                        new_velocity = key_to_velocity(key, speed)
                        if new_velocity is not None:
                            velocity = new_velocity
                            command = velocity_command(velocity)
                            send_command(sock, command)
                            last_send = time.monotonic()
                            print(command.strip())

                now = time.monotonic()
                if velocity != (0, 0, 0) and now - last_send >= period_s:
                    send_command(sock, velocity_command(velocity))
                    last_send = now
        finally:
            try:
                send_command(sock, "S\n")
                time.sleep(0.05)
                send_command(sock, "S\n")
            finally:
                termios.tcsetattr(stdin_fd, termios.TCSANOW, old_tty)
                print("\nStopped.")


def main() -> int:
    parser = argparse.ArgumentParser(description="WiFi keyboard control for the mecanum robot")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--speed", type=int, default=DEFAULT_SPEED)
    parser.add_argument("--rate", type=float, default=DEFAULT_RATE_HZ,
                        help="velocity resend rate in Hz")
    args = parser.parse_args()

    if args.rate <= 0:
        raise SystemExit("--rate must be positive")

    run(args.host, args.port, args.speed, args.rate)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
