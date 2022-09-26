//
// Created by manu on 9/26/22.
//

#ifndef MAIN_TCPSERVER_H
#define MAIN_TCPSERVER_H

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "websites/websites.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"


#define TCP_PORT 4242
#define BUF_SIZE 2048
#define POLL_TIME_S 5

extern uint32_t connection;
extern uint16_t currently_connected;
extern uint32_t rgb;
extern char httpheader[];

typedef struct TCP_CLIENT_T {
    struct tcp_pcb *client_pcb;
    uint8_t buffer_recv[BUF_SIZE];
    int recv_len;
    int con_num;
} TCP_CLIENT_T;


err_t tcp_server_close(struct tcp_pcb* server_pcb);

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb, char *data, u16_t len);

err_t tcp_server_client_close(struct TCP_CLIENT_T* state);

err_t http_serve_response(struct TCP_CLIENT_T* client);

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);

static void tcp_server_err(void *arg, err_t err);

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);

struct tcp_pcb* tcp_serveropen();


#endif //MAIN_TCPSERVER_H
