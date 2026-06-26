#include "drivers/led/led.h"

#include "bsp/board.h"

static int g_led1_active;

void led_init(void)
{
    board_gpio_output_init(board_led1_pin());
    led1_off();
}

void led1_on(void)
{
    g_led1_active = 1;
    board_gpio_write(board_led1_pin(), 1);
}

void led1_off(void)
{
    g_led1_active = 0;
    board_gpio_write(board_led1_pin(), 0);
}

void led1_toggle(void)
{
    if (g_led1_active) {
        led1_off();
    } else {
        led1_on();
    }
}
