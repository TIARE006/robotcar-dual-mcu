#ifndef DRIVERS_CHASSIS_MECANUM_H
#define DRIVERS_CHASSIS_MECANUM_H

#include "common/types.h"

typedef enum {
    MECANUM_MOTION_STOP = 0,
    MECANUM_MOTION_FORWARD,
    MECANUM_MOTION_BACKWARD,
    MECANUM_MOTION_LEFT,
    MECANUM_MOTION_RIGHT,
    MECANUM_MOTION_ROTATE_LEFT,
    MECANUM_MOTION_ROTATE_RIGHT,
    MECANUM_MOTION_COUNT
} mecanum_motion_t;

void mecanum_init(void);
void mecanum_stop(void);
void mecanum_drive(int forward, int strafe, int rotate);
void mecanum_control_task(void);
void mecanum_motion(mecanum_motion_t motion, int speed);
int mecanum_set_wheel_trim(unsigned wheel, int trim_permille);
int mecanum_get_wheel_trim(unsigned wheel);
void mecanum_set_closed_loop(int enabled);
int mecanum_get_closed_loop(void);
int mecanum_get_wheel_delta(unsigned wheel);
int mecanum_get_wheel_output(unsigned wheel);

#endif
