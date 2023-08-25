#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#define F_CPU 2000000UL

inline void delay_ms(uint16_t ms) {
    uint32_t i;
    for (i = 0; i < ((F_CPU / 12000UL) * ms); i++)
        __asm__("nop");    
}

// Minimum is 6 us, delay will be to the nearest factor of 6 us
inline void delay_us(uint16_t us) {
    for (uint32_t i = 0; i < ((us / 6) - 1); i++) {
        __asm__("nop");
    }
}

#endif