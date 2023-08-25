#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "stm8s.h"

/* Library for master only I2C operations */

#define I2C_WRITE 0x00
#define i2c_delay 6


/* Initialize I2C bus for master only operation */
void I2C_init();

/* Send I2C start command */
void I2C_start();

/* Send I2C stop command */
void I2C_stop();

/* Write data byte to the I2C bus and wait for response */
void I2C_write_byte(uint8_t data);

#endif