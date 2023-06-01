#include "uart.h"
#include "stm8s.h"

void UART_init() {
    UART_BRR2 = 0x00;
    UART_BRR1 = 0x0D;
    UART_CR2 = (1 << UART_TEN) | (1 << UART_REN);
}

void UART_write(uint8_t data) {
    UART_DR = data;
    while (!(UART_SR & (1 << UART_TC)));
}

uint8_t UART_read() {
    while (!(UART_SR & (1 << UART_RXNE)));
    return UART_DR;
}

int putchar(int c) {
    UART_write(c);
    return 0;
}