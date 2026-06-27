# Bring-Up Notes

## Wiring

H750 to F407 UART bridge:

- Logic level is 3.3 V TTL. Do not connect 5 V to UART signal pins.
- Current H750 firmware uses `USART3` on the H750 `P2` expansion header:
  - H750 `P2 pin 3 / PD8 / USART3_TX` -> F407 `H1 pin 20 / PC11 / USART3_RX`
  - H750 `P2 pin 2 / PD9 / USART3_RX` <- F407 `H1 pin 19 / PC10 / USART3_TX`
  - H750 `GND` -> F407 `GND`
- Current H750-F407 UART speed is `115200 8N1`.
- The F407 `BT/UART2` header is `PD5/PD6`, not `PC10/PC11`; do not use it with the current `USART3` chassis firmware.
- If reverting the H750 firmware to `USART1`, the schematic exposes:
  - H750 `DAPLINK pin 1 / PA9 / USART1_TX`
  - H750 `DAPLINK pin 3 / PA10 / USART1_RX`
  - H750 `DAPLINK pin 5 or 6 / GND`

## WiFi Flow

1. Power the F407 chassis controller and motor power rail.
2. Power the H750 bridge.
3. Wait for the ESP-12F to join the 2.4 GHz WiFi network.
4. Find the H750/ESP IP address from the router or hotspot client list.
5. Send commands from the Mac:

```sh
python3 tools/host/wifi_keyboard.py --host <device-ip>
python3 tools/host/wifi_car_client.py --host <device-ip> --cmd f
python3 tools/host/wifi_car_client.py --host <device-ip> --cmd stop
```

Known tested IP during bring-up: `192.168.1.73`.
