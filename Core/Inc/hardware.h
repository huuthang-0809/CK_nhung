#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

void initHardware(void);
void delay_ms(uint32_t ms);
void handleSystemState(void);
void USART2_Init(void);
void UART2_SendChar(char c);
void UART2_SendString(const char *s);
void UART2_SendFloat(float f, uint8_t dec);

#endif
