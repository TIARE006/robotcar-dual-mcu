#ifndef DRIVERS_UART_UART_H
#define DRIVERS_UART_UART_H

#include "common/types.h"

void uart1_init(uint32_t baudrate);
int uart1_read_char_nonblocking(char *ch);
void uart1_write_char(char ch);
void uart1_write(const char *text);
void uart1_write_hex8(uint8_t value);
void uart1_write_u32(uint32_t value);

void uart2_init(uint32_t baudrate);
int uart2_read_char_nonblocking(char *ch);
void uart2_write_char(char ch);
void uart2_write(const char *text);

void uart3_init(uint32_t baudrate);
int uart3_read_char_nonblocking(char *ch);
void uart3_write_char(char ch);
void uart3_write(const char *text);

#endif
