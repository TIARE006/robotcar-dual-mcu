#ifndef DRIVERS_BUZZER_BUZZER_H
#define DRIVERS_BUZZER_BUZZER_H

#include "common/types.h"

void buzzer_init(void);
void buzzer_on(uint32_t frequency_hz);
void buzzer_off(void);
void buzzer_beep_ms(uint32_t frequency_hz, uint32_t duration_ms);

#endif
