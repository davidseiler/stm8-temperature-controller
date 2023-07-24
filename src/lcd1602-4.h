#ifndef LCD16024_H
#define LCD16024_H

#include <stdint.h>

/* HD44780 LCD1602 operating in 4-bit parallel mode */

/* Pin mappings
LCD1602 -- STM8S207K8   
RS -> PA3
RW -> PD6
E  -> PA1

D4 -> PC4
D5 -> PC5
D6 -> PC6
D7 -> PC7
*/

#define RS_PIN 3
#define RW_PIN 6
#define E_PIN 1

/* Initialize the lcd with default settings */
void LCD_init();

/* Clear display and set cursor to home location */
void LCD_clear();

/* Write a single character to the display where the at the current cursor position */
void LCD_write_character(char c, uint16_t display_speed_ms);

/* Write full display (16x2) with refresh */
void LCD_write_16_2(char data[32], uint16_t display_speed_ms);

/* Write data of any size to the LCD's DDRAM. This device is limited to 80 bytes,
 but only displays addresses 0-15 and 40-56*/
void LCD_write(char* data, uint8_t size, uint16_t display_speed_ms);

/* Write a single character and a specfic position on the display.
Positions start at 1 and go to 32*/
void LCD_write_position(char* data, uint8_t position, uint8_t num_characters, uint16_t display_speed_ms);

#endif