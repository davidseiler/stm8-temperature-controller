#include "lcd.h"
#include "stm8s.h"
#include "delay.h"

#ifndef I2C
   #define I2C 0
#endif
#if I2C
    // Configure for the I2C display
    #include "i2c.h"

    static void inline pulse_enable(uint8_t data) {
        I2C_start();
        I2C_write_byte(0x4E);
        I2C_write_byte(0b0001100 | data);
        I2C_stop();
        delay_ms(2);

        I2C_start();
        I2C_write_byte(0x4E);
        I2C_write_byte(~0b00000100 & data);
        I2C_stop();
    }

    static void init_by_instruction() {
        I2C_start();
        I2C_write_byte(0x4E);

        I2C_write_byte(0b00110000);
        I2C_stop();

        pulse_enable(0b00110000);
        delay_ms(6);

        pulse_enable(0b00110000);
        delay_us(150);
        pulse_enable(0b00110000);
    }

    static void send_instruction(uint8_t flags, uint8_t data) { // flags 0b000000(RS)(RW)
        // For the I2C controller the bits are reversed since it is MSB first
        // D7 D6 D5 D4 BL E RW RS

        // Swap flags
        flags = flags >> 1 | (flags << 1) & 0b00000010;
        // Keep the display backlight pin enabled
        flags |= 0b00001000;

        I2C_start();
        I2C_write_byte(0x4E);
        
        uint8_t first = data >> 4;
        I2C_write_byte((first << 4) | flags);

        I2C_stop();

        pulse_enable((first << 4) | flags);
        delay_ms(2);

        I2C_start();
        I2C_write_byte(0x4E);

        // Send the next 4 bits of the instruction
        I2C_write_byte((data << 4) | flags);
        I2C_stop();

        pulse_enable((data << 4) | flags);
        delay_ms(2);
        
    }

    void LCD_init() {
        I2C_init();

        init_by_instruction();

        // Force into 4 bit mode
        // RS R/W DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
        send_instruction(0, 0b00100000);
        pulse_enable(0b001000000);
        delay_ms(4);

        // Function set 00 00101000: 4 bit operations, on 2 lines
        send_instruction(0, 0b00101000);

        // Display on/off control 00 00001111
        send_instruction(0, 0b00001111);

        // Entry mode set 00 00000110
        send_instruction(0, 0b00000110);

        // Clear display
        send_instruction(0, 1);
    }
#else
    // Configure for 4-bit parallel display
    static inline void set_data_bits(uint8_t data) {
        PC_ODR &= 0b00001111;
        PC_ODR |= ((data & 0b00001111) << 4);
    }

    void send_instruction(uint8_t flags, uint8_t data) {    // flags: 0b000000(RS)(RW)
        PA_ODR &= ~(1 << RS_PIN);
        PD_ODR &= ~(1 << RW_PIN);
        uint8_t first = data >> 4;
        PA_ODR |= ((flags >> 1) << RS_PIN);
        PD_ODR |= ((flags & 0b00000001) << RW_PIN);
        set_data_bits(first);
        PA_ODR |= (1 << E_PIN);
        PA_ODR &= ~(1 << E_PIN);
        delay_ms(2);
        // Send the next 4 bits of the instruction
        set_data_bits(data);
        PA_ODR |= (1 << E_PIN);
        PA_ODR &= ~(1 << E_PIN);
        delay_ms(2);
    }

    void init_by_instruction() {    // Required after reset in 4-bit mode
        set_data_bits(0b1100);
        PA_ODR |= (1 << E_PIN);
        PA_ODR &= ~(1 << E_PIN);
        delay_ms(5);

        PA_ODR |= (1 << E_PIN);
        PA_ODR &= ~(1 << E_PIN);
        delay_us(150);

        PA_ODR |= (1 << E_PIN);
        PA_ODR &= ~(1 << E_PIN);
    }

    void LCD_init() {
        // RS and E
        PA_DDR |= 0b00001010;   // Set as output
        PA_CR1 |= 0b00001010;   // Enable as push pull

        // RW
        PD_DDR |= 0b01000000;   // Set as output
        PD_CR1 |= 0b01000000;   // Enable as push pull

        // DB4-DB7
        PC_DDR |= 0b11110000;   // Set as output
        PC_CR1 |= 0b11110000;   // Enable as push pull

        delay_ms(20);   // LCD takes 10 ms to initialize itself after power on

        init_by_instruction();

        // RS RW 7 6 5 4 3 2 1 0
        // Force into 4 bit operation mode
        PA_ODR &= ~(1 << RS_PIN);
        PD_ODR &= ~(1 << RW_PIN);

        PC_ODR |= (1 << 6);

        PA_ODR |= (1 << E_PIN);
        PA_ODR &= ~(1 << E_PIN);
        delay_ms(4);

        // Function set 00 00101000: 4 bit operations, on 2 lines
        send_instruction(0, 0b00101000);

        // Display on/off control 00 00001111
        send_instruction(0, 0b00001111);

        // Entry mode set 00 00000110
        send_instruction(0, 0b00000110);

        // Clear display
        send_instruction(0, 1);
    }
#endif

void LCD_clear() {
    send_instruction(0, 1);
}

void LCD_write_character(char c, uint16_t display_speed_ms) {
    send_instruction(2, c);
    delay_ms(display_speed_ms);
}

void LCD_write_16_2(char data[32], uint16_t display_speed_ms) {
    LCD_clear();
    for (uint8_t i = 0; i < 32; i++) {
        if (i == 16) { 
            // Set address to next line 0x40
            send_instruction(0, 0b11000000);
        }
        LCD_write_character(data[i], display_speed_ms);
    }
}

void LCD_write(char* data, uint8_t size, uint16_t display_speed_ms) {
    for (uint8_t i = 0; i < size; i++) {
        LCD_write_character(data[i], display_speed_ms);
    }
}

void LCD_write_position(char* data, uint8_t position, uint8_t num_characters, uint16_t display_speed_ms) {
    position -= 1;
    if (position > 15) {
        position += 48;
    }
    send_instruction(0, (position) | 0b10000000);

    for (uint8_t i = 0; i < num_characters; i++) {
        LCD_write_character(data[i], display_speed_ms);
    }
    // Move cursor back
    send_instruction(0, 0b01011100);
}