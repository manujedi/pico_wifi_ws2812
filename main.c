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
#define DROPS 10

volatile uint32_t leds[ROWS][LEDS];

typedef struct DROP {
    uint8_t col;
    uint8_t row;
    uint32_t color;
} DROP;


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


    volatile DROP drop[DROPS];

    for (int i = 0; i < DROPS; ++i) {
        drop[i].row = 0x0;
        drop[i].col = 0x0;
        drop[i].color = 0xFFFFFF00;
    }

    srand(rnd());

    while (1) {

        for (int i = 0; i < DROPS; ++i) {
            printf("drop[%i].row%i\n", i,drop[i].row);
            if (drop[i].row >= ROWS) {
                if((((uint32_t) rand()) & 0x3) == 0) {            //async
                    printf("resetting drop %i\n",i);
                    drop[i].col = ((uint32_t) rand()) & 0xF;
                    drop[i].row = 0;
                    drop[i].color = ((uint32_t) rand()) & 0xFFFFFF00;
                }
            }else{
                leds[drop[i].row][drop[i].col] = drop[i].color;
                drop[i].row++;
            }
        }

        writeLeds();

        sleep_ms(100);

    }


    return 0;
}