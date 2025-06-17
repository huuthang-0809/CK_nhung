#include "stm32f4xx.h"
#include "hardware.h"
#include "sensor.h"
#include "display.h"
#include "alarm.h"

volatile uint8_t system_running = 1;

int main(void) {
    initHardware();
    display_init();
    while (1) {
        handleSystemState();
        if (system_running) {
            float gas_ppm;
            if (readGasSensor(&gas_ppm)) {
                handleAlarm(gas_ppm);
                updateDisplay(gas_ppm);
            }
        } else {
            updateDisplayStopped();
            GPIOB->ODR |= (1 << 4);  // LED xanh lá
        }
        delay_ms(1000);
    }
}
