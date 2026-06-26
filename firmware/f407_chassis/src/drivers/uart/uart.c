#include "drivers/uart/uart.h"

#include "bsp/board.h"
#include "common/bitops.h"
#include "platform/stm32f407_regs.h"

#define RCC_USART1_EN  BIT(4)
#define RCC_USART2_EN  BIT(17)
#define RCC_USART3_EN  BIT(18)
#define USART_SR_TXE   BIT(7)
#define USART_SR_RXNE  BIT(5)
#define USART_CR1_UE   BIT(13)
#define USART_CR1_TE   BIT(3)
#define USART_CR1_RE   BIT(2)
#define PCLK2_HZ       16000000UL
#define PCLK1_HZ       16000000UL

static void uart_init_regs(uint32_t base, uint32_t baudrate, uint32_t pclk_hz)
{
    USART_CR1(base) = 0;
    USART_CR2(base) = 0;
    USART_CR3(base) = 0;
    USART_BRR(base) = (pclk_hz + (baudrate / 2U)) / baudrate;
    USART_CR1(base) = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static int uart_read_char_nonblocking(uint32_t base, char *ch)
{
    if ((USART_SR(base) & USART_SR_RXNE) == 0UL) {
        return 0;
    }

    *ch = (char)(USART_DR(base) & 0xFFU);
    return 1;
}

static void uart_write_char(uint32_t base, char ch)
{
    if (ch == '\n') {
        uart_write_char(base, '\r');
    }
    while ((USART_SR(base) & USART_SR_TXE) == 0UL) {
    }
    USART_DR(base) = (uint32_t)(uint8_t)ch;
}

static void uart_write(uint32_t base, const char *text)
{
    while (*text != '\0') {
        uart_write_char(base, *text++);
    }
}

void uart1_init(uint32_t baudrate)
{
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_A, 9U, 7U }, 0);
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_A, 10U, 7U }, 0);

    RCC_APB2ENR |= RCC_USART1_EN;
    (void)RCC_APB2ENR;
    uart_init_regs(USART1_BASE, baudrate, PCLK2_HZ);
}

int uart1_read_char_nonblocking(char *ch)
{
    return uart_read_char_nonblocking(USART1_BASE, ch);
}

void uart1_write_char(char ch)
{
    uart_write_char(USART1_BASE, ch);
}

void uart1_write(const char *text)
{
    uart_write(USART1_BASE, text);
}

void uart1_write_hex8(uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    uart1_write_char(hex[(value >> 4) & 0x0F]);
    uart1_write_char(hex[value & 0x0F]);
}

void uart1_write_u32(uint32_t value)
{
    char buffer[11];
    unsigned index = sizeof(buffer);

    buffer[--index] = '\0';
    do {
        buffer[--index] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    uart1_write(&buffer[index]);
}

void uart2_init(uint32_t baudrate)
{
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_D, 5U, 7U }, 0);
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_D, 6U, 7U }, 0);

    RCC_APB1ENR |= RCC_USART2_EN;
    (void)RCC_APB1ENR;
    uart_init_regs(USART2_BASE, baudrate, PCLK1_HZ);
}

int uart2_read_char_nonblocking(char *ch)
{
    return uart_read_char_nonblocking(USART2_BASE, ch);
}

void uart2_write_char(char ch)
{
    uart_write_char(USART2_BASE, ch);
}

void uart2_write(const char *text)
{
    uart_write(USART2_BASE, text);
}

void uart3_init(uint32_t baudrate)
{
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_C, 10U, 7U }, 0);
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_C, 11U, 7U }, 0);

    RCC_APB1ENR |= RCC_USART3_EN;
    (void)RCC_APB1ENR;
    uart_init_regs(USART3_BASE, baudrate, PCLK1_HZ);
}

int uart3_read_char_nonblocking(char *ch)
{
    return uart_read_char_nonblocking(USART3_BASE, ch);
}

void uart3_write_char(char ch)
{
    uart_write_char(USART3_BASE, ch);
}

void uart3_write(const char *text)
{
    uart_write(USART3_BASE, text);
}
