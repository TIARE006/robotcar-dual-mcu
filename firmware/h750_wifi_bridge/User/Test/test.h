#ifndef __TEST_H
#define __TEST_H

#include "stm32h7xx.h"

#if __has_include("wifi_config.h")
#include "wifi_config.h"
#else
#define ROBOTCAR_WIFI_SSID     "YOUR_2G_WIFI_SSID"
#define ROBOTCAR_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#define macUser_ESP8266_StaSsid        ROBOTCAR_WIFI_SSID
#define macUser_ESP8266_StaPwd         ROBOTCAR_WIFI_PASSWORD
#define macUser_ESP8266_TcpServer_Port "5000"

extern volatile uint8_t ucTcpClosedFlag;

void RobotCar_WifiBridge_Run(void);

#endif
