#include "timer0.h"

static volatile bool flag_250ms = false;
static volatile uint16_t contador_ticks = 0;

void timer0_init(void) {
	// Modo CTC
	TCCR0A = (1 << WGM01);
	
	// Prescaler de 64 
	TCCR0B = (1 << CS01) | (1 << CS00);
	
	// 249 + 1 = 250 ticks -> (64 * 250) / 16MHz = 1 ms 
	OCR0A = 249;
	
	// Habilitar interrupción 
	TIMSK0 |= (1 << OCIE0A);
}

bool timer0_flag_250ms(void) {
	if (flag_250ms) {
		flag_250ms = false;
		return true;
	}
	return false;
}

ISR(TIMER0_COMPA_vect) {
	contador_ticks++;
	if (contador_ticks >= 250) { // 250 ms exactos
		contador_ticks = 0;
		flag_250ms = true;
	}
}