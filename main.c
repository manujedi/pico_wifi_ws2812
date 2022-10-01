/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <pico/printf.h>
#include <pico/stdlib.h>

#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/regs/rosc.h>
#include <hardware/regs/addressmap.h>

#include "generated/ws2812.pio.h"


#define LEDS 16
#define ROWS 6

volatile uint32_t leds[ROWS][LEDS];

volatile uint32_t refreshrate = 0;


uint32_t rnd(void) {
    int k, random = 0;
    volatile uint32_t *rnd_reg = (uint32_t *) (ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

    for (k = 0; k < 32; k++) {
        random = random << 1;
        random = random + (0x00000001 & (*rnd_reg));
    }
    return random;
}

void setLed(uint16_t line, uint16_t led, uint8_t red, uint8_t green, uint8_t blue) {
    leds[line][led] = (red << 16) | (green << 24) | (blue << 8);

}


void writeLeds() {

    //write all
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio0, 0, leds[0][j]);
    }
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio0, 1, leds[1][j]);
    }
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio0, 2, leds[2][j]);
    }
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio0, 3, leds[3][j]);
    }
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio1, 0, leds[4][j]);
    }
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio1, 1, leds[5][j]);
    }


}

bool timer_callback(struct repeating_timer *t) {
    refreshrate++;
    writeLeds();
    return true;
}


int main() {
    stdio_init_all();;
    uint offset0 = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset0, 21, 800000, 0);
    ws2812_program_init(pio0, 1, offset0, 20, 800000, 0);
    ws2812_program_init(pio0, 2, offset0, 19, 800000, 0);
    ws2812_program_init(pio0, 3, offset0, 18, 800000, 0);

    uint offset1 = pio_add_program(pio1, &ws2812_program);
    ws2812_program_init(pio1, 0, offset1, 17, 800000, 0);
    ws2812_program_init(pio1, 1, offset1, 16, 800000, 0);


    //struct repeating_timer timer;
    //add_repeating_timer_us(-100000, timer_callback, NULL, &timer); //120HZ

    //multicore_launch_core1(core1_entry);

    srand(rnd());

    uint8_t w_r = 0xff;
    uint8_t w_g = 0xff;
    uint8_t w_b = 0xff;

    uint8_t red = 0xff;
    uint8_t green = 0xff;
    uint8_t blue = 0xff;


    while (1) {

        if(rand()%0xFF == 0) {
            w_r = rand() & 0xff;
            w_g = rand() & 0xff;
            w_b = rand() & 0xff;
        }

        while (red != w_r || green != w_g || blue != w_b){
            if(w_r > red)
                red++;
            if(w_r < red)
                red--;
            if (w_b > blue)
                blue++;
            if (w_b < blue)
                blue--;
            if (w_g > green)
                green++;
            if (w_g < green)
                green--;

            setLed(0,0,red,green,blue);

            for (int i = ROWS-1; i > 0; i--) {
                leds[i][0] = leds[i-1][0];
            }

            for (int i = LEDS-1; i > 0; i--) {
                for (int j = 0; j < ROWS; ++j) {
                    leds[j][i] = leds[j][i-1];
                }
            }
            writeLeds();
            sleep_ms(10);
        }
        sleep_ms(100);


    }


    return 0;
}