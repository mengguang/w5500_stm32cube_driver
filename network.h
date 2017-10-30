#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "wizchip_conf.h"
#include <gpio.h>
#include <spi.h>

#define _MAIN_DEBUG_

#define SOCK_DHCP 0
#define MY_MAX_DHCP_RETRY 3

#define DATA_BUF_SIZE 2048

extern uint8_t gDATABUF[];

void network_init(void);
uint8_t network_start();

#endif