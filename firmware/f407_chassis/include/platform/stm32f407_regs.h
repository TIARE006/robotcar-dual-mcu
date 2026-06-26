#ifndef PLATFORM_STM32F407_REGS_H
#define PLATFORM_STM32F407_REGS_H

#include "common/types.h"

#define REG32(address) (*(volatile uint32_t *)(address))

#define RCC_BASE       0x40023800UL
#define GPIOA_BASE     0x40020000UL
#define GPIOB_BASE     0x40020400UL
#define GPIOC_BASE     0x40020800UL
#define GPIOD_BASE     0x40020C00UL
#define GPIOE_BASE     0x40021000UL
#define USART1_BASE    0x40011000UL
#define USART2_BASE    0x40004400UL
#define USART3_BASE    0x40004800UL
#define I2C2_BASE      0x40005800UL
#define TIM1_BASE      0x40010000UL
#define TIM4_BASE      0x40000800UL
#define TIM9_BASE      0x40014000UL

#define RCC_AHB1ENR    REG32(RCC_BASE + 0x30UL)
#define RCC_APB1ENR    REG32(RCC_BASE + 0x40UL)
#define RCC_APB2ENR    REG32(RCC_BASE + 0x44UL)

#define GPIO_MODER(base)    REG32((base) + 0x00UL)
#define GPIO_OTYPER(base)   REG32((base) + 0x04UL)
#define GPIO_OSPEEDR(base)  REG32((base) + 0x08UL)
#define GPIO_PUPDR(base)    REG32((base) + 0x0CUL)
#define GPIO_IDR(base)      REG32((base) + 0x10UL)
#define GPIO_ODR(base)      REG32((base) + 0x14UL)
#define GPIO_BSRR(base)     REG32((base) + 0x18UL)
#define GPIO_AFRL(base)     REG32((base) + 0x20UL)
#define GPIO_AFRH(base)     REG32((base) + 0x24UL)

#define USART_SR(base)      REG32((base) + 0x00UL)
#define USART_DR(base)      REG32((base) + 0x04UL)
#define USART_BRR(base)     REG32((base) + 0x08UL)
#define USART_CR1(base)     REG32((base) + 0x0CUL)
#define USART_CR2(base)     REG32((base) + 0x10UL)
#define USART_CR3(base)     REG32((base) + 0x14UL)

#define I2C_CR1(base)       REG32((base) + 0x00UL)
#define I2C_CR2(base)       REG32((base) + 0x04UL)
#define I2C_OAR1(base)      REG32((base) + 0x08UL)
#define I2C_DR(base)        REG32((base) + 0x10UL)
#define I2C_SR1(base)       REG32((base) + 0x14UL)
#define I2C_SR2(base)       REG32((base) + 0x18UL)
#define I2C_CCR(base)       REG32((base) + 0x1CUL)
#define I2C_TRISE(base)     REG32((base) + 0x20UL)

#define TIM_CR1(base)       REG32((base) + 0x00UL)
#define TIM_EGR(base)       REG32((base) + 0x14UL)
#define TIM_CCMR1(base)     REG32((base) + 0x18UL)
#define TIM_CCMR2(base)     REG32((base) + 0x1CUL)
#define TIM_CCER(base)      REG32((base) + 0x20UL)
#define TIM_PSC(base)       REG32((base) + 0x28UL)
#define TIM_ARR(base)       REG32((base) + 0x2CUL)
#define TIM_CCR1(base)      REG32((base) + 0x34UL)
#define TIM_CCR2(base)      REG32((base) + 0x38UL)
#define TIM_CCR3(base)      REG32((base) + 0x3CUL)
#define TIM_CCR4(base)      REG32((base) + 0x40UL)
#define TIM_BDTR(base)      REG32((base) + 0x44UL)

#endif
