# Architecture

The repository is split by firmware target, with shared host-side tools and documentation at the top level.

This follows the same broad pattern used by mature embedded projects:

- target-specific firmware lives under a dedicated board/firmware folder;
- reusable host tools live outside target firmware;
- documentation describes wiring, protocol, and bring-up independently from generated build output.

## Targets

`firmware/f407_chassis` is the real-time chassis controller. It owns motor timing, encoder reads, wheel mixing, wheel trims, and the serial command protocol.

`firmware/h750_wifi_bridge` is a communication bridge. It owns WiFi association through ESP-12F, TCP server handling, and forwarding received lines to the F407 over USART3.

The H750 does not implement motor control logic. Keeping the motor layer on the F407 makes the WiFi board replaceable later.
