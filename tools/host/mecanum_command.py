#!/usr/bin/env python3
"""Send one command to the mecanum firmware and print the response."""

import argparse
import glob
import os
import select
import termios
import time


DEFAULT_PORT = "/dev/cu.usbserial-59170110321"
DEFAULT_BAUD = 115200

BAUD_MAP = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
}


def auto_port() -> str:
    ports = sorted(glob.glob("/dev/cu.usbserial-*") + glob.glob("/dev/cu.wchusbserial-*"))
    if len(ports) >= 2:
        for port in ports:
            if port != DEFAULT_PORT:
                return port
    if ports:
        return ports[0]
    return DEFAULT_PORT


def open_serial(path: str, baud: int) -> int:
    if baud not in BAUD_MAP:
        raise ValueError(f"unsupported baud rate: {baud}")

    fd = os.open(path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    attrs = termios.tcgetattr(fd)
    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    attrs[3] = 0
    attrs[4] = BAUD_MAP[baud]
    attrs[5] = BAUD_MAP[baud]
    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    return fd


def read_response(fd: int, timeout_s: float) -> str:
    deadline = time.monotonic() + timeout_s
    chunks: list[bytes] = []

    while time.monotonic() < deadline:
        readable, _, _ = select.select([fd], [], [], 0.05)
        if fd not in readable:
            continue

        try:
            data = os.read(fd, 1024)
        except BlockingIOError:
            continue

        if data:
            chunks.append(data)
            if b"\n" in data:
                break

    return b"".join(chunks).decode("utf-8", errors="replace")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", nargs="+", help='command text, e.g. "G" or "T 1 950"')
    parser.add_argument("--port", default=None)
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD)
    parser.add_argument("--timeout", type=float, default=0.5)
    args = parser.parse_args()

    command = " ".join(args.command).strip()
    if not command:
        raise SystemExit("empty command")

    fd = open_serial(args.port or auto_port(), args.baud)
    try:
        os.write(fd, (command + "\n").encode("ascii"))
        response = read_response(fd, args.timeout)
        if response:
            print(response, end="")
    finally:
        os.close(fd)


if __name__ == "__main__":
    main()
