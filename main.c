/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pico/printf.h>

#include "pico/stdlib.h"

#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "generated/ws2812.pio.h"

#include "pico/multicore.h"


#define LEDS 16
#define LINES 6

uint32_t leds[LINES][LEDS];

volatile uint32_t refreshrate = 0;


void setLed(uint16_t line, uint16_t led, uint8_t red, uint8_t green, uint8_t blue) {
    leds[line][led] = (red << 16) | (green << 24) | (blue << 8);

}


void writeLeds() {

    //write all
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio0, 0, leds[0][j]);
        pio_sm_put_blocking(pio0, 1, leds[1][j]);
        pio_sm_put_blocking(pio0, 2, leds[2][j]);
        pio_sm_put_blocking(pio0, 3, leds[3][j]);
        pio_sm_put_blocking(pio1, 0, leds[4][j]);
        pio_sm_put_blocking(pio1, 1, leds[5][j]);
    }

}

bool timer_callback(struct repeating_timer *t) {
    refreshrate++;
    writeLeds();
    return true;
}

void core1_entry() {

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    uint8_t state = 0;

    while (1) {

        switch (state) {
            //from red to green
            case 0:
                red--;
                green++;
                if (green == 255) {
                    red = 0;
                    state = 1;
                }
                break;
            case 1:
                //green to blue
                green--;
                blue++;
                if (blue == 255) {
                    green = 0;
                    state = 2;
                }
                break;
            case 2:
                //blue to red
                blue--;
                red++;
                if (red == 255) {
                    blue = 0;
                    state = 0;
                }
                break;
            default:
                break;
        }

        setLed(0, 0, red, green, blue);



        for (int line = LINES-1; line >= 0; line--) {
            for (int led = LEDS-1; led >= 0; led--) {
                if(led == 0 && line != 0) {
                    leds[line][0] = leds[line-1][LEDS-1];
                    continue;
                }

                if(led == 0)
                    continue;

                leds[line][led] = leds[line][led-1];
            }
        }
        sleep_ms(1000);
    }



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


    setLed(LINES - 1, LEDS - 1, 0xf, 0xf, 0xf);

    struct repeating_timer timer;
    add_repeating_timer_ms(-1000, timer_callback, NULL, &timer); //120HZ

    multicore_launch_core1(core1_entry);


    while (1) {
        printf("refreshrate: %iHz\n", refreshrate);
        refreshrate = 0;
        sleep_ms(1000);

    }




    return 0;
}