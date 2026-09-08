#define F_CPU 16000000UL
#include <avr/io.h>
// #include <util/delay.h>
#include "adc.h"
#include "display_latch.h"

int main(void) {
	// inicialización de periféricos
	adc_init();
	display_latch_init();

	uint16_t valor_adc = 0;

	while (1) {
		// polling 
		valor_adc = adc_read();

		// enviar valor a displays
		display_mostrar_hex(valor_adc);

		// _delay_ms(50); // Tiempo entre lecturas para no saturar el bus
	}
}