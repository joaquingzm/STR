#include "uart.h"
#include <avr/io.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
void UART_Init(uint32_t baudrate)
{
    uint16_t divisor = (F_CPU / (16UL * baudrate)) - 1;
    UCSR0A = 0; /* Velocidad normal, U2X0=0. */
    UBRR0H = (uint8_t)(divisor >> 8);
    UBRR0L = (uint8_t)divisor;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8N1. */
}
void UART_SendChar(char c)
{
    while (!(UCSR0A & (1 << UDRE0))) { }
    UDR0 = c;
}
void UART_SendString(const char *texto)
{
    while (*texto != '\0')
        UART_SendChar(*texto++);
}
