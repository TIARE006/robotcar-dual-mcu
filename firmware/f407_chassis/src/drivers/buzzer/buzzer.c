#include "drivers/buzzer/buzzer.h"

#include "FreeRTOS.h"
#include "bsp/board.h"
#include "common/bitops.h"
#include "platform/stm32f407_regs.h"
#include "task.h"

#define RCC_TIM1_EN    BIT(0)
#define TIM_CR1_CEN    BIT(0)
#define TIM_CR1_ARPE   BIT(7)
#define TIM_EGR_UG     BIT(0)
#define TIM_CCER_CC1E  BIT(0)
#define TIM_BDTR_MOE   BIT(15)
#define PWM_MODE1      6UL
#define TIMER_CLOCK_HZ 16000000UL

void buzzer_init(void)
{
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_A, 8U, 1U }, 0);

    RCC_APB2ENR |= RCC_TIM1_EN;
    (void)RCC_APB2ENR;

    TIM_CR1(TIM1_BASE) = 0;
    TIM_PSC(TIM1_BASE) = 0;
    TIM_ARR(TIM1_BASE) = 999U;
    TIM_CCR1(TIM1_BASE) = 0;
    TIM_CCMR1(TIM1_BASE) &= ~0xFFUL;
    TIM_CCMR1(TIM1_BASE) |= BIT(3) | (PWM_MODE1 << 4);
    TIM_CCER(TIM1_BASE) |= TIM_CCER_CC1E;
    TIM_BDTR(TIM1_BASE) |= TIM_BDTR_MOE;
    TIM_EGR(TIM1_BASE) = TIM_EGR_UG;
    TIM_CR1(TIM1_BASE) = TIM_CR1_ARPE | TIM_CR1_CEN;
}

void buzzer_on(uint32_t frequency_hz)
{
    uint32_t period;

    if (frequency_hz < 100U) {
        frequency_hz = 100U;
    }
    if (frequency_hz > 10000U) {
        frequency_hz = 10000U;
    }

    period = (TIMER_CLOCK_HZ / frequency_hz);
    if (period < 2U) {
        period = 2U;
    }

    TIM_ARR(TIM1_BASE) = period - 1U;
    TIM_CCR1(TIM1_BASE) = period / 2U;
    TIM_EGR(TIM1_BASE) = TIM_EGR_UG;
}

void buzzer_off(void)
{
    TIM_CCR1(TIM1_BASE) = 0;
}

void buzzer_beep_ms(uint32_t frequency_hz, uint32_t duration_ms)
{
    buzzer_on(frequency_hz);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    buzzer_off();
}
