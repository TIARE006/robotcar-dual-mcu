#ifndef DRIVERS_ENCODER_ENCODER_H
#define DRIVERS_ENCODER_ENCODER_H

#include "bsp/board.h"
#include "common/types.h"

void encoder_init(void);
void encoder_poll(void);
void encoder_reset_all(void);
int32_t encoder_get_count(board_motor_id_t motor);

#endif
