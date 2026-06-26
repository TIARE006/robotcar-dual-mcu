#include "bsp/board.h"

#include "common/bitops.h"
#include "platform/stm32f407_regs.h"

#define RCC_GPIOA_EN   BIT(0)
#define RCC_GPIOB_EN   BIT(1)
#define RCC_GPIOC_EN   BIT(2)
#define RCC_GPIOD_EN   BIT(3)
#define RCC_GPIOE_EN   BIT(4)

#define BOARD_K2_PORT  GPIOE_BASE
#define BOARD_K2_PIN   0U
#define BOARD_K1_PORT  GPIOE_BASE
#define BOARD_K1_PIN   1U

static const board_gpio_pin_t g_led1 = { BOARD_GPIO_PORT_E, 10U, 1U };
static const board_gpio_pin_t g_buzzer = { BOARD_GPIO_PORT_A, 8U, 0U };

static const board_motor_channel_t g_motor_channels[BOARD_MOTOR_COUNT] = {
    /* U5 SA8870C: IN1=PE14 -> M1_F, IN2=PE13 -> M1_B */
    [BOARD_MOTOR_M1] = {
        { TIM1_BASE, 4U, { BOARD_GPIO_PORT_E, 14U, 1U } },
        { TIM1_BASE, 3U, { BOARD_GPIO_PORT_E, 13U, 1U } },
    },
    /* U10 SA8870C: IN1=PE11 -> M2_F, IN2=PE9 -> M2_B */
    [BOARD_MOTOR_M2] = {
        { TIM1_BASE, 2U, { BOARD_GPIO_PORT_E, 11U, 1U } },
        { TIM1_BASE, 1U, { BOARD_GPIO_PORT_E, 9U, 1U } },
    },
    /* U8 SA8870C: IN1=PE5 -> M3_F, IN2=PE6 -> M3_B */
    [BOARD_MOTOR_M3] = {
        { TIM9_BASE, 1U, { BOARD_GPIO_PORT_E, 5U, 3U } },
        { TIM9_BASE, 2U, { BOARD_GPIO_PORT_E, 6U, 3U } },
    },
    /* U12 SA8870C: IN1=PB9 -> M4_F, IN2=PB8 -> M4_B */
    [BOARD_MOTOR_M4] = {
        { TIM4_BASE, 4U, { BOARD_GPIO_PORT_B, 9U, 2U } },
        { TIM4_BASE, 3U, { BOARD_GPIO_PORT_B, 8U, 2U } },
    },
};

void board_init(void)
{
    RCC_AHB1ENR |= RCC_GPIOA_EN | RCC_GPIOB_EN | RCC_GPIOC_EN |
                   RCC_GPIOD_EN | RCC_GPIOE_EN;
    (void)RCC_AHB1ENR;
}

uint32_t board_gpio_base(board_gpio_port_t port)
{
    if (port == BOARD_GPIO_PORT_A) {
        return GPIOA_BASE;
    }
    if (port == BOARD_GPIO_PORT_B) {
        return GPIOB_BASE;
    }
    if (port == BOARD_GPIO_PORT_C) {
        return GPIOC_BASE;
    }
    if (port == BOARD_GPIO_PORT_D) {
        return GPIOD_BASE;
    }
    return GPIOE_BASE;
}

void board_gpio_output_init(board_gpio_pin_t pin)
{
    uint32_t base = board_gpio_base(pin.port);

    GPIO_MODER(base) &= ~(3UL << (pin.pin * 2U));
    GPIO_MODER(base) |=  (1UL << (pin.pin * 2U));
    GPIO_OTYPER(base) &= ~BIT(pin.pin);
    GPIO_OSPEEDR(base) |= (2UL << (pin.pin * 2U));
    board_gpio_write(pin, 0);
}

void board_gpio_input_pullup_init(board_gpio_pin_t pin)
{
    uint32_t base = board_gpio_base(pin.port);

    GPIO_MODER(base) &= ~(3UL << (pin.pin * 2U));
    GPIO_PUPDR(base) &= ~(3UL << (pin.pin * 2U));
    GPIO_PUPDR(base) |=  (1UL << (pin.pin * 2U));
}

void board_gpio_af_init(board_pwm_pin_t pin, int open_drain)
{
    uint32_t base = board_gpio_base(pin.port);
    volatile uint32_t *afr = (pin.pin < 8U) ? &GPIO_AFRL(base) : &GPIO_AFRH(base);
    unsigned shift = (pin.pin & 7U) * 4U;

    GPIO_MODER(base) &= ~(3UL << (pin.pin * 2U));
    GPIO_MODER(base) |=  (2UL << (pin.pin * 2U));
    GPIO_OSPEEDR(base) |= (2UL << (pin.pin * 2U));
    if (open_drain) {
        GPIO_OTYPER(base) |= BIT(pin.pin);
    } else {
        GPIO_OTYPER(base) &= ~BIT(pin.pin);
    }
    *afr &= ~(15UL << shift);
    *afr |=  ((uint32_t)pin.alternate_function << shift);
}

void board_gpio_write(board_gpio_pin_t pin, int active)
{
    uint32_t base = board_gpio_base(pin.port);
    int level_high = pin.active_low ? !active : active;

    if (level_high) {
        GPIO_BSRR(base) = BIT(pin.pin);
    } else {
        GPIO_BSRR(base) = BIT(pin.pin + 16U);
    }
}

int board_gpio_read_active(board_gpio_pin_t pin)
{
    uint32_t base = board_gpio_base(pin.port);
    int level_high = (GPIO_IDR(base) & BIT(pin.pin)) != 0UL;

    return pin.active_low ? !level_high : level_high;
}

void board_k2_init(void)
{
    board_gpio_input_pullup_init((board_gpio_pin_t){ BOARD_GPIO_PORT_E, BOARD_K2_PIN, 1U });
}

int board_k2_is_pressed(void)
{
    return (GPIO_IDR(BOARD_K2_PORT) & BIT(BOARD_K2_PIN)) == 0UL;
}

void board_k1_init(void)
{
    board_gpio_input_pullup_init((board_gpio_pin_t){ BOARD_GPIO_PORT_E, BOARD_K1_PIN, 1U });
}

int board_k1_is_pressed(void)
{
    return (GPIO_IDR(BOARD_K1_PORT) & BIT(BOARD_K1_PIN)) == 0UL;
}

const board_motor_channel_t *board_motor_channel(board_motor_id_t motor)
{
    if ((unsigned)motor >= BOARD_MOTOR_COUNT) {
        return NULL;
    }

    return &g_motor_channels[motor];
}

board_gpio_pin_t board_led1_pin(void)
{
    return g_led1;
}

board_gpio_pin_t board_buzzer_pin(void)
{
    return g_buzzer;
}
