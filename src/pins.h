#ifndef PINS_H
#define PINS_H

#include <stdint.h>

// ILI9341 TFT on SPI0
#define TFT_SPI_INST        spi0
#define TFT_PIN_SCK         14
#define TFT_PIN_MOSI        15
#define TFT_PIN_CS          13
#define TFT_PIN_DC          12
#define TFT_PIN_RST         11

// I2C EEPROM (24AA32AF)
#define EEPROM_PIN_SDA      4
#define EEPROM_PIN_SCL      5
#define EEPROM_I2C_ADDR     0x50

// Joystick
#define JOY_PIN_X_ADC       45
#define JOY_PIN_Y_ADC       44
#define JOY_PIN_BTN         6

// Audio
#define AUDIO_PIN_PWM       30

// Debug LED
#define DEBUG_LED_PIN       17

// Free GPIO
#define FREE_GPIO_7         7
#define FREE_GPIO_8         8
#define FREE_GPIO_9         9
#define FREE_GPIO_10        10
#define FREE_GPIO_18        18
#define FREE_GPIO_19        19

// Display details
#define TFT_WIDTH           320
#define TFT_HEIGHT          240

// RGB565 color helpers
#define RGB565(r, g, b)     (uint16_t)((((r) & 0xF8u) << 8) | (((g) & 0xFCu) << 3) | ((b) >> 3))
#define COLOR_BLACK         ((uint16_t)0x0000)
#define COLOR_WHITE         ((uint16_t)0xFFFF)
#define COLOR_RED           ((uint16_t)0xF800)
#define COLOR_GREEN         ((uint16_t)0x07E0)
#define COLOR_BLUE          ((uint16_t)0x001F)
#define COLOR_YELLOW        ((uint16_t)0xFFE0)
#define COLOR_CYAN          ((uint16_t)0x07FF)
#define COLOR_MAGENTA       ((uint16_t)0xF81F)
#define COLOR_DARK_GRAY     ((uint16_t)0x4208)

#endif // PINS_H
