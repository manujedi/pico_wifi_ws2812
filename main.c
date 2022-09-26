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

#define LEDS 77
uint8_t leds[LEDS*3];


void setLed(uint16_t led, uint8_t red, uint8_t green, uint8_t blue){
    leds[led*3+0]=red;
    leds[led*3+1]=green;
    leds[led*3+2]=blue;

}

void ledcopy(uint16_t src, uint16_t dest){
    leds[dest*3+0] = leds[src*3+0];
    leds[dest*3+1] = leds[src*3+1];
    leds[dest*3+2] = leds[src*3+2];
}

void writeLeds(){

    for (int j = 0; j < LEDS*3; j+=3) {
        pio_sm_put_blocking(pio0, 0, (leds[j]<<16) + (leds[j+2]<<8) + (leds[j+1]<<24));
    }
}

int main() {
    stdio_init_all();

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, 2, 800000, 0);


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

    __attribute__((unused)) struct tcp_pcb *server_pcb = tcp_serveropen();
    while(1){
        printf("alive... %i connected\n", currently_connected);

        red = rgb >> 16;
        green = (rgb >> 8) & 0xFF;
        blue = rgb & 0xFF;

        printf("LED: R: %x, G: %x, B: %x\n",red,green,blue);

        setLed(0,red,green,blue);


        i=LEDS;
        while(i > 1){
            i--;
            ledcopy(i-1,i);
        }

        writeLeds();

        sleep_ms(1000);

    }
    tcp_server_close(server_pcb);


    cyw43_arch_deinit();
    return 0;
}