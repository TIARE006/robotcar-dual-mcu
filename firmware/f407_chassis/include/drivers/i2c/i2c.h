#ifndef DRIVERS_I2C_I2C_H
#define DRIVERS_I2C_I2C_H

#include "common/types.h"

void i2c2_init(void);
int i2c2_probe(uint8_t address_7bit);

#endif
