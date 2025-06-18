#ifndef ALARM_H
#define ALARM_H

#include <stdint.h>

typedef enum {
    STATE_SAFE,
    STATE_WARNING,
    STATE_DANGER,
    STATE_EXTREME_DANGER
} SystemState;

extern const char* stateStrings[];

void handleAlarm(float gas_ppm);
SystemState getSystemState(float gas_ppm);

#endif
