#include "drivers/i2c/i2c.h"

#include "bsp/board.h"
#include "common/bitops.h"
#include "platform/stm32f407_regs.h"

#define RCC_I2C2_EN       BIT(22)
#define I2C_CR1_PE        BIT(0)
#define I2C_CR1_START     BIT(8)
#define I2C_CR1_STOP      BIT(9)
#define I2C_SR1_SB        BIT(0)
#define I2C_SR1_ADDR      BIT(1)
#define I2C_SR1_AF        BIT(10)
#define APB1_MHZ          16UL

static int wait_set(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t timeout = 200000UL;

    while (((*reg) & mask) == 0UL) {
        if (--timeout == 0UL) {
            return 0;
        }
    }
    return 1;
}

void i2c2_init(void)
{
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_B, 10U, 4U }, 1);
    board_gpio_af_init((board_pwm_pin_t){ BOARD_GPIO_PORT_B, 11U, 4U }, 1);

    RCC_APB1ENR |= RCC_I2C2_EN;
    (void)RCC_APB1ENR;

    I2C_CR1(I2C2_BASE) = 0;
    I2C_CR2(I2C2_BASE) = APB1_MHZ;
    I2C_CCR(I2C2_BASE) = 80U;
    I2C_TRISE(I2C2_BASE) = APB1_MHZ + 1U;
    I2C_CR1(I2C2_BASE) = I2C_CR1_PE;
}

int i2c2_probe(uint8_t address_7bit)
{
    volatile uint32_t dummy;

    I2C_SR1(I2C2_BASE) &= ~I2C_SR1_AF;
    I2C_CR1(I2C2_BASE) |= I2C_CR1_START;
    if (!wait_set(&I2C_SR1(I2C2_BASE), I2C_SR1_SB)) {
        I2C_CR1(I2C2_BASE) |= I2C_CR1_STOP;
        return 0;
    }

    I2C_DR(I2C2_BASE) = ((uint32_t)address_7bit << 1);

    for (uint32_t timeout = 200000UL; timeout > 0UL; --timeout) {
        if ((I2C_SR1(I2C2_BASE) & I2C_SR1_ADDR) != 0UL) {
            dummy = I2C_SR1(I2C2_BASE);
            dummy = I2C_SR2(I2C2_BASE);
            (void)dummy;
            I2C_CR1(I2C2_BASE) |= I2C_CR1_STOP;
            return 1;
        }
        if ((I2C_SR1(I2C2_BASE) & I2C_SR1_AF) != 0UL) {
            I2C_SR1(I2C2_BASE) &= ~I2C_SR1_AF;
            I2C_CR1(I2C2_BASE) |= I2C_CR1_STOP;
            return 0;
        }
    }

    I2C_CR1(I2C2_BASE) |= I2C_CR1_STOP;
    return 0;
}
