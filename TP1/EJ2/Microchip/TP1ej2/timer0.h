#ifndef TIMER0_H_
#define TIMER0_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

void timer0_init(void);
bool timer0_flag_250ms(void);

#endif /* TIMER0_H_ */