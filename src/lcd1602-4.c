#include "lcd1602-4.h"
#include "stm8s.h"
#include "delay.h"

static inline void set_data_bits(uint8_t data) {
    PB_ODR &= 0b11001111;
    PF_ODR &= 0b11101111;
    PC_ODR &= 0b11011111;
    PB_ODR |= ((data & 0b00000001) << 4); // PB4
    PB_ODR |= ((data & 0b00000010) << 4); // PB5
    PF_ODR |= (((data & 0b000000100)) << 2); // PF4
    PC_ODR |= ((data & 0b00001000) << 2);   // PC5
}

void send_instruction(uint8_t flags, uint8_t data) {    // flags: 0b000000(RS)(RW)
    PC_ODR &= ~(1 << RS_PIN);
    PC_ODR &= ~(1 << RW_PIN);
    uint8_t first = data >> 4;
    PC_ODR |= ((flags >> 1) << RS_PIN);
    PC_ODR |= ((flags & 0b00000001) << RW_PIN);
    set_data_bits(first);
    PC_ODR |= (1 << E_PIN);
    PC_ODR &= ~(1 << E_PIN);
    delay_ms(2);
    // Send the next 4 bits of the instruction
    set_data_bits(data);
    PC_ODR |= (1 << E_PIN);
    PC_ODR &= ~(1 << E_PIN);
    delay_ms(2);
}

// TODO: tidy so that it can be committed
// TODO: Update LCD to be configurable

void init_by_instruction() {    // required after reset in 4-bit mode
    set_data_bits(0b1100);
    PC_ODR |= (1 << E_PIN);
    PC_ODR &= ~(1 << E_PIN);
    delay_ms(5);

    PC_ODR |= (1 << E_PIN);
    PC_ODR &= ~(1 << E_PIN);
    delay_us(150);

    PC_ODR |= (1 << E_PIN);
    PC_ODR &= ~(1 << E_PIN);
}

void LCD_init() {
    PB_DDR |= 0b00110000;     // Set as output
    PB_CR1 |= 0b00110000;     // Enable as push pull

    PC_DDR |= 0b11110000;     // Set as output
    PC_CR1 |= 0b11110000;     // Enable as push pull

    PF_DDR |= (1 << 4);       // Set as output
    PF_CR1 |= (1 << 4);       // Enable as push pull

    delay_ms(20);   // LCD takes 10 ms to initialize itself after power on

    init_by_instruction();

    // RS RW 7 6 5 4 3 2 1 0
    // Force into 4 bit operation mode
    PC_ODR &= ~(1 << RS_PIN);
    PC_ODR &= ~(1 << RW_PIN);
    // set_data_bits(0b0010);
    PF_ODR |= (1 << 4);

    PC_ODR |= (1 << E_PIN);
    PC_ODR &= ~(1 << E_PIN);
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

void LCD_clear() {
    send_instruction(0, 1);
}

void LCD_write_character(char c, uint16_t display_speed_ms) {
    // Write data to CGRAM/DDRAM
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