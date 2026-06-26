#include "app/board_test_app.h"

#include "FreeRTOS.h"
#include "bsp/board.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/i2c/i2c.h"
#include "drivers/led/led.h"
#include "drivers/uart/uart.h"
#include "task.h"

#define TASK_STACK_WORDS 256U
#define BUZZER_FREQ_HZ   2400U

static void heartbeat_task(void *argument)
{
    (void)argument;

    for (;;) {
        led1_toggle();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void button_buzzer_task(void *argument)
{
    int last_k1 = 0;
    int last_k2 = 0;

    (void)argument;

    for (;;) {
        int k1 = board_k1_is_pressed();
        int k2 = board_k2_is_pressed();

        if (k1 != last_k1) {
            uart1_write(k1 ? "[KEY] K1 pressed\n" : "[KEY] K1 released\n");
            last_k1 = k1;
        }

        if (k2 != last_k2) {
            uart1_write(k2 ? "[KEY] K2 pressed\n" : "[KEY] K2 released\n");
            last_k2 = k2;
        }

        if (k1 || k2) {
            buzzer_on(BUZZER_FREQ_HZ);
        } else {
            buzzer_off();
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void i2c_probe_task(void *argument)
{
    (void)argument;

    for (;;) {
        int found_6a = i2c2_probe(0x6AU);
        int found_6b = i2c2_probe(0x6BU);

        uart1_write("[I2C2] QMI8658 probe 0x6A=");
        uart1_write(found_6a ? "ACK" : "none");
        uart1_write(" 0x6B=");
        uart1_write(found_6b ? "ACK" : "none");
        uart1_write("\n");

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void status_task(void *argument)
{
    uint32_t seconds = 0;

    (void)argument;

    for (;;) {
        uart1_write("[RTOS] alive, uptime=");
        uart1_write_u32(seconds);
        uart1_write("s, test: LED1/K1/K2/buzzer/UART1/I2C2\n");
        seconds += 5U;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void board_test_app_start(void)
{
    board_init();
    board_k1_init();
    board_k2_init();
    led_init();
    buzzer_init();
    buzzer_off();
    uart1_init(115200U);
    i2c2_init();

    uart1_write("\nROS Robot Controller V1.2 FreeRTOS board test\n");
    uart1_write("Safe test: motors are not initialized in this firmware.\n");
    uart1_write("K1/K2 hold = buzzer on, release = off. LED1 blinks at 1Hz.\n");

    (void)xTaskCreate(heartbeat_task, "led", TASK_STACK_WORDS, NULL, 1, NULL);
    (void)xTaskCreate(button_buzzer_task, "keys", TASK_STACK_WORDS, NULL, 2, NULL);
    (void)xTaskCreate(i2c_probe_task, "i2c", TASK_STACK_WORDS, NULL, 1, NULL);
    (void)xTaskCreate(status_task, "status", TASK_STACK_WORDS, NULL, 1, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
}
