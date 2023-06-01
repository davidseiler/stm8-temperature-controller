#include <stdint.h>
#include <stdio.h>

#ifndef USE_UART
   #define USE_UART 0
#endif

#define UART_TXE    7
#define UART_TC     6
#define UART_RXNE   5

#define UART_TEN    3
#define UART_REN    2

#if USE_UART
   #define PRINTF(...)     printf(__VA_ARGS__)
#else
   #define PRINTF(...)
#endif

/* Configure STM8S uart */
void UART_init();

/* Write data to the serial port */
void UART_write(uint8_t data);

/* Read data from the serial port */
uint8_t UART_read();

/* Tells printf to print to the uart */
int putchar(int c);