// #include "lcd1602-8.h"
// #include "stm8s.h"
// #include "delay.h"

// static inline void set_data_byte(uint8_t data) {
//     PB_ODR &= 0b11000000;
//     PF_ODR &= 0b11101111;
//     PC_ODR &= 0b11011111;
//     PB_ODR |= (data & 0b00111111);
//     PF_ODR |= (((data & 0b01000000) >> 6) << 4);
//     PC_ODR |= ((data >> 7) << 5);
// }

// void LCD_init() {
//         // Setup pins
//     PB_DDR |= 0b00111111;     // Set as output
//     PB_CR1 |= 0b00111111;     // Enable as push pull

//     PC_DDR |= 0b11110000;     // Set as output
//     PC_CR1 |= 0b11110000;     // Enable as push pull

//     PF_DDR |= (1 << 4);       // Set as output
//     PF_CR1 |= (1 << 4);       // Enable as push pull

//     delay_ms(10);   // LCD takes 10 ms to initialize itself after power on

//     // RS RW 7 6 5 4 3 2 1 0
//     // Clear display 0000000001
//     PC_ODR &= ~(1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte(0b00000001);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2);    // Max lcd instruction time

//     // Function set 00 00111000
//     PC_ODR &= ~(1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte(0b00111000);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2);

//     // Display on/off control 0000001111
//     PC_ODR &= ~(1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte(0b00001111);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2);

//     // Entry mode set 0000000110
//     PC_ODR &= ~(1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte(0b00000110);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2);
// }

// void LCD_clear() {
//     // Clear display 0000000001
//     PC_ODR &= ~(1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte(0b00000001);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2); 
// }


// void LCD_write_character(char c, uint16_t display_speed_ms) {
//     // Write data to CGRAM/DDRAM
//     PC_ODR |= (1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte(c);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2);
//     delay_ms(display_speed_ms);
// }

// void LCD_write(char* data, uint8_t size, uint16_t display_speed_ms) {
//     for (uint8_t i = 0; i < size; i++) {
//         LCD_write_character(data[i], display_speed_ms);
//     }
// }

// void LCD_write_16_2(char data[32], uint16_t display_speed_ms) {
//     LCD_clear();
//     for (uint8_t i = 0; i < 32; i++) {
//         if (i == 16) { 
//             // Set address to next line 0x40
//             PC_ODR &= ~(1 << RS_PIN);
//             PC_ODR &= ~(1 << RW_PIN);
//             set_data_byte(0b11000000);
//             PC_ODR |= (1 << E_PIN);
//             PC_ODR &= ~(1 << E_PIN);
//             delay_ms(2);
//         }
//         LCD_write_character(data[i], display_speed_ms);
//     }
// }

// void LCD_write_position(char* data, uint8_t position, uint8_t num_characters, uint16_t display_speed_ms) {
//     position -= 1;
//     if (position > 15) {
//         position += 48;
//     }
//     PC_ODR &= ~(1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte((position) | 0b10000000);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2);

//     for (uint8_t i = 0; i < num_characters; i++) {
//         LCD_write_character(data[i], display_speed_ms);
//     }

//     // Move cursor back
//     PC_ODR &= ~(1 << RS_PIN);
//     PC_ODR &= ~(1 << RW_PIN);
//     set_data_byte(0b01011100);
//     PC_ODR |= (1 << E_PIN);
//     PC_ODR &= ~(1 << E_PIN);
//     delay_ms(2);
// }