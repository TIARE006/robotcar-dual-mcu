#include "drivers/motor/motor.h"

#include "common/bitops.h"
#include "platform/stm32f407_regs.h"

#define RCC_TIM4_EN    BIT(2)
#define RCC_TIM1_EN    BIT(0)
#define RCC_TIM9_EN    BIT(16)

#define TIM_CR1_CEN    BIT(0)
#define TIM_CR1_ARPE   BIT(7)
#define TIM_EGR_UG     BIT(0)
#define TIM_CCER_CC1E  BIT(0)
#define TIM_CCER_CC2E  BIT(4)
#define TIM_CCER_CC3E  BIT(8)
#define TIM_CCER_CC4E  BIT(12)
#define TIM_BDTR_MOE   BIT(15)
#define PWM_MODE1      6UL
#define MOTOR_MIN_RUNNING_DUTY 450U

static uint32_t gpio_base(board_gpio_port_t port)
{
    if (port == BOARD_GPIO_PORT_A) {
        return GPIOA_BASE;
    }
    if (port == BOARD_GPIO_PORT_B) {
        return GPIOB_BASE;
    }
    return GPIOE_BASE;
}

static unsigned clamp_duty(int speed)
{
    if (speed < 0) {
        speed = -speed;
    }
    if (speed > (int)BOARD_PWM_MAX_DUTY) {
        speed = (int)BOARD_PWM_MAX_DUTY;
    }
    return (unsigned)speed;
}

static unsigned apply_min_running_duty(unsigned duty)
{
    if (duty == 0U) {
        return 0U;
    }
    if (duty < MOTOR_MIN_RUNNING_DUTY) {
        return MOTOR_MIN_RUNNING_DUTY;
    }
    return duty;
}

static void pwm_pin_init(board_pwm_pin_t pin)
{
    uint32_t base = gpio_base(pin.port);
    volatile uint32_t *afr = (pin.pin < 8U) ? &GPIO_AFRL(base) : &GPIO_AFRH(base);
    unsigned shift = (pin.pin & 7U) * 4U;

    GPIO_MODER(base) &= ~(3UL << (pin.pin * 2U));
    GPIO_MODER(base) |=  (2UL << (pin.pin * 2U));
    *afr &= ~(15UL << shift);
    *afr |=  ((uint32_t)pin.alternate_function << shift);
}

static void timer_init(uint32_t base, int advanced_timer)
{
    /*
     * Reset HSI is 16 MHz. 16 MHz / 16 / 1000 = 1 kHz PWM.
     * Keep this conservative until motor current and heating are measured.
     */
    TIM_PSC(base) = 15UL;
    TIM_ARR(base) = BOARD_PWM_MAX_DUTY - 1UL;
    TIM_CCR1(base) = 0UL;
    TIM_CCR2(base) = 0UL;
    TIM_CCR3(base) = 0UL;
    TIM_CCR4(base) = 0UL;

    TIM_CCMR1(base) =
        BIT(3) | (PWM_MODE1 << 4) |
        BIT(11) | (PWM_MODE1 << 12);
    TIM_CCMR2(base) =
        BIT(3) | (PWM_MODE1 << 4) |
        BIT(11) | (PWM_MODE1 << 12);
    TIM_CCER(base) =
        TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

    if (advanced_timer) {
        TIM_BDTR(base) = TIM_BDTR_MOE;
    }

    TIM_EGR(base) = TIM_EGR_UG;
    TIM_CR1(base) = TIM_CR1_ARPE | TIM_CR1_CEN;
}

static void pwm_write(board_motor_pwm_t pwm, unsigned duty)
{
    if (pwm.channel == 1U) {
        TIM_CCR1(pwm.timer_base) = duty;
    } else if (pwm.channel == 2U) {
        TIM_CCR2(pwm.timer_base) = duty;
    } else if (pwm.channel == 3U) {
        TIM_CCR3(pwm.timer_base) = duty;
    } else {
        TIM_CCR4(pwm.timer_base) = duty;
    }
}

void motor_init(void)
{
    unsigned i;

    board_init();

    RCC_APB1ENR |= RCC_TIM4_EN;
    RCC_APB2ENR |= RCC_TIM1_EN | RCC_TIM9_EN;
    (void)RCC_APB2ENR;

    for (i = 0; i < (unsigned)BOARD_MOTOR_COUNT; ++i) {
        const board_motor_channel_t *channel = board_motor_channel((board_motor_id_t)i);
        pwm_pin_init(channel->forward.gpio);
        pwm_pin_init(channel->backward.gpio);
    }

    timer_init(TIM1_BASE, 1);
    timer_init(TIM4_BASE, 0);
    timer_init(TIM9_BASE, 0);
    motor_stop_all();
}

void motor_set(motor_id_t motor, int speed)
{
    const board_motor_channel_t *channel = board_motor_channel(motor);
    unsigned duty;

    if (channel == NULL) {
        return;
    }

    duty = apply_min_running_duty(clamp_duty(speed));
    if (speed > 0) {
        pwm_write(channel->backward, 0);
        pwm_write(channel->forward, duty);
    } else if (speed < 0) {
        pwm_write(channel->forward, 0);
        pwm_write(channel->backward, duty);
    } else {
        pwm_write(channel->forward, 0);
        pwm_write(channel->backward, 0);
    }
}

void motor_stop_all(void)
{
    motor_set(MOTOR_M1, 0);
    motor_set(MOTOR_M2, 0);
    motor_set(MOTOR_M3, 0);
    motor_set(MOTOR_M4, 0);
}

void motor_drive_lr(int left_speed, int right_speed)
{
    motor_set(MOTOR_LEFT_FRONT, left_speed);
    motor_set(MOTOR_LEFT_REAR, left_speed);
    motor_set(MOTOR_RIGHT_FRONT, right_speed);
    motor_set(MOTOR_RIGHT_REAR, right_speed);
}
