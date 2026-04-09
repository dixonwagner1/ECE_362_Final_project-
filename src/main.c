#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"

#include "display.h"
#include "pins.h"

static void blink_led(uint32_t on_ms, uint32_t off_ms, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        gpio_put(DEBUG_LED_PIN, 1);
        sleep_ms(on_ms);
        gpio_put(DEBUG_LED_PIN, 0);
        sleep_ms(off_ms);
    }
}

int main(void) {
    stdio_init_all();

    gpio_init(DEBUG_LED_PIN);
    gpio_set_dir(DEBUG_LED_PIN, GPIO_OUT);
    gpio_put(DEBUG_LED_PIN, 0);

    // Blink at startup first so there is immediate proof firmware is running,
    // even if display wiring/init is wrong.
    blink_led(120, 120, 4);

    display_init_spi();
    display_hard_reset();
    bool ok = display_init();

    if (ok) {
        // Simple one-time solid color test sequence.
        display_fill_screen(COLOR_RED);
        sleep_ms(500);
        display_fill_screen(COLOR_GREEN);
        sleep_ms(500);
        display_fill_screen(COLOR_BLUE);
        sleep_ms(500);
        display_fill_screen(COLOR_BLACK);
        sleep_ms(300);

        // Static menu screen.
        display_fill_screen(COLOR_BLACK);

        display_draw_text(86, 24, "GAME MENU", COLOR_YELLOW, COLOR_BLACK, 3);

        const uint16_t option_scale = 2;
        const uint16_t text_x = 92;
        const uint16_t opt1_y = 90;
        const uint16_t opt2_y = 126;
        const uint16_t opt3_y = 162;

        display_draw_text(text_x, opt1_y, "FLAPPY BIRD", COLOR_WHITE, COLOR_BLACK, option_scale);
        display_draw_text(text_x, opt2_y, "TETRIS", COLOR_WHITE, COLOR_BLACK, option_scale);
        display_draw_text(text_x, opt3_y, "SETTINGS", COLOR_WHITE, COLOR_BLACK, option_scale);

        // Highlight box around one option (TETRIS).
        display_fill_rect(74, (uint16_t)(opt2_y - 8), 2, 30, COLOR_CYAN); // left border
        display_fill_rect(74, (uint16_t)(opt2_y - 8), 176, 2, COLOR_CYAN); // top border
        display_fill_rect(248, (uint16_t)(opt2_y - 8), 2, 30, COLOR_CYAN); // right border
        display_fill_rect(74, (uint16_t)(opt2_y + 20), 176, 2, COLOR_CYAN); // bottom border
    }

    // Do not redraw in a loop. Leave the menu on screen and blink LED slowly.
    while (true) {
        gpio_put(DEBUG_LED_PIN, 1);
        sleep_ms(500);
        gpio_put(DEBUG_LED_PIN, 0);
        sleep_ms(500);
    }
}
