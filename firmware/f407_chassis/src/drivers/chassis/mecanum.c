#include "drivers/chassis/mecanum.h"

#include "bsp/board.h"
#include "drivers/encoder/encoder.h"
#include "drivers/motor/motor.h"

#define SPEED_MAX ((int)BOARD_PWM_MAX_DUTY)

/*
 * Wheel calibration for this chassis.
 * M1/M2 are reversed relative to the first software convention.
 * M3 has been verified and is enabled for normal mecanum control.
 */
#define LEFT_FRONT_SIGN   (-1)
#define LEFT_REAR_SIGN    (-1)
#define RIGHT_FRONT_SIGN  (1)
#define RIGHT_REAR_SIGN   (1)

#define LEFT_FRONT_ENABLE  1
#define LEFT_REAR_ENABLE   1
#define RIGHT_FRONT_ENABLE 1
#define RIGHT_REAR_ENABLE  1

#define WHEEL_TRIM_MIN 500
#define WHEEL_TRIM_MAX 1500
#define WHEEL_TRIM_ONE 1000
#define CONTROL_PERIOD_POLLS 6000U
#define CONTROL_KP_NUM 2
#define CONTROL_KP_DEN 5
#define CONTROL_KI_DEN 80
#define CONTROL_INTEGRAL_LIMIT 20000

static int g_wheel_trim[BOARD_MOTOR_COUNT] = {
    890,
    920,
    1270,
    1040,
};

static const int g_motor_sign[BOARD_MOTOR_COUNT] = {
    LEFT_FRONT_SIGN,
    LEFT_REAR_SIGN,
    RIGHT_FRONT_SIGN,
    RIGHT_REAR_SIGN,
};

static const int g_wheel_enable[BOARD_MOTOR_COUNT] = {
    LEFT_FRONT_ENABLE,
    LEFT_REAR_ENABLE,
    RIGHT_FRONT_ENABLE,
    RIGHT_REAR_ENABLE,
};

static const int g_encoder_polarity[BOARD_MOTOR_COUNT] = {
    1,
    1,
    -1,
    -1,
};

static int g_closed_loop_enabled = 1;
static int g_target_speed[BOARD_MOTOR_COUNT];
static int g_motor_output[BOARD_MOTOR_COUNT];
static int g_last_delta[BOARD_MOTOR_COUNT];
static int g_error_integral[BOARD_MOTOR_COUNT];
static int32_t g_last_encoder_count[BOARD_MOTOR_COUNT];
static unsigned g_control_poll_counter;

static int clamp_speed(int value)
{
    if (value > SPEED_MAX) {
        return SPEED_MAX;
    }
    if (value < -SPEED_MAX) {
        return -SPEED_MAX;
    }
    return value;
}

static int max_abs4(int a, int b, int c, int d)
{
    int values[4] = { a, b, c, d };
    int max = 0;

    for (unsigned i = 0; i < 4U; ++i) {
        int value = values[i];
        if (value < 0) {
            value = -value;
        }
        if (value > max) {
            max = value;
        }
    }
    return max;
}

static int apply_trim(motor_id_t motor, int speed)
{
    int trim = g_wheel_trim[(unsigned)motor];
    long scaled = ((long)speed * (long)trim) / WHEEL_TRIM_ONE;

    return clamp_speed((int)scaled);
}

static int feedforward_output(motor_id_t motor, int speed)
{
    return apply_trim(motor, speed * g_motor_sign[(unsigned)motor]);
}

static void wheel_set(motor_id_t motor, int speed)
{
    unsigned index = (unsigned)motor;

    if (!g_wheel_enable[index]) {
        g_target_speed[index] = 0;
        g_motor_output[index] = 0;
        motor_set(motor, 0);
        return;
    }

    g_target_speed[index] = clamp_speed(speed);
    g_motor_output[index] = feedforward_output(motor, g_target_speed[index]);
    motor_set(motor, g_motor_output[index]);
}

static int normalized_encoder_count(unsigned wheel)
{
    int32_t count = encoder_get_count((board_motor_id_t)wheel);

    return (int)(count * g_encoder_polarity[wheel]);
}

static int abs_int(int value)
{
    return value < 0 ? -value : value;
}

static int signed_for_target(int magnitude, int target)
{
    if (target < 0) {
        return -magnitude;
    }
    return magnitude;
}

static void reset_control_state(void)
{
    unsigned i;

    for (i = 0U; i < (unsigned)BOARD_MOTOR_COUNT; ++i) {
        g_last_encoder_count[i] = normalized_encoder_count(i);
        g_last_delta[i] = 0;
        g_error_integral[i] = 0;
        g_motor_output[i] = 0;
        g_target_speed[i] = 0;
    }
    g_control_poll_counter = 0U;
}

void mecanum_init(void)
{
    motor_init();
    reset_control_state();
    mecanum_stop();
}

void mecanum_stop(void)
{
    reset_control_state();
    motor_stop_all();
}

