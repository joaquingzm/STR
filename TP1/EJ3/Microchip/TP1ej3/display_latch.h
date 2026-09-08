#ifndef DISPLAY_LATCH_H_
#define DISPLAY_LATCH_H_

#include <avr/io.h>

void display_latch_init(void);
void display_mostrar_hex(uint16_t valor);

#endif /* DISPLAY_LATCH_H_ */