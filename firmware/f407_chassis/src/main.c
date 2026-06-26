#include "app/motor_demo_app.h"

int main(void)
{
    motor_demo_app_init();

    for (;;) {
        motor_demo_app_task();
    }
}
