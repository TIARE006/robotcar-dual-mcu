# STM32F407 Chassis Controller

This firmware runs on the Ros Robot Controller V1.2 STM32F407 board. It owns motor PWM, direction control, encoder feedback, wheel trims, and the UART command protocol used by the Mac direct client or by the H750 WiFi bridge.

## Layout

```text
include/bsp/             board-specific pins and hardware mappings
include/drivers/chassis/ mecanum chassis API
include/drivers/encoder/ encoder API
include/drivers/motor/   motor driver API
include/drivers/uart/    UART logging/control API
src/app/                 application logic and serial command parser
src/bsp/                 board support implementation
src/drivers/chassis/     mecanum kinematics and wheel trim logic
src/drivers/encoder/     encoder implementation
src/drivers/motor/       PWM/direction motor implementation
src/main.c               startup handoff
```

## Build

```sh
make
```

Outputs:

- `build/ros_robot_controller.elf`
- `build/ros_robot_controller.bin`

## Flash

ST-Link:

```sh
make flash
```

UART ROM bootloader:

```sh
make flash-uart UART_PORT=/dev/cu.usbserial-XXXX
```

## Control

Serial settings: `115200 8N1`.

See `../../docs/protocol.md` for the command protocol. In this chassis convention:

- `+x` is forward
- `+y` is right
- `+z` is clockwise/right rotation

Current wheel trim values from bring-up:

```text
TRIM 890 920 1270 1040
```
