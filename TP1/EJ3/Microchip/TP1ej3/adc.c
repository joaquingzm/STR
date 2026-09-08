#include "adc.h"

void adc_init(void) {
	// AVCC con capacitor en AREF (REFS0 = 1), canal ADC0: MUX3..0 = 0000 (por defecto), ADLAR = 0 (por defecto)
	ADMUX = (1 << REFS0);

	// habilitar adc, ADEN=1, prescaler de 128: ADPS2=1, ADPS1=1, ADPS0=1 (16MHz / 128 = 125kHz) -> rango recomendado
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(void) {
	// start conversion=1 (iniciar conversion)
	ADCSRA |= (1 << ADSC);

	// polling: esperar hasta que el bit ADSC vuelva a 0 
	while (ADCSRA & (1 << ADSC));

	return ADC; // ADCL primero y luego ADCH en el orden correcto
}