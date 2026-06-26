# Dual-MCU Mecanum Robot Car

This repository contains the two firmwares and host tools for the STM32 robot car:

- `firmware/f407_chassis`: STM32F407 robot controller for motor PWM, encoder feedback, wheel trims, UART command parsing, and mecanum motion.
- `firmware/h750_wifi_bridge`: STM32H750 + ESP-12F WiFi TCP bridge that forwards Mac/PC commands to the F407 UART.
- `tools/host`: Mac/PC Python clients for direct UART control and WiFi TCP control.
- `docs`: bring-up notes, protocol, and system architecture.

## System Architecture

```text
Mac / PC
  |
  | TCP :5000 over 2.4 GHz WiFi
  v
STM32H750_W5500_MINI + ESP-12F
  |
  | USART1 115200 8N1
  v
STM32F407 Ros Robot Controller V1.2
  |
  | PWM + direction + encoder feedback
  v
JGB37-520 12 V geared motors + mecanum wheels
```

## Build

Install an ARM embedded GCC toolchain first, then run:

```sh
make
```

Build only one target:

```sh
make f407
make h750
```

The H750 project uses `PREFIX ?= arm-none-eabi-`. If your compiler is not on `PATH`, override it:

```sh
make h750 PREFIX=/path/to/arm-none-eabi-
```

## WiFi Credentials

The real WiFi SSID/password are intentionally not committed. Create:

```text
firmware/h750_wifi_bridge/User/Test/wifi_config.h
```

from:

```text
firmware/h750_wifi_bridge/User/Test/wifi_config.example.h
```

## Control

WiFi control:

```sh
python3 tools/host/wifi_car_client.py --host 192.168.1.73 --cmd f
python3 tools/host/wifi_car_client.py --host 192.168.1.73 --cmd stop
```

Direct UART control:

```sh
python3 tools/host/mecanum_keyboard.py --port /dev/cu.usbserial-XXXX
```

See `docs/protocol.md` for the F407 command protocol.
