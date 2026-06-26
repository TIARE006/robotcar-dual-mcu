#!/usr/bin/env python3
"""Keyboard controller for the ROS Robot Controller mecanum firmware.

Protocol over UART:
    V <x> <y> <z>\n
    S\n

Coordinate convention:
    +x forward, -x backward
    +y right, -y left
    +z rotate right, -z rotate left
"""

import argparse
import glob
import os
import select
import sys
import termios
import time
import tty


DEFAULT_PORT = "/dev/cu.usbserial-59170110321"
DEFAULT_BAUD = 115200
DEFAULT_SPEED = 350
SEND_PERIOD_S = 0.10


BAUD_MAP = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
}


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


def auto_port() -> str:
    ports = sorted(glob.glob("/dev/cu.usbserial-*") + glob.glob("/dev/cu.wchusbserial-*"))
    if len(ports) >= 2:
        for port in ports:
            if port != DEFAULT_PORT:
                return port
    if ports:
        return ports[0]
    return DEFAULT_PORT


def write_cmd(fd: int, text: str) -> None:
    os.write(fd, text.encode("ascii"))


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


def run(port: str, baud: int, speed: int) -> None:
    serial_fd = open_serial(port, baud)
    stdin_fd = sys.stdin.fileno()
    old_tty = termios.tcgetattr(stdin_fd)
    last_send = 0.0
    current = (0, 0, 0)

    print(f"Connected to {port} @ {baud}")
    print("W/S forward/back, A/D left/right, Q/E rotate, Space stop, X quit")
    print("Hold-style terminal input is not available here; tap keys to update velocity.")

    try:
        tty.setcbreak(stdin_fd)
        write_cmd(serial_fd, "S\n")

        while True:
            readable, _, _ = select.select([stdin_fd, serial_fd], [], [], 0.02)
            if stdin_fd in readable:
                key = os.read(stdin_fd, 1).decode("utf-8", errors="ignore")
                if key.lower() == "x" or key == "\x03":
                    break

                velocity = key_to_velocity(key, speed)
                if velocity is not None:
                    current = velocity
                    x, y, z = current
                    if current == (0, 0, 0):
                        write_cmd(serial_fd, "S\n")
                        print("STOP")
                    else:
                        write_cmd(serial_fd, f"V {x} {y} {z}\n")
                        print(f"V x={x} y={y} z={z}")
                    last_send = time.monotonic()

            if serial_fd in readable:
                try:
                    data = os.read(serial_fd, 1024)
                    if data:
                        sys.stdout.write(data.decode("utf-8", errors="replace"))
                        sys.stdout.flush()
                except BlockingIOError:
                    pass

            now = time.monotonic()
            if current != (0, 0, 0) and now - last_send >= SEND_PERIOD_S:
                x, y, z = current
                write_cmd(serial_fd, f"V {x} {y} {z}\n")
                last_send = now
    finally:
        try:
            write_cmd(serial_fd, "S\n")
        finally:
            termios.tcsetattr(stdin_fd, termios.TCSANOW, old_tty)
            os.close(serial_fd)
            print("\nStopped.")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default=None)
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD)
    parser.add_argument("--speed", type=int, default=DEFAULT_SPEED)
    args = parser.parse_args()
    run(args.port or auto_port(), args.baud, args.speed)


if __name__ == "__main__":
    main()
