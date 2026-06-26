#include "FreeRTOS.h"
#include "drivers/buzzer/buzzer.h"
#include "drivers/led/led.h"
#include "task.h"

void task_assert_failed(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) {
        led1_on();
        buzzer_on(1200U);
    }
}

void vApplicationMallocFailedHook(void)
{
    task_assert_failed();
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    task_assert_failed();
}
