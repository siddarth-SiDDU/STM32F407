#include "stm32f4xx_hal.h"

// Define Ports & Pins
#define LCD_DATA_PORT GPIOD
#define LCD_CTRL_PORT GPIOE
#define LCD_RS_PIN    GPIO_PIN_0
#define LCD_E_PIN     GPIO_PIN_1

// Microsecond Delay helper using SysTick or a simple loop
void LCD_Delay_us(uint32_t us) {
    uint32_t count = us * (SystemCoreClock / 1000000U) / 5U;
    while(count--) { __NOP(); }
}

void LCD_EnablePulse(void) {
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_E_PIN, GPIO_PIN_SET);
    LCD_Delay_us(10);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_E_PIN, GPIO_PIN_RESET);
    LCD_Delay_us(50);
}

void LCD_SendCommand(uint8_t cmd) {
    // RS = 0 for Command
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS_PIN, GPIO_PIN_RESET);

    // Write byte to PD0-PD7
    LCD_DATA_PORT->ODR = (LCD_DATA_PORT->ODR & 0xFF00) | cmd;

    LCD_EnablePulse();

    // Commands like Clear Display require longer execution time (~2ms)
    if (cmd == 0x01 || cmd == 0x02) {
        HAL_Delay(2);
    }
}

void LCD_SendData(uint8_t data) {
    // RS = 1 for Data
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS_PIN, GPIO_PIN_SET);

    // Write byte to PD0-PD7
    LCD_DATA_PORT->ODR = (LCD_DATA_PORT->ODR & 0xFF00) | data;

    LCD_EnablePulse();
}

void LCD_Init(void) {
    // Wait >15ms after power-on
    HAL_Delay(50);

    // Function set: 8-bit mode, 2 lines, 5x8 font
    LCD_SendCommand(0x38);
    HAL_Delay(5);

    LCD_SendCommand(0x38);
    LCD_Delay_us(150);

    LCD_SendCommand(0x38);

    // Display ON, Cursor OFF, Blink OFF
    LCD_SendCommand(0x0C);

    // Clear Display
    LCD_SendCommand(0x01);

    // Entry Mode Set: Increment cursor, no shift
    LCD_SendCommand(0x06);
}

void LCD_SendString(const char *str) {
    while (*str) {
        LCD_SendData((uint8_t)(*str++));
    }
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_SendCommand(address);
}
