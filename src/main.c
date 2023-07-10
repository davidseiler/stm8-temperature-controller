#include "stm8s.h"
#include "delay.h"
#include "uart.h"
#include "ds18b20.h"
#include "lcd1602-8.h"
#include "fan.h"

#define NUM_SENSORS 3

// Use Port A and C: PA1 and PC1 for setting the target temperature
#define PA_ISR              3   // EXTI0 interrupt irq for Port A external
#define PA_ENCODER_A        2
#define PC_ISR              5   // EXTI2 interrupt irq for Port C external
#define PC_ENCODER_B        1
#define PD_ISR              6   // interrupt irq for Port D external
#define PD_ENCODER_C        2   // Pin for EC3 encoder push button


// Temperature related values
volatile uint8_t target_temp = 25;
volatile uint8_t kp = 1;
volatile uint8_t ki = 1;
volatile uint8_t kd = 1;

// TODO: interrupts during LCD operations mess things up, to the point that reset is required,
// I think what is happening is LCD_write_position doesn't return the memory section to the correct spot,
// so it is possible after the interrupt is exited the next text is randomly sent somewhere else
void encoder_triggered(void) __interrupt(PA_ISR) {
    PA_CR2 &= ~(1 << PA_ENCODER_A);     // Disable interrupts
    uint8_t b_val = (PC_IDR & (1 << PC_ENCODER_B)) >> PC_ENCODER_B;  // Read value of B line
    
    // When b=0 => counter clockwise when b=1 =>clockwise
    if (b_val) {
        if (target_temp < 255) {
            target_temp += 5;
        }
    } else {
        if (target_temp > 0) {
            target_temp -= 5;
        }
    }

    char temp[4];
    sprintf(temp,"%3d", target_temp);
    LCD_write_position(temp, (uint8_t)19, 3, 0);

    // Wait for both lines to bounce up
    while (!((PC_IDR & (1 << PC_ENCODER_B)) >> PC_ENCODER_B) || !((PA_IDR & (1 << PA_ENCODER_A)) >> PA_ENCODER_A)); 

    PA_CR2 |= (1 << PA_ENCODER_A);     // Enable interrupts
}

void encoder_click(void) __interrupt(PD_ISR) {
    PD_CR2 &= ~(1 << PD_ENCODER_C);     // Disable interrupts
    
    pid_menu();
    // Wait for button to bounce back
    while (!((PD_IDR & (1 << PD_ENCODER_C)) >> PD_ENCODER_C)); 
    delay_ms(50);   // Ignore double bounces

    PD_CR2 |= (1 << PD_ENCODER_C);     // Enable interrupts
}

static void pid_menu() {
    uint8_t selected = 0;
    LCD_clear();
    char message[32];
    sprintf(message, "This is the menuP:%2d, I:%2d, D:%2d", kp, ki, kp);
    LCD_write_16_2(message, 0);
}

static void monitor_temp(uint8_t* rom_bytes, uint16_t frequency_ms) {
    uint8_t fan_speed = 1;
    while(1) {
        int16_t average_temp = 0;
        uint16_t average_decimal_temp = 0;
        PWM_set_duty_cycle(fan_speed);

        ONE_WIRE_convertTemperature();

        uint8_t* scratchpad = calloc(9 * NUM_SENSORS, sizeof(uint8_t));

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
        // sprintf(temps, "%.5s%cC %.5s%cC %.5s%cC Fan:%3d%c", t1 + 2, 0xDF, t2 + 2, 0xDF, t3 + 2, 0xDF, ((fan_speed * 100) / 80) - 1, 0x25);
        sprintf(temps, "Avg: %.5s%cC    T:%3d%cC Fan:%3d%c", avg_temp_string, 0xDF, target_temp, 0xDF, ((fan_speed * 100) / 80) - 1, 0x25);
        LCD_clear();
        LCD_write_16_2(temps, 0);

        // Update fan speed to reach temperature target
        if (average_temp >= target_temp && fan_speed < 80) {
            fan_speed = fan_speed + 10;
        } else if (average_temp < target_temp && fan_speed != 1) {
            fan_speed = fan_speed - 10;
        }

        free(scratchpad);
        delay_ms(frequency_ms);
    }
} 

void main() {
    #if USE_UART
        UART_init();
    #endif

    LCD_init();
    PWM_init();
    PWM_start();
    ONE_WIRE_init();

    // Setup Port A and C: A1 C1 for adjusting target temperature
    __asm__("rim");         // Enable interrupts
    PA_DDR &= ~(1 << PA_ENCODER_A);     // Enable as input
    PA_CR1 |= (1 << PA_ENCODER_A);      // Enable as pull up
    PA_CR2 |= (1 << PA_ENCODER_A);      // Enable interrupts
    PC_DDR &= ~(1 << PC_ENCODER_B);     // Enable as input
    PC_CR1 |= (1 << PC_ENCODER_B);      // Enable as pull up
    PD_DDR &= ~(1 << PD_ENCODER_C);     // Enable as input
    PD_CR1 |= (1 << PD_ENCODER_C);      // Enable as pull up
    PD_CR2 |= (1 << PD_ENCODER_C);      // Enable interrupts

    // Fetch the ROM's of all the devices on the bus
    uint8_t* rom_bytes = calloc(NUM_SENSORS * 8, sizeof(uint8_t));
    ONE_WIRE_searchROMs(rom_bytes, NUM_SENSORS);

    // Expected DS18B20 serial numbers
    // 0xc1a967770e64ff28
    // 0xe29815740e64ff28
    // 0x3f760e770e64ff28
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        PRINTF("\nROM%d SERIAL NUMBER: 0x", i); 
        for (uint8_t j = 0; j < 8; j++) {
            PRINTF("%02x", rom_bytes[j + i * 8]);
        }
    }
    PRINTF("\n");

    // Start temperature controlling
    monitor_temp(rom_bytes, 500);
    free(rom_bytes);
}