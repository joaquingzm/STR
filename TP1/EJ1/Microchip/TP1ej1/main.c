#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

void setup_hardware(void);
bool boton_presionado(uint8_t pin_bit);

int main(void) {
	
	setup_hardware();

	bool modo_parpadeo = false;

	while (1) {
		// esperamos a que se presionen los botones
		if (!modo_parpadeo) {
			if (boton_presionado(PIND2) || boton_presionado(PIND3)) {
				modo_parpadeo = true; // se detectó presión
			}
		}
		if (modo_parpadeo) {
			// Estado 1: Forzar PB0 en HIGH y PB1 en LOW de forma directa
			PORTB = (PORTB & ~((1 << PORTB0) | (1 << PORTB1))) | (1 << PORTB0);
			_delay_ms(250);

			// Estado 2: Forzar PB0 en LOW y PB1 en HIGH de forma directa
			PORTB = (PORTB & ~((1 << PORTB0) | (1 << PORTB1))) | (1 << PORTB1);
			_delay_ms(250);
		}
	}
}

// Configuración completa del hardware
void setup_hardware(void) {
	// PB0 y PB1 como salidas
	DDRB |= (1 << DDB0) | (1 << DDB1);
	// PD2 y PD3 como entradas
	DDRD &= ~((1 << DDD2) | (1 << DDD3));
	// resistencias Pull-Up internas en PD2 y PD3
	PORTD |= (1 << PORTD2) | (1 << PORTD3);
	// ambos LEDs totalmente encendidos al iniciar
	PORTB |= (1 << PORTB0) | (1 << PORTB1);
}

// lectura de botones
bool boton_presionado(uint8_t pin_bit) {
	if (!(PIND & (1 << pin_bit))) {      // presionado
		_delay_ms(20);
		if (!(PIND & (1 << pin_bit))) {  // rechequeo del estado
			return true;
		}
	}
	return false;
}