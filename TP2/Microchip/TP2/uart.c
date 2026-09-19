#include "uart.h"
#define F_CPU 16000000UL
#include <avr/io.h>

void UART_Init(uint32_t baudrate)
{
	// Cálculo del baudrate en modo normal
	uint16_t ubrr = (F_CPU / (16UL * baudrate)) - 1;

	// Configurar baudrate
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr & 0xFF);

	// Habilitar transmisor y receptor
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	// Formato: 8 bits de datos, sin paridad, 1 bit de stop
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendChar(char c)
{
	// Esperar hasta que el buffer de transmisión esté vacío
	while (!(UCSR0A & (1 << UDRE0)))
	{
		// Esperar
	}

	// Colocar el carácter en el registro de datos
	UDR0 = c;
}

void UART_SendString(const char *str)
{
	while (*str != '\0')
	{
		UART_SendChar(*str);
		str++;
	}
}