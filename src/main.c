#include "stm8s.h"
#include "delay.h"
#include "uart.h"
#include "ds18b20.h"
#include "fan.h"
#include "lcd.h"

#define NUM_SENSORS 3
#define CONTROL_LOOP_INTERVAL 500

// Port setup for rotary encoder
#define PD_ISR              6   // interrupt irq for Port D external
#define PD_ENCODER_A        2   // EC2
#define PD_ENCODER_B        3   // EC1
#define PD_ENCODER_C        1   // Pin for EC3 encoder push button

/*
Rotary Encoder -- STM8S
EC1 -> PD3
EC2 -> PD2
EC3 -> PD1
*/

// Since there is limited EEPROM space for global variables, hard code the global into the first RAM address
#define target_temp (*(volatile uint8_t *)(0x0000))


void encoder_triggered(void) __interrupt(PD_ISR) {
    PD_CR2 &= ~(1 << PD_ENCODER_A);     // Disable interrupts
    uint8_t b_val = (PD_IDR & (1 << PD_ENCODER_B)) >> PD_ENCODER_B;  // Read value of B line

    // When b=1 => counter clockwise when b=0 =>clockwise
    if (!b_val) {
        if (target_temp < 255) {
            target_temp += 5;
        }
    } else {
        if (target_temp > 0) {
            target_temp -= 5;
        }
    }

    char temp[4];
    sprintf(temp, "%3d", target_temp);
    LCD_write_position(temp, (uint8_t)19, 3, 0);

    // Wait for both lines to bounce up
    while (!((PD_IDR & (1 << PD_ENCODER_B)) >> PD_ENCODER_B) || !((PD_IDR & (1 << PD_ENCODER_A)) >> PD_ENCODER_A)); 

    PD_CR2 |= (1 << PD_ENCODER_A);     // Re-enable interrupts
}

// Control Loop
static void monitor_temp(uint8_t* rom_bytes, uint16_t frequency_ms) {
    uint8_t fan_speed = 1;
    static uint8_t scratchpad[9 * NUM_SENSORS];

    while(1) {
        int16_t average_temp = 0;
        uint16_t average_decimal_temp = 0;
        PWM_set_duty_cycle(fan_speed);

        ONE_WIRE_convertTemperature();

        // Address each sensor and fetch temperature
        for (uint8_t i = 0; i < NUM_SENSORS; i++) {
            ONE_WIRE_readScratchPad((rom_bytes + (i * 8)), (scratchpad + (i * 9)));

            average_temp += ONE_WIRE_whole_temperature((scratchpad + (i * 9) + 7));
            average_decimal_temp += ONE_WIRE_decimal_temperature((scratchpad + (i * 9) + 7));
            if (average_decimal_temp >= 10000) {
                average_temp += 1;
                average_decimal_temp -= 10000;
            }
        }
        average_temp /= NUM_SENSORS;
        average_decimal_temp /= NUM_SENSORS;

        // Display temp values
        char avg_temp_string[9];
        sprintf(avg_temp_string, "%2d.%04d", average_temp, average_decimal_temp);

        char temps[36];
        sprintf(temps, "Avg: %.5s%cC    T:%3d%cC Fan:%3d%c", avg_temp_string, 0xDF, target_temp, 0xDF, ((fan_speed * 100) / 80) - 1, 0x25);
        LCD_clear();
        LCD_write_16_2(temps, 0);

        // Update fan speed to reach temperature targets
        if (average_temp >= target_temp && fan_speed < 80) {
            fan_speed = fan_speed + 10;
        } else if (average_temp < target_temp && fan_speed != 1) {
            fan_speed = fan_speed - 10;
        }

        // Clear scratchpad memory
        for (int i = 0; i < NUM_SENSORS * 9; i++) {
            scratchpad[i] = 0;
        }
        delay_ms(frequency_ms);
    }
}

void main() {
    #if USE_UART
        UART_init();
    #endif

    target_temp = 25;
    
    LCD_init();
    LCD_write_16_2("Initializing....                ", 4);
    PWM_init();
    PWM_start();
    ONE_WIRE_init();

    // Setup ports for adjusting target temperature
    __asm__("rim");         // Enable interrupts
    PD_DDR &= ~(1 << PD_ENCODER_A);     // Enable as input
    PD_CR1 |= (1 << PD_ENCODER_A);      // Enable as pull up
    PD_CR2 |= (1 << PD_ENCODER_A);      // Enable interrupts
    PD_DDR &= ~(1 << PD_ENCODER_B);     // Enable as input
    PD_CR1 |= (1 << PD_ENCODER_B);      // Enable as pull up

    // Fetch the ROM's of all the devices on the bus
    static uint8_t rom_bytes[NUM_SENSORS * 8];
    // Clear rom_bytes memory
    for (int i = 0; i < NUM_SENSORS * 8; i++) {
        rom_bytes[i] = 0;
    }

    ONE_WIRE_searchROMs(rom_bytes, NUM_SENSORS);

    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        PRINTF("\nROM%d SERIAL NUMBER: 0x", i); 
        for (uint8_t j = 0; j < 8; j++) {
            PRINTF("%02x", rom_bytes[j + i * 8]);
        }
    }
    PRINTF("\n");

    // Start temperature controlling
    monitor_temp(rom_bytes, CONTROL_LOOP_INTERVAL);
}