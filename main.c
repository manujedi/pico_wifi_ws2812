/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "tcpserver.h"

#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "generated/ws2812.pio.h"

#include "pico/multicore.h"

#define LEDS 16
#define LINES 4

uint32_t leds[LINES][LEDS];


void setLed(uint16_t  line, uint16_t led, uint8_t red, uint8_t green, uint8_t blue){
    leds[line][led] = (red<<16) | (green << 24) | (blue << 8);

}

void writeLeds(){
    for (int j = 0; j < LEDS; j++) {
        pio_sm_put_blocking(pio0, 0, leds[0][j]);
        pio_sm_put_blocking(pio0, 1, leds[1][j]);
        pio_sm_put_blocking(pio0, 2, leds[2][j]);
        pio_sm_put_blocking(pio0, 3, leds[3][j]);
    }
}



void core1_entry() {
    while (1) {

        writeLeds();
        sleep_ms(10);
    }
}


int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    for (int i = 0; i < 4; ++i) {
        ws2812_program_init(pio, i, offset, i+2, 800000, 0);
    }


    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }

    uint8_t red=0;
    uint8_t green=0;
    uint8_t blue=0;
    uint16_t i=0;

    rgb = 0x000001;
    multicore_launch_core1(core1_entry);

    __attribute__((unused)) struct tcp_pcb *server_pcb = tcp_serveropen();
    while(1){
        printf("alive... %i connected\n", currently_connected);

        red = rgb >> 16;
        green = (rgb >> 8) & 0xFF;
        blue = rgb & 0xFF;

        printf("LED: R: %x, G: %x, B: %x\n",red,green,blue);

        setLed(0, 0,red,green,blue);

        for (int j = 1; j < LINES; ++j) {
            leds[j][0] = leds[j-1][LEDS-1];
        }
        for (int j = 0; j <LINES; ++j) {
            i=LEDS;
            while(i > 1){
                i--;
                leds[j][i] = leds[j][i-1];
            }
        }



        sleep_ms(1000);

    }
    tcp_server_close(server_pcb);


    cyw43_arch_deinit();
    return 0;
}