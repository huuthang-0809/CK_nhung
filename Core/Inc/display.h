#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

void display_init(void);
void updateDisplay(float gas_ppm, bool is_reset);
void updateDisplayStopped(void);

#endif
