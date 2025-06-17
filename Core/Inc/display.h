#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void display_init(void);
void updateDisplay(float gas_ppm);
void updateDisplayStopped(void);

#endif
