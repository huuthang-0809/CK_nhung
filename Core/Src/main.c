#include "stm32f4xx.h"
#include "hardware.h"
#include "sensor.h"
#include "display.h"
#include "alarm.h"

volatile uint8_t system_running = 1;
volatile uint32_t safe_timer = 0;

int main(void) {
    initHardware();
    display_init();
    while (1) {
        handleSystemState();
        if (system_running) {
            float gas_ppm;
            if (readGasSensor(&gas_ppm)) {
                if (safe_timer > 0) {
                    // Trạng thái reset: Đặt hệ thống về trạng thái an toàn tạm thời
                    updateDisplay(0.0, true);
                    safe_timer--; // Giảm thời gian reset
                } else {
                    // Hoạt động bình thường
                	//if(safe_timer == 0) {
                        TIM2->CR1 |= TIM_CR1_CEN;  // Bật counter của TIM2
                        NVIC_EnableIRQ(TIM2_IRQn); // Bật ngắt của TIM2
                	//}
                    handleAlarm(gas_ppm);
                    updateDisplay(gas_ppm, false);
                }
            }
        } else {
            updateDisplayStopped();
            GPIOB->ODR |= (1 << 4);  // LED xanh lá
        }
        delay_ms(1000);
    }
}
