#ifndef BSP_BOARD_H
#define BSP_BOARD_H

#include "common/types.h"

typedef enum {
    BOARD_GPIO_PORT_A = 0,
    BOARD_GPIO_PORT_B,
    BOARD_GPIO_PORT_C,
    BOARD_GPIO_PORT_D,
    BOARD_GPIO_PORT_E
} board_gpio_port_t;

typedef struct {
    board_gpio_port_t port;
    uint8_t pin;
    uint8_t active_low;
} board_gpio_pin_t;

typedef struct {
    board_gpio_port_t port;
    uint8_t pin;
    uint8_t alternate_function;
} board_pwm_pin_t;

typedef struct {
    uint32_t timer_base;
    uint8_t channel;
    board_pwm_pin_t gpio;
} board_motor_pwm_t;

typedef struct {
    board_motor_pwm_t forward;
    board_motor_pwm_t backward;
} board_motor_channel_t;

typedef enum {
    BOARD_MOTOR_M1 = 0,
    BOARD_MOTOR_M2,
    BOARD_MOTOR_M3,
    BOARD_MOTOR_M4,
    BOARD_MOTOR_COUNT
} board_motor_id_t;

/*
 * The schematic names only M1..M4. These aliases are the local chassis
 * convention and are intentionally isolated here.
 */
#define BOARD_MOTOR_LEFT_FRONT   BOARD_MOTOR_M1
#define BOARD_MOTOR_LEFT_REAR    BOARD_MOTOR_M2
#define BOARD_MOTOR_RIGHT_FRONT  BOARD_MOTOR_M3
#define BOARD_MOTOR_RIGHT_REAR   BOARD_MOTOR_M4

#define BOARD_PWM_MAX_DUTY 1000U

void board_init(void);
uint32_t board_gpio_base(board_gpio_port_t port);
void board_gpio_output_init(board_gpio_pin_t pin);
void board_gpio_input_pullup_init(board_gpio_pin_t pin);
void board_gpio_af_init(board_pwm_pin_t pin, int open_drain);
void board_gpio_write(board_gpio_pin_t pin, int active);
int board_gpio_read_active(board_gpio_pin_t pin);

void board_k2_init(void);
int board_k2_is_pressed(void);
void board_k1_init(void);
int board_k1_is_pressed(void);

const board_motor_channel_t *board_motor_channel(board_motor_id_t motor);
board_gpio_pin_t board_led1_pin(void);
board_gpio_pin_t board_buzzer_pin(void);

#endif
