#include "sensor.h"
#include "stm32f4xx.h"
#include "config.h"

static uint16_t Read_ADC1(void) {
    ADC1->SR = 0;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    return ADC1->DR;
}

bool readGasSensor(float *gas_ppm) {
    uint16_t adc_val = Read_ADC1();
    int32_t adj = (int32_t)adc_val - ADC_BASELINE;
    if (adj < 0) adj = 0;
    *gas_ppm = ((float)adj / (4095.0f - ADC_BASELINE)) * PPM_MAX;
    return true;
}
