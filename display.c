#include "display.h"

#include <stddef.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/time.h"

#include "pins.h"

#define ILI9341_CMD_SWRESET   0x01
#define ILI9341_CMD_SLPOUT    0x11
#define ILI9341_CMD_DISPOFF   0x28
#define ILI9341_CMD_DISPON    0x29
#define ILI9341_CMD_CASET     0x2A
#define ILI9341_CMD_PASET     0x2B
#define ILI9341_CMD_RAMWR     0x2C
#define ILI9341_CMD_MADCTL    0x36
#define ILI9341_CMD_PIXFMT    0x3A
#define ILI9341_CMD_FRMCTR1   0xB1
#define ILI9341_CMD_DFUNCTR   0xB6
#define ILI9341_CMD_PWCTR1    0xC0
#define ILI9341_CMD_PWCTR2    0xC1
#define ILI9341_CMD_VMCTR1    0xC5
#define ILI9341_CMD_VMCTR2    0xC7
#define ILI9341_CMD_GAMMASET  0x26
#define ILI9341_CMD_GMCTRP1   0xE0
#define ILI9341_CMD_GMCTRN1   0xE1

// SPI mode 0, conservative frequency for reliable bring-up.
// Start around 10-20 MHz. Here we use 12 MHz.
#define TFT_SPI_BAUD_HZ       12000000u

#define FONT_W                5u
#define FONT_H                7u

static void tft_select(void) {
    gpio_put(TFT_PIN_CS, 0);
}

static void tft_deselect(void) {
    gpio_put(TFT_PIN_CS, 1);
}

static void tft_write_command(uint8_t cmd) {
    tft_select();
    gpio_put(TFT_PIN_DC, 0);
    spi_write_blocking(TFT_SPI_INST, &cmd, 1);
    tft_deselect();
}

static void tft_write_data(const uint8_t *data, size_t len) {
    if (len == 0) {
        return;
    }
    tft_select();
    gpio_put(TFT_PIN_DC, 1);
    spi_write_blocking(TFT_SPI_INST, data, (size_t)len);
    tft_deselect();
}

static void tft_write_u16(uint16_t value) {
    uint8_t buf[2] = {(uint8_t)(value >> 8), (uint8_t)(value & 0xFFu)};
    tft_write_data(buf, sizeof(buf));
}

static void tft_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    tft_write_command(ILI9341_CMD_CASET);
    tft_write_u16(x0);
    tft_write_u16(x1);

    tft_write_command(ILI9341_CMD_PASET);
    tft_write_u16(y0);
    tft_write_u16(y1);

    tft_write_command(ILI9341_CMD_RAMWR);
}

static const uint8_t *font_pattern(char c) {
    // 5x7 glyphs, each byte is one column (bit0 = top pixel).
    static const uint8_t GLYPH_SPACE[FONT_W] = {0x00, 0x00, 0x00, 0x00, 0x00};
    static const uint8_t GLYPH_A[FONT_W] = {0x7E, 0x09, 0x09, 0x09, 0x7E};
    static const uint8_t GLYPH_B[FONT_W] = {0x7F, 0x49, 0x49, 0x49, 0x36};
    static const uint8_t GLYPH_D[FONT_W] = {0x7F, 0x41, 0x41, 0x22, 0x1C};
    static const uint8_t GLYPH_E[FONT_W] = {0x7F, 0x49, 0x49, 0x49, 0x41};
    static const uint8_t GLYPH_F[FONT_W] = {0x7F, 0x09, 0x09, 0x09, 0x01};
    static const uint8_t GLYPH_G[FONT_W] = {0x3E, 0x41, 0x49, 0x49, 0x7A};
    static const uint8_t GLYPH_I[FONT_W] = {0x00, 0x41, 0x7F, 0x41, 0x00};
    static const uint8_t GLYPH_L[FONT_W] = {0x7F, 0x40, 0x40, 0x40, 0x40};
    static const uint8_t GLYPH_M[FONT_W] = {0x7F, 0x02, 0x0C, 0x02, 0x7F};
    static const uint8_t GLYPH_N[FONT_W] = {0x7F, 0x04, 0x08, 0x10, 0x7F};
    static const uint8_t GLYPH_O[FONT_W] = {0x3E, 0x41, 0x41, 0x41, 0x3E};
    static const uint8_t GLYPH_P[FONT_W] = {0x7F, 0x09, 0x09, 0x09, 0x06};
    static const uint8_t GLYPH_R[FONT_W] = {0x7F, 0x09, 0x19, 0x29, 0x46};
    static const uint8_t GLYPH_S[FONT_W] = {0x46, 0x49, 0x49, 0x49, 0x31};
    static const uint8_t GLYPH_T[FONT_W] = {0x01, 0x01, 0x7F, 0x01, 0x01};
    static const uint8_t GLYPH_Y[FONT_W] = {0x03, 0x04, 0x78, 0x04, 0x03};

    switch (c) {
        case 'A': return GLYPH_A;
        case 'B': return GLYPH_B;
        case 'D': return GLYPH_D;
        case 'E': return GLYPH_E;
        case 'F': return GLYPH_F;
        case 'G': return GLYPH_G;
        case 'I': return GLYPH_I;
        case 'L': return GLYPH_L;
        case 'M': return GLYPH_M;
        case 'N': return GLYPH_N;
        case 'O': return GLYPH_O;
        case 'P': return GLYPH_P;
        case 'R': return GLYPH_R;
        case 'S': return GLYPH_S;
        case 'T': return GLYPH_T;
        case 'Y': return GLYPH_Y;
        case ' ': return GLYPH_SPACE;
        default:  return GLYPH_SPACE;
    }
}

