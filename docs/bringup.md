# Bring-Up Notes

## Wiring

H750 to F407 UART bridge:

- H750 `PA9 / USART1_TX` -> F407 `PD6 / USART2_RX`
- H750 `PA10 / USART1_RX` -> F407 `PD5 / USART2_TX`
- H750 `GND` -> F407 `GND`

## WiFi Flow

1. Power the F407 chassis controller and motor power rail.
2. Power the H750 bridge.
3. Wait for the ESP-12F to join the 2.4 GHz WiFi network.
4. Find the H750/ESP IP address from the router or hotspot client list.
5. Send commands from the Mac:

```sh
python3 tools/host/wifi_car_client.py --host <device-ip> --cmd f
python3 tools/host/wifi_car_client.py --host <device-ip> --cmd stop
```

Known tested IP during bring-up: `192.168.1.73`.
