#include "stm32f4xx.h"
#include "hardware.h"
#include "config.h"

static volatile uint8_t prev_btn1 = 1;
static volatile uint8_t prev_btn2 = 1;
extern volatile uint8_t system_running;
extern volatile uint32_t safe_timer;

// Hàm delay bằng SysTick
void delay_ms(uint32_t ms) {
    SysTick->LOAD = 16000 - 1;  // 1ms với HCLK 16MHz
    SysTick->VAL = 0;
    SysTick->CTRL = 5;
    for(uint32_t i = 0; i < ms; i++) {
        while(!(SysTick->CTRL & (1 << 16)));
    }
    SysTick->CTRL = 0;
}

// Cấu hình clock (HSI 16MHz mặc định)
static void SystemClock_Config(void) {

}

// Cấu hình GPIO cho LED, relay, còi và nút bấm
static void GPIO_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // PC13, PC12: input pull-up cho SW1, SW2
    GPIOC->MODER &= ~((3 << (13 * 2)) | (3 << (12 * 2)));
    GPIOC->PUPDR &= ~((3 << (13 * 2)) | (3 << (12 * 2)));
    GPIOC->PUPDR |= (1 << (13 * 2)) | (1 << (12 * 2));

    // PA1 (relay), PA5 (còi), PA6 (ESP): output
    GPIOA->MODER &= ~((3 << (1 * 2)) | (3 << (5 * 2)) | (3 << (6 * 2)));
    GPIOA->MODER |= (1 << (1 * 2)) | (1 << (5 * 2)) | (1 << (6 * 2));
    GPIOA->OTYPER &= ~((1 << 1) | (1 << 5) | (1 << 6));
    GPIOA->OSPEEDR |= (3 << (1 * 2)) | (3 << (5 * 2)) | (2 << (6 * 2));
    GPIOA->PUPDR &= ~((3 << (1 * 2)) | (3 << (5 * 2)) | (3 << (6 * 2)));

    // PB1 (LED đỏ), PB2 (LED vàng), PB3 (LED xanh), PB4 (LED xanh lá): output
    GPIOB->MODER &= ~((3 << (1 * 2)) | (3 << (2 * 2)) | (3 << (3 * 2)) | (3 << (4 * 2)));
    GPIOB->MODER |= (1 << (1 * 2)) | (1 << (2 * 2)) | (1 << (3 * 2)) | (1 << (4 * 2));
    GPIOB->OTYPER &= ~((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
    GPIOB->OSPEEDR |= (3 << (1 * 2)) | (3 << (2 * 2)) | (3 << (3 * 2)) | (3 << (4 * 2));
    GPIOB->PUPDR &= ~((3 << (1 * 2)) | (3 << (2 * 2)) | (3 << (3 * 2)) | (3 << (4 * 2)));
}

// Cấu hình ADC1 tại PA0
static void ADC1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER |= (3 << (0 * 2));
    GPIOA->PUPDR &= ~(3 << (0 * 2));

    ADC1->CR2 = 0;
    ADC1->SQR3 = 0;
    ADC1->SMPR2 |= (7 << 0);
    ADC1->CR2 |= ADC_CR2_ADON;
}

// Cấu hình I2C1 cho LCD
static void I2C1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~((3 << (8 * 2)) | (3 << (9 * 2)));
    GPIOB->MODER |= (2 << (8 * 2)) | (2 << (9 * 2));
    GPIOB->AFR[1] &= ~((0xF << 0) | (0xF << 4));
    GPIOB->AFR[1] |= (4 << 0) | (4 << 4);
    GPIOB->OTYPER |= (1 << 8) | (1 << 9);
    GPIOB->PUPDR &= ~((3 << (8 * 2)) | (3 << (9 * 2)));

    I2C1->CR2 = 16;
    I2C1->CCR = 80;
    I2C1->TRISE = 17;
    I2C1->CR1 |= I2C_CR1_PE;
}

// Cấu hình Timer2 cho nhấp nháy LED
static void Timer2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 16000 - 1;
    TIM2->ARR = 1000;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 0);
}

// Khởi tạo tất cả phần cứng
void initHardware(void) {
    SystemClock_Config();
    GPIO_Init();
    ADC1_Init();
    I2C1_Init();
    USART2_Init();
    Timer2_Init();
}

// Xử lý trạng thái nút bấm
void handleSystemState(void) {
    uint8_t btn1 = (GPIOC->IDR & (1 << 13)) ? 1 : 0;  // SW1 - PC13
    uint8_t btn2 = (GPIOC->IDR & (1 << 12)) ? 1 : 0;  // SW2 - PC12

    // Phát hiện cạnh xuống SW1
    if (prev_btn1 == 1 && btn1 == 0) {
        system_running = !system_running;
        GPIOB->ODR &= ~((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));
        GPIOA->ODR &= ~((1 << 1) | (1 << 5) | (1 << 6));
        delay_ms(200);
    }
    prev_btn1 = btn1;

    // Phát hiện cạnh xuống SW2
    if (prev_btn2 == 1 && btn2 == 0) {
        system_running = 1; // Đảm bảo hệ thống hoạt động
        safe_timer = 5; // Đặt thời gian reset là 5 giây
        TIM2->CR1 &= ~TIM_CR1_CEN;
        NVIC_DisableIRQ(TIM2_IRQn);
        GPIOB->ODR &= ~((1 << 1) | (1 << 2) | (1 << 4));
        GPIOB->ODR |= (1 << 3);
        GPIOA->ODR &= ~((1 << 1) | (1 << 5) | (1 << 6));
        delay_ms(200);
    }
    prev_btn2 = btn2;
}

// Khởi tạo USART2 @115200
void USART2_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOA->MODER &= ~((3 << (2 * 2)) | (3 << (3 * 2)));
    GPIOA->MODER |= (2 << (2 * 2)) | (2 << (3 * 2));
    GPIOA->AFR[0] &= ~((0xF << (2 * 4)) | (0xF << (3 * 4)));
    GPIOA->AFR[0] |= (7 << (2 * 4)) | (7 << (3 * 4));

    uint32_t div = (16 * 1000000 + 115200 / 2) / 115200;
    USART2->BRR = div;
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
    for (volatile int i = 0; i < 1000; i++);
}

// Gửi 1 ký tự qua UART
void UART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (c & 0xFF);
}

// Gửi chuỗi qua UART
void UART2_SendString(const char *s) {
    while (*s) UART2_SendChar(*s++);
}

// Gửi số float qua UART
void UART2_SendFloat(float f, uint8_t dec) {
    if (f < 0) { UART2_SendChar('-'); f = -f; }
    uint32_t ip = (uint32_t)f;
    float frac = f - (float)ip;
    char buf[12];
    int i = 0;
    if (ip == 0) buf[i++] = '0';
    while (ip) {
        buf[i++] = '0' + (ip % 10);
        ip /= 10;
    }
    while (i--) UART2_SendChar(buf[i]);
    UART2_SendChar('.');
    for (uint8_t d = 0; d < dec; d++) {
        frac *= 10;
        uint8_t digit = (uint8_t)frac;
        UART2_SendChar('0' + digit);
        frac -= digit;
    }
}
