#include "i2c.h"
#include "delay.h"

static inline void pulse_scl() {
    PB_ODR |= 0b00010000; 
    delay_us(i2c_delay);
    PB_ODR &= ~0b00010000;
}

void I2C_init() {
    // SCL PB4, SDA PB5
    PB_DDR = 0b11111111;    // all outputs
    PB_ODR = 0b00110000;
    PB_CR1 = 0b00000000;    // floating/open drain
    PB_CR2 = 0b00000000;    // no interrupts
    delay_ms(100);
}

void I2C_start() {
    // Start condition
    delay_us(i2c_delay);

    PB_ODR &= ~0b00100000;
    delay_us(i2c_delay);
    PB_ODR &= ~0b00010000; 
}

void I2C_stop() {
    PB_ODR &= ~0b00100000;   // Pull down for stop condition
    delay_us(i2c_delay);
    
    // Stop Condition
    PB_ODR |= 0b00010000;
    delay_us(i2c_delay);
    PB_ODR |= 0b00100000;
}

void I2C_write_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        uint8_t next_bit = (data >>  7 - i) & 0x01;
        PB_ODR &= ~0b00100000;
        PB_ODR |= next_bit << 5;
        delay_us(i2c_delay);
        pulse_scl();
    }

    // ACK follows directly here
    PB_ODR |= 0b00100000;   // Release SDA line
    PB_DDR &= ~0b00100000;  // Set input
    delay_us(i2c_delay);
    pulse_scl();

    while(!(PB_IDR & 0b00100000) >> 5);   // Wait for ACK
    PB_DDR |= 0b00100000;   // Set output
}
