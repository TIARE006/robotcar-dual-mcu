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
#define ROBOTCAR_DIAG_BURST_COUNT 20U
#define ROBOTCAR_REPLY_POLL_MS 500U

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
    char frame[96];
    int frame_len = 0;

    if (length <= 0) {
        return;
    }

    while (frame_len < (int)(sizeof(frame) - 2U) &&
           frame_len < length &&
           data[frame_len] != '\r' &&
           data[frame_len] != '\n' &&
           data[frame_len] != '\0') {
        frame[frame_len] = data[frame_len];
        ++frame_len;
    }

    if (frame_len <= 0) {
        return;
    }

    frame[frame_len++] = '\n';

    {
        int i;

        for (i = 0; i < frame_len; ++i) {
            HAL_UART_Transmit(&UartHandle,
                              (uint8_t *)&frame[i],
                              1U,
                              0xFFFF);
        }
    }
}

static void robot_uart_send_text(const char *text)
{
    HAL_UART_Transmit(&UartHandle,
                      (uint8_t *)text,
                      (uint16_t)strlen(text),
                      0xFFFF);
}

static void robot_uart_diag_burst(void)
{
    unsigned index;

    for (index = 0U; index < ROBOTCAR_DIAG_BURST_COUNT; ++index) {
        robot_uart_send_text("G\n");
        Delay_ms(20);
    }
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

static void robot_uart_flush_rx(void)
{
    char discard[32];

    while (robot_uart_read_available(discard, (int)sizeof(discard)) > 0) {
    }
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

static void robot_uart_collect_reply(void)
{
    static char reply[ROBOTCAR_MAX_FRAME];
    int length = 0;
    unsigned waited = 0U;

    while (waited < ROBOTCAR_REPLY_POLL_MS &&
           length < (int)sizeof(reply) - 1) {
        int got = robot_uart_read_available(&reply[length],
                                            (int)sizeof(reply) - 1 - length);
        if (got > 0) {
            length += got;
        }
        Delay_ms(5);
        waited += 5U;
    }

    if (length > 0) {
        reply[length] = '\0';
        wifi_send_to_pc("[F407] ", 7);
        wifi_send_to_pc(reply, length);
        if (reply[length - 1] != '\n') {
            wifi_send_to_pc("\r\n", 2);
        }
    } else {
        wifi_send_to_pc("[F407] <no reply>\r\n", 19);
    }
}

static int is_fast_control_command(char command)
{
    return command == 'V' || command == 'v' ||
           command == 'S' || command == 's' ||
           command == 'M' || command == 'm' ||
           command == 'T' || command == 't' ||
           command == 'P' || command == 'p' ||
           command == 'C' || command == 'c';
}

static void process_wifi_frame(void)
{
    int payload_len = 0;
    ENUM_ID_NO_TypeDef tcp_id = Multiple_ID_0;
    char *payload;
    char command[ROBOTCAR_MAX_FRAME];
    int command_len;

    if (!strEsp8266_Fram_Record.InfBit.FramFinishFlag) {
        return;
    }

    strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
    payload = find_ipd_payload(strEsp8266_Fram_Record.Data_RX_BUF, &payload_len, &tcp_id);
    if (payload != 0 && payload_len > 0) {
        command_len = payload_len;
        if (command_len > (int)sizeof(command) - 1) {
            command_len = (int)sizeof(command) - 1;
        }
        memcpy(command, payload, (size_t)command_len);
        command[command_len] = '\0';

        active_tcp_id = tcp_id;
        if (command[0] == 'X' || command[0] == 'x') {
            wifi_send_to_pc("[H7 RX] X\r\n", 11);
            wifi_send_to_pc("[H7 UART DIAG BURST]\r\n", 23);
            robot_uart_flush_rx();
            robot_uart_diag_burst();
            robot_uart_collect_reply();
        } else {
            robot_uart_flush_rx();
            robot_uart_send(command, command_len);
            if (!is_fast_control_command(command[0])) {
                robot_uart_collect_reply();
            }
        }
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
