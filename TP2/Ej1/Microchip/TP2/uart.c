#include "uart.h"
#define F_CPU 16000000UL
#include <avr/io.h>

void UART_Init(uint32_t baudrate)
{
	// Cálculo del baudrate en modo normal
	// Sabiendo que BAUD= F_CPU/16( UBRR + 1 ) en modo normal
	// Utilizamos 16UL para que no desborde la multiplicación del denominador
	uint16_t ubrr = (F_CPU / (16UL * baudrate)) - 1;

	// Configurar baudrate
	UBRR0H = (uint8_t)((ubrr >> 8) & 0x0F); //Según datasheet debo dejar en cero los 4 bits mas significativos
	UBRR0L = (uint8_t)(ubrr & 0xFF);

	// Habilitar transmisor y receptor
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	// Formato: 8 bits de datos, sin paridad, 1 bit de stop (8N1)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendChar(char c)
{
	// Esperar hasta que el buffer de transmisión esté vacío
	while (!(UCSR0A & (1 << UDRE0)))
	{
		
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