#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

void display_init_spi(void);
void display_hard_reset(void);
bool display_init(void);

void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void display_fill_screen(uint16_t color);
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

void display_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
void display_draw_text(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg, uint8_t scale);

void display_draw_test_rect_center(uint16_t color);

#endif // DISPLAY_H