void display_init_spi(void) {
    spi_init(TFT_SPI_INST, TFT_SPI_BAUD_HZ);

    gpio_set_function(TFT_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(TFT_PIN_MOSI, GPIO_FUNC_SPI);

    spi_set_format(TFT_SPI_INST, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_init(TFT_PIN_CS);
    gpio_set_dir(TFT_PIN_CS, GPIO_OUT);
    gpio_put(TFT_PIN_CS, 1);

    gpio_init(TFT_PIN_DC);
    gpio_set_dir(TFT_PIN_DC, GPIO_OUT);
    gpio_put(TFT_PIN_DC, 1);

    gpio_init(TFT_PIN_RST);
    gpio_set_dir(TFT_PIN_RST, GPIO_OUT);
    gpio_put(TFT_PIN_RST, 1);
}

void display_hard_reset(void) {
    // Required reset timing (safe values):
    // RST high >= 10 ms, low >= 20 ms, then high >= 120 ms before init commands.
    gpio_put(TFT_PIN_RST, 1);
    sleep_ms(10);
    gpio_put(TFT_PIN_RST, 0);
    sleep_ms(20);
    gpio_put(TFT_PIN_RST, 1);
    sleep_ms(120);
}

bool display_init(void) {
    // White screen usually means command/data sequencing or init order is wrong,
    // so keep this sequence simple and explicit.
    tft_write_command(ILI9341_CMD_DISPOFF);

    tft_write_command(ILI9341_CMD_SWRESET);
    sleep_ms(150);

    tft_write_command(ILI9341_CMD_SLPOUT);
    sleep_ms(120);

    {
        uint8_t d = 0x55; // 16-bit/pixel (RGB565)
        tft_write_command(ILI9341_CMD_PIXFMT);
        tft_write_data(&d, 1);
    }

    {
        uint8_t d = 0x01;
        tft_write_command(ILI9341_CMD_GAMMASET);
        tft_write_data(&d, 1);
    }

    {
        uint8_t d[2] = {0x23, 0x10};
        tft_write_command(ILI9341_CMD_PWCTR1);
        tft_write_data(d, 2);
    }

    {
        uint8_t d = 0x10;
        tft_write_command(ILI9341_CMD_PWCTR2);
        tft_write_data(&d, 1);
    }

    {
        uint8_t d[2] = {0x3E, 0x28};
        tft_write_command(ILI9341_CMD_VMCTR1);
        tft_write_data(d, 2);
    }

    {
        uint8_t d = 0x86;
        tft_write_command(ILI9341_CMD_VMCTR2);
        tft_write_data(&d, 1);
    }

    {
        uint8_t d[2] = {0x00, 0x18};
        tft_write_command(ILI9341_CMD_FRMCTR1);
        tft_write_data(d, 2);
    }

    {
        uint8_t d[3] = {0x08, 0x82, 0x27};
        tft_write_command(ILI9341_CMD_DFUNCTR);
        tft_write_data(d, 3);
    }

    {
        // Landscape orientation with BGR color order (common on ILI9341 modules):
        // MY=0, MX=0, MV=1, ML=0, BGR=1, MH=0 -> 0x28
        uint8_t d = 0x28;
        tft_write_command(ILI9341_CMD_MADCTL);
        tft_write_data(&d, 1);
    }

    {
        const uint8_t pos_gamma[15] = {
            0x0F, 0x31, 0x2B, 0x0C, 0x0E,
            0x08, 0x4E, 0xF1, 0x37, 0x07,
            0x10, 0x03, 0x0E, 0x09, 0x00
        };
        tft_write_command(ILI9341_CMD_GMCTRP1);
        tft_write_data(pos_gamma, sizeof(pos_gamma));
    }

    {
        const uint8_t neg_gamma[15] = {
            0x00, 0x0E, 0x14, 0x03, 0x11,
            0x07, 0x31, 0xC1, 0x48, 0x08,
            0x0F, 0x0C, 0x31, 0x36, 0x0F
        };
        tft_write_command(ILI9341_CMD_GMCTRN1);
        tft_write_data(neg_gamma, sizeof(neg_gamma));
    }

    tft_set_addr_window(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

    tft_write_command(ILI9341_CMD_DISPON);
    sleep_ms(20);

    return true;
}

void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) {
        return;
    }

    tft_set_addr_window(x, y, x, y);
    tft_write_u16(color);
}

void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT || w == 0 || h == 0) {
        return;
    }

    uint16_t x1 = (uint16_t)(x + w - 1);
    uint16_t y1 = (uint16_t)(y + h - 1);

    if (x1 >= TFT_WIDTH) {
        x1 = TFT_WIDTH - 1;
    }
    if (y1 >= TFT_HEIGHT) {
        y1 = TFT_HEIGHT - 1;
    }

    uint32_t pixels = (uint32_t)(x1 - x + 1) * (uint32_t)(y1 - y + 1);

    tft_set_addr_window(x, y, x1, y1);

    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFFu);
    uint8_t chunk[128];

    for (size_t i = 0; i < sizeof(chunk); i += 2) {
        chunk[i] = hi;
        chunk[i + 1] = lo;
    }

    tft_select();
    gpio_put(TFT_PIN_DC, 1);
    while (pixels > 0) {
        uint32_t px_in_chunk = pixels;
        if (px_in_chunk > (sizeof(chunk) / 2u)) {
            px_in_chunk = sizeof(chunk) / 2u;
        }
        spi_write_blocking(TFT_SPI_INST, chunk, px_in_chunk * 2u);
        pixels -= px_in_chunk;
    }
    tft_deselect();
}

