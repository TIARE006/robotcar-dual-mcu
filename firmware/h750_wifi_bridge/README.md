# STM32H750 ESP-12F WiFi Bridge

This firmware makes the STM32H750_W5500_MINI board act as a WiFi TCP bridge:

```text
Mac TCP client -> router/hotspot WiFi -> ESP-12F -> H750 USART1 -> F407 robot controller UART
```

## WiFi

- Mode: STA
- Band: 2.4 GHz
- TCP server port: `5000`
- IP address: assigned by router/hotspot DHCP

Copy `User/Test/wifi_config.example.h` to `User/Test/wifi_config.h` and fill in local credentials. The real `wifi_config.h` is ignored by git.

## Wiring To F407

- H750 `PA9 / USART1_TX` -> F407 `PC11 / USART3_RX`
- H750 `PA10 / USART1_RX` -> F407 `PC10 / USART3_TX`
- H750 `GND` -> F407 `GND`

USART1 is configured as `115200 8N1`.

## Build

```sh
make clean all
```

If the compiler is not on `PATH`:

```sh
make PREFIX=/path/to/arm-none-eabi-
```
