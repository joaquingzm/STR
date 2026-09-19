#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void UART_Init(uint32_t baudrate);

void UART_SendChar(char c);

void UART_SendString(const char *str);

#endif /* UART_H_ */