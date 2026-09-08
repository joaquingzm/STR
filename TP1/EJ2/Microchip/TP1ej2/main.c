#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "timer0.h"

void setup_hardware(void);
bool boton_presionado(uint8_t pin_bit);

int main(void) {
	setup_hardware();
	timer0_init();
	sei();

	bool modo_parpadeo = false;
	uint8_t estado_led = 0; // 0: PB0 ON / PB1 OFF,  1: PB0 OFF / PB1 ON

	while (1) {
		// modo inicial
		if (!modo_parpadeo) {
			if (boton_presionado(PIND2) || boton_presionado(PIND3)) {
				modo_parpadeo = true;
			}
		}

		// flag de 250ms
		if (modo_parpadeo && timer0_flag_250ms()) {
			if (estado_led == 0) {
				// estado 1: PB0 encendido, PB1 apagado
				PORTB = (PORTB & ~(1 << PORTB1)) | (1 << PORTB0);
				estado_led = 1; // Siguiente estado
				} else {
				// estado 2: PB0 apagado, PB1 encendido
				PORTB = (PORTB & ~(1 << PORTB0)) | (1 << PORTB1);
				estado_led = 0; // Siguiente estado
			}
		}
	}
}

void setup_hardware(void) {
	DDRB |= (1 << DDB0) | (1 << DDB1);           // PB0 y PB1 como salidas
	DDRD &= ~((1 << DDD2) | (1 << DDD3));        // PD2 y PD3 como entradas
	PORTD |= (1 << PORTD2) | (1 << PORTD3);      // Pull-ups activadas

	PORTB |= (1 << PORTB0) | (1 << PORTB1);      // Estado inicial: Ambos LEDs ON
}

bool boton_presionado(uint8_t pin_bit) {
	if (!(PIND & (1 << pin_bit))) {
		_delay_ms(20);
		if (!(PIND & (1 << pin_bit))) {
			return true;
		}
	}
	return false;
}