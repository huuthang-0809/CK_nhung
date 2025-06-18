#include "display.h"
#include "stm32f4xx.h"
#include "hardware.h"
#include "config.h"
#include "alarm.h"
#include <stdio.h>
#include <stdbool.h>

static void I2C1_WriteByte(uint8_t addr, uint8_t data) {
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
    (void)I2C1->SR1;
    I2C1->DR = addr;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1; (void)I2C1->SR2;
    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->CR1 |= I2C_CR1_STOP;
}

static void lcd_send_cmd(uint8_t cmd) {
    uint8_t high = cmd & 0xF0;
    uint8_t low = (cmd << 4) & 0xF0;
    uint8_t data_t[4] = {
        high | 0x0C, high | 0x08,
        low | 0x0C, low | 0x08
    };
    for (int i = 0; i < 4; i++) {
        I2C1_WriteByte(LCD_ADDR, data_t[i]);
        delay_ms(1);
    }
}

static void lcd_send_data(uint8_t data) {
    uint8_t high = data & 0xF0;
    uint8_t low = (data << 4) & 0xF0;
    uint8_t data_t[4] = {
        high | 0x0D, high | 0x09,
        low | 0x0D, low | 0x09
    };
    for (int i = 0; i < 4; i++) {
        I2C1_WriteByte(LCD_ADDR, data_t[i]);
        delay_ms(1);
    }
}

static void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_send_cmd(addr);
}

static void lcd_send_string(char *str) {
    while (*str) lcd_send_data(*str++);
}

void display_init(void) {
    delay_ms(50);
    lcd_send_cmd(0x33);
    lcd_send_cmd(0x32);
    lcd_send_cmd(0x28);
    lcd_send_cmd(0x0C);
    lcd_send_cmd(0x06);
    lcd_send_cmd(0x01);
    delay_ms(2);
}

void updateDisplay(float gas_ppm, bool is_reset) {
    char buffer[20];
    if (is_reset) {
        // Hiển thị trạng thái reset
        snprintf(buffer, sizeof(buffer), "%-16s", "Gas: -- ppm"); // Padding lên 16 ký tự
        lcd_set_cursor(0, 0);
        lcd_send_string(buffer);
        snprintf(buffer, sizeof(buffer), "%-16s", "Status: RESET");
        lcd_set_cursor(1, 0);
        lcd_send_string(buffer);
    } else {
        // Hiển thị bình thường
        snprintf(buffer, sizeof(buffer), "Gas: %5d ppm    ", (int)gas_ppm);
        lcd_set_cursor(0, 0);
        lcd_send_string(buffer);
        SystemState state = getSystemState(gas_ppm);
        const char* status = stateStrings[state];
        snprintf(buffer, sizeof(buffer), "Status: %-8s", status);
        lcd_set_cursor(1, 0);
        lcd_send_string(buffer);
    }
}

void updateDisplayStopped(void) {
    lcd_set_cursor(0, 0);
    lcd_send_string("                ");
    lcd_set_cursor(1, 0);
    lcd_send_string("Status: STOPPED ");
}