void mecanum_drive(int forward, int strafe, int rotate)
{
    int left_front = forward + strafe + rotate;
    int left_rear = forward - strafe + rotate;
    int right_front = forward - strafe - rotate;
    int right_rear = forward + strafe - rotate;
    int max = max_abs4(left_front, left_rear, right_front, right_rear);

    if (max > SPEED_MAX) {
        left_front = (left_front * SPEED_MAX) / max;
        left_rear = (left_rear * SPEED_MAX) / max;
        right_front = (right_front * SPEED_MAX) / max;
        right_rear = (right_rear * SPEED_MAX) / max;
    }

    wheel_set(MOTOR_LEFT_FRONT, left_front);
    wheel_set(MOTOR_LEFT_REAR, left_rear);
    wheel_set(MOTOR_RIGHT_FRONT, right_front);
    wheel_set(MOTOR_RIGHT_REAR, right_rear);
}

void mecanum_control_task(void)
{
    unsigned i;
    long speed_sum = 0;
    unsigned active_count = 0;
    long scale;

    if (!g_closed_loop_enabled) {
        return;
    }

    ++g_control_poll_counter;
    if (g_control_poll_counter < CONTROL_PERIOD_POLLS) {
        return;
    }
    g_control_poll_counter = 0U;

    for (i = 0U; i < (unsigned)BOARD_MOTOR_COUNT; ++i) {
        int current = normalized_encoder_count(i);
        int delta = current - (int)g_last_encoder_count[i];
        int target = g_target_speed[i];
        int abs_target = abs_int(target);

        g_last_delta[i] = delta;
        g_last_encoder_count[i] = current;

        if (abs_target > 0 && g_wheel_enable[i]) {
            speed_sum += ((long)abs_int(delta) * 1000L) / (long)abs_target;
            ++active_count;
        }
    }

    if (active_count == 0U) {
        return;
    }

    scale = speed_sum / (long)active_count;

    for (i = 0U; i < (unsigned)BOARD_MOTOR_COUNT; ++i) {
        int target = g_target_speed[i];
        int desired;
        int error;
        long correction;

        if (target == 0 || !g_wheel_enable[i]) {
            g_error_integral[i] = 0;
            continue;
        }

        desired = signed_for_target((int)((scale * (long)abs_int(target)) / 1000L), target);
        error = desired - g_last_delta[i];
        g_error_integral[i] += error;
        if (g_error_integral[i] > CONTROL_INTEGRAL_LIMIT) {
            g_error_integral[i] = CONTROL_INTEGRAL_LIMIT;
        } else if (g_error_integral[i] < -CONTROL_INTEGRAL_LIMIT) {
            g_error_integral[i] = -CONTROL_INTEGRAL_LIMIT;
        }

        correction = ((long)error * CONTROL_KP_NUM) / CONTROL_KP_DEN;
        correction += g_error_integral[i] / CONTROL_KI_DEN;
        g_motor_output[i] = clamp_speed(g_motor_output[i] + ((int)correction * g_motor_sign[i]));
        motor_set((motor_id_t)i, g_motor_output[i]);
    }
}

void mecanum_motion(mecanum_motion_t motion, int speed)
{
    switch (motion) {
    case MECANUM_MOTION_FORWARD:
        mecanum_drive(speed, 0, 0);
        break;
    case MECANUM_MOTION_BACKWARD:
        mecanum_drive(-speed, 0, 0);
        break;
    case MECANUM_MOTION_LEFT:
        mecanum_drive(0, -speed, 0);
        break;
    case MECANUM_MOTION_RIGHT:
        mecanum_drive(0, speed, 0);
        break;
    case MECANUM_MOTION_ROTATE_LEFT:
        mecanum_drive(0, 0, -speed);
        break;
    case MECANUM_MOTION_ROTATE_RIGHT:
        mecanum_drive(0, 0, speed);
        break;
    case MECANUM_MOTION_STOP:
    default:
        mecanum_stop();
        break;
    }
}

int mecanum_set_wheel_trim(unsigned wheel, int trim_permille)
{
    if (wheel >= (unsigned)BOARD_MOTOR_COUNT) {
        return 0;
    }

    if (trim_permille < WHEEL_TRIM_MIN || trim_permille > WHEEL_TRIM_MAX) {
        return 0;
    }

    g_wheel_trim[wheel] = trim_permille;
    return 1;
}

int mecanum_get_wheel_trim(unsigned wheel)
{
    if (wheel >= (unsigned)BOARD_MOTOR_COUNT) {
        return 0;
    }

    return g_wheel_trim[wheel];
}

void mecanum_set_closed_loop(int enabled)
{
    g_closed_loop_enabled = enabled ? 1 : 0;
    reset_control_state();
}

int mecanum_get_closed_loop(void)
{
    return g_closed_loop_enabled;
}

int mecanum_get_wheel_delta(unsigned wheel)
{
    if (wheel >= (unsigned)BOARD_MOTOR_COUNT) {
        return 0;
    }

    return g_last_delta[wheel];
}

int mecanum_get_wheel_output(unsigned wheel)
{
    if (wheel >= (unsigned)BOARD_MOTOR_COUNT) {
        return 0;
    }

    return g_motor_output[wheel];
}
