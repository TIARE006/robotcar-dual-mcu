#ifndef DRIVERS_MOTOR_MOTOR_H
#define DRIVERS_MOTOR_MOTOR_H

#include "bsp/board.h"

typedef board_motor_id_t motor_id_t;

#define MOTOR_M1 BOARD_MOTOR_M1
#define MOTOR_M2 BOARD_MOTOR_M2
#define MOTOR_M3 BOARD_MOTOR_M3
#define MOTOR_M4 BOARD_MOTOR_M4
#define MOTOR_COUNT BOARD_MOTOR_COUNT

#define MOTOR_LEFT_FRONT   BOARD_MOTOR_LEFT_FRONT
#define MOTOR_LEFT_REAR    BOARD_MOTOR_LEFT_REAR
#define MOTOR_RIGHT_FRONT  BOARD_MOTOR_RIGHT_FRONT
#define MOTOR_RIGHT_REAR   BOARD_MOTOR_RIGHT_REAR

void motor_init(void);
void motor_set(motor_id_t motor, int speed);
void motor_stop_all(void);
void motor_drive_lr(int left_speed, int right_speed);

#endif
