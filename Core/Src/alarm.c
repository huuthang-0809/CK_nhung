#include "alarm.h"
#include "stm32f4xx.h"
#include "config.h"
#include "hardware.h"

static volatile uint8_t led_red_enable = 0;
const char* stateStrings[] = {"SAFE", "WARNING", "DANGER", "DANGER++"};

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF; // Xóa cờ ngắt
        if (led_red_enable) {
            GPIOB->ODR ^= (1 << 1);  // Toggle LED đỏ
        } else {
            GPIOB->ODR &= ~(1 << 1);
        }
    }
}

void handleAlarm(float gas_ppm) {
    if (gas_ppm > THRESHOLD_DANGER) {
        GPIOB->ODR &= ~((1 << 2) | (1 << 3) | (1 << 4));
        GPIOA->ODR |= (1 << 1) | (1 << 5);
        GPIOA->ODR |= (1 << 6);
        delay_ms(100);
        GPIOA->ODR &= ~(1 << 6);
        int base_ppm = (int)gas_ppm;
        int step = (base_ppm - THRESHOLD_DANGER) / 1000;
        int blink_period = 500 - (step * 50);
        if (blink_period < 100) blink_period = 100;
        TIM2->ARR = blink_period;
        TIM2->EGR |= TIM_EGR_UG; // Tạo sự kiện cập nhật
        GPIOB->ODR |= (1 << 1);
        led_red_enable = 1;
        TIM2->CR1 |= TIM_CR1_CEN;  // Bật counter
        TIM2->DIER |= TIM_DIER_UIE; // Bật ngắt cập nhật
        NVIC_EnableIRQ(TIM2_IRQn);  // Bật ngắt NVIC
    } else if (gas_ppm > THRESHOLD_WARNING) {
        GPIOB->ODR &= ~((1 << 2) | (1 << 3) | (1 << 4));
        GPIOA->ODR |= (1 << 1) | (1 << 5);
        GPIOA->ODR |= (1 << 6);
        delay_ms(100);
        GPIOA->ODR &= ~(1 << 6);
        TIM2->ARR = 1000;
        TIM2->EGR |= TIM_EGR_UG; // Tạo sự kiện cập nhật
        GPIOB->ODR |= (1 << 1);
        led_red_enable = 1;
        TIM2->CR1 |= TIM_CR1_CEN;  // Bật counter
        TIM2->DIER |= TIM_DIER_UIE; // Bật ngắt cập nhật
        NVIC_EnableIRQ(TIM2_IRQn);  // Bật ngắt NVIC
        UART2_SendString("Warning: TIM2 enabled, ARR=1000\n");
    } else if (gas_ppm > THRESHOLD_SAFE) {
        GPIOB->ODR &= ~((1 << 1) | (1 << 3) | (1 << 4));
        GPIOA->ODR |= (1 << 1) | (1 << 5);
        GPIOB->ODR |= (1 << 2);
        GPIOA->ODR |= (1 << 6);
        delay_ms(100);
        GPIOA->ODR &= ~(1 << 6);
        led_red_enable = 0;
        TIM2->CR1 &= ~TIM_CR1_CEN; // Tắt counter
        TIM2->DIER &= ~TIM_DIER_UIE; // Tắt ngắt
        UART2_SendString("Safe: LED off, TIM2 disabled\n");
    } else {
        GPIOB->ODR &= ~((1 << 1) | (1 << 2) | (1 << 4));
        GPIOA->ODR &= ~((1 << 1) | (1 << 5));
        GPIOB->ODR |= (1 << 3);
        led_red_enable = 0;
        TIM2->CR1 &= ~TIM_CR1_CEN; // Tắt counter
        TIM2->DIER &= ~TIM_DIER_UIE; // Tắt ngắt
        UART2_SendString("Safe: LED off, TIM2 disabled\n");
    }
}

SystemState getSystemState(float gas_ppm) {
    if (gas_ppm > THRESHOLD_DANGER) return STATE_EXTREME_DANGER;
    else if (gas_ppm > THRESHOLD_WARNING) return STATE_DANGER;
    else if (gas_ppm > THRESHOLD_SAFE) return STATE_WARNING;
    else return STATE_SAFE;
}
