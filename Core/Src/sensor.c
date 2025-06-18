#include "sensor.h"
#include "stm32f4xx.h"
#include "config.h"
#include <math.h>

static uint16_t Read_ADC1(void) {
    ADC1->SR = 0;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    return ADC1->DR;
}

bool readGasSensor(float *gas_ppm) {
    uint16_t adc_val = Read_ADC1();
    float adc_voltage = (adc_val / 4095.0f) * VOLTAGE_REF; // Tính điện áp đầu ra
    float Rs;
    if (adc_voltage > 0.0f) {
        Rs = (VCC * RL / adc_voltage) - RL;
    } else {
        Rs = 0.0f;
    }
    float ratio = (R0 > 0.0f) ? (Rs / R0) : 0.0f;
    *gas_ppm = (ratio > 0.0f) ? (100.0f * pow(ratio, -2.5f)) : 0.0f;
}
