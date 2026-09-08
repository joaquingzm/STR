#include "display_latch.h"
#include <util/delay.h>

void display_latch_init(void) {
	// PORTB como salidas
	DDRB = 0xFF; PORTB = 0x00;

	// habilitar PC1 y PC2 como salidas para el control de los latches
	DDRC |= (1 << DDC1) | (1 << DDC2);
	PORTC &= ~((1 << PORTC1) | (1 << PORTC2));
}

void display_mostrar_hex(uint16_t valor) { // se trae el valor del adcread por parametro
	// carga de dígitos bajos en Latch U4 (PC1)
	PORTB = (uint8_t)(valor & 0x00FF); // para quedarnos solo con los bits mas bajos
	
	// pulso para pasar los datos al latch
	PORTC |= (1 << PORTC1);
	PORTC &= ~(1 << PORTC1);

	// carga de dígitos altos en Latch U3 (PC2)
	PORTB = (uint8_t)((valor >> 8) & 0x03); // para quedarnos solo con los bits mas altos
	
	// pulso para pasar los datos al latch
	PORTC |= (1 << PORTC2);
	PORTC &= ~(1 << PORTC2);
}
