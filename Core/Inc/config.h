#ifndef CONFIG_H
#define CONFIG_H

#define LCD_ADDR (0x27 << 1)
#define ADC_BASELINE 1300
#define PPM_MAX 10000
#define THRESHOLD_SAFE 500
#define THRESHOLD_WARNING 1000
#define THRESHOLD_DANGER 5000
#define BLINK_MIN_PERIOD 100
#define BLINK_MAX_PERIOD 500

#define VOLTAGE_REF    3.3f    // Điện áp tham chiếu của ADC (3.3V)
#define RL             10000.0f // Điện trở tải (10kΩ)
#define VCC            5.0f     // Điện áp nguồn cung cấp cho MQ5
#define R0             39505.0f // Giá trị R0 đã hiệu chuẩn (ADC 1255)

#endif
