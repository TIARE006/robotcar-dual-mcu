#include "./Test/test.h"
#include "./ESP8266/bsp_esp8266.h"
#include "./SysTick/bsp_SysTick.h"
#include "./usart/bsp_usart.h"

#include <stdbool.h>
#include <string.h>

volatile uint8_t ucTcpClosedFlag = 0;
char cStr[1500] = { 0 };

#define ROBOTCAR_UART_TIMEOUT 10U
#define ROBOTCAR_MAX_FRAME 512U

static ENUM_ID_NO_TypeDef active_tcp_id = Multiple_ID_0;

static void esp8266_frame_reset(void)
{
    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
}

static char *find_ipd_payload(char *frame, int *payload_len, ENUM_ID_NO_TypeDef *tcp_id)
{
    char *ipd = strstr(frame, "+IPD,");
    char *colon;
    char *cursor;
    char *field;
    int len = 0;
    int id = 0;

    if (ipd == 0) {
        return 0;
    }

    colon = strchr(ipd, ':');
    if (colon == 0) {
        return 0;
    }

    field = ipd + 5;
    if (*field >= '0' && *field <= '4' && *(field + 1) == ',') {
        id = *field - '0';
        field += 2;
    }

    cursor = colon;
    while (cursor > field && *(cursor - 1) >= '0' && *(cursor - 1) <= '9') {
        --cursor;
    }
    while (cursor < colon && *cursor >= '0' && *cursor <= '9') {
        len = (len * 10) + (*cursor - '0');
        ++cursor;
    }

    *payload_len = len;
    *tcp_id = (ENUM_ID_NO_TypeDef)id;
    return colon + 1;
}

static void robot_uart_send(const char *data, int length)
{
    if (length <= 0) {
        return;
    }

    HAL_UART_Transmit(&UartHandle, (uint8_t *)data, (uint16_t)length, 0xFFFF);
}

static int robot_uart_read_available(char *buffer, int max_length)
{
    int length = 0;

    while (length < max_length &&
           __HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) != RESET) {
        uint8_t ch = 0;
        __HAL_UART_CLEAR_OREFLAG(&UartHandle);
        HAL_UART_Receive(&UartHandle, &ch, 1, ROBOTCAR_UART_TIMEOUT);
        buffer[length++] = (char)ch;
    }

    return length;
}

static void wifi_send_to_pc(char *data, int length)
{
    if (length <= 0) {
        return;
    }

    if (length > ROBOTCAR_MAX_FRAME) {
        length = ROBOTCAR_MAX_FRAME;
    }

    ESP8266_SendString(DISABLE, data, (uint32_t)length, active_tcp_id);
}

static void process_wifi_frame(void)
{
    int payload_len = 0;
    ENUM_ID_NO_TypeDef tcp_id = Multiple_ID_0;
    char *payload;

    if (!strEsp8266_Fram_Record.InfBit.FramFinishFlag) {
        return;
    }

    strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
    payload = find_ipd_payload(strEsp8266_Fram_Record.Data_RX_BUF, &payload_len, &tcp_id);
    if (payload != 0 && payload_len > 0) {
        active_tcp_id = tcp_id;
        robot_uart_send(payload, payload_len);
    }

    if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED") != 0) {
    }

    esp8266_frame_reset();
}

static void process_robot_uart(void)
{
    static char uart_buf[ROBOTCAR_MAX_FRAME];
    int length = robot_uart_read_available(uart_buf, (int)sizeof(uart_buf) - 1);

    if (length > 0) {
        uart_buf[length] = '\0';
        wifi_send_to_pc(uart_buf, length);
    }
}

static void wifi_bridge_setup(void)
{
    ESP8266_AT_Test();
    while (!ESP8266_Net_Mode_Choose(STA)) {
    }
    while (!ESP8266_DHCP_CUR()) {
    }
    while (!ESP8266_JoinAP(macUser_ESP8266_StaSsid,
                           macUser_ESP8266_StaPwd)) {
    }
    while (!ESP8266_Enable_MultipleId(ENABLE)) {
    }
    while (!ESP8266_StartOrShutServer(ENABLE,
                                      macUser_ESP8266_TcpServer_Port,
                                      "180")) {
    }

    esp8266_frame_reset();
}

void RobotCar_WifiBridge_Run(void)
{
    wifi_bridge_setup();

    for (;;) {
        process_wifi_frame();
        process_robot_uart();
    }
}