void display_fill_screen(uint16_t color) {
    display_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

void display_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
    if (scale == 0) {
        scale = 1;
    }

    const uint8_t *glyph = font_pattern(c);
    for (uint8_t col = 0; col < FONT_W; col++) {
        uint8_t bits = glyph[col];
        for (uint8_t row = 0; row < FONT_H; row++) {
            uint16_t color = (bits & (1u << row)) ? fg : bg;
            display_fill_rect((uint16_t)(x + col * scale), (uint16_t)(y + row * scale), scale, scale, color);
        }
    }

    // 1-column spacing
    display_fill_rect((uint16_t)(x + FONT_W * scale), y, scale, (uint16_t)(FONT_H * scale), bg);
}

void display_draw_text(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg, uint8_t scale) {
    if (text == NULL) {
        return;
    }

    uint16_t cursor_x = x;
    while (*text != '\0') {
        display_draw_char(cursor_x, y, *text, fg, bg, scale);
        cursor_x = (uint16_t)(cursor_x + (FONT_W + 1u) * scale);
        text++;
    }
}

void display_draw_test_rect_center(uint16_t color) {
    const uint16_t w = 120;
    const uint16_t h = 80;
    const uint16_t x = (TFT_WIDTH - w) / 2u;
    const uint16_t y = (TFT_HEIGHT - h) / 2u;
    display_fill_rect(x, y, w, h, color);
}

/*
DISPLAY DEBUG CHECKLIST

If nothing appears on screen, check these first:
- wrong SPI instance or wrong pin function mapping
- wrong SPI mode
- SPI clock too fast
- missing or incorrect reset timing
- incorrect CS/DC handling
- wrong ILI9341 init sequence
- wrong MADCTL/rotation
- address window bug
- backlight not enabled if this module requires it
- wiring or power issue
*/
