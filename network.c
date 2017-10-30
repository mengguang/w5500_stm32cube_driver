/*

You need to label the w5500 spi cs pin as WCS in STM32CubeMX.
Or change the code below to proper value.

Then include network.h in main.c and call network_start();

*/

#include <gpio.h>
#include <spi.h>

#include <network.h>

#include "Ethernet/socket.h"
#include "Internet/DHCP/dhcp.h"
#include "network.h"

uint8_t gDATABUF[DATA_BUF_SIZE];

static void w5500_select() {
  HAL_GPIO_WritePin(WCS_GPIO_Port, WCS_Pin, GPIO_PIN_RESET);
}

static void w5500_deselect() {
  HAL_GPIO_WritePin(WCS_GPIO_Port, WCS_Pin, GPIO_PIN_SET);
}

static uint8_t w5500_spi_readbyte() {
  uint8_t data = 0;
  HAL_SPI_Receive(&hspi1, &data, 1, 1000);
  return data;
}

static void w5500_spi_writebyte(uint8_t data) {
  HAL_SPI_Transmit(&hspi1, &data, 1, 1000);
}

wiz_NetInfo gWIZNETINFO = {.mac = {0x00, 0x08, 0xdc, 0x00, 0xab, 0xcd},
    .ip = {192, 168, 1, 123},
    .sn = {255, 255, 255, 0},
    .gw = {192, 168, 1, 1},
    .dns = {0, 0, 0, 0},
    .dhcp = NETINFO_DHCP};

void network_init(void) {
  // Set Network information from netinfo structure
  ctlnetwork(CN_SET_NETINFO, (void *)&gWIZNETINFO);

#ifdef _MAIN_DEBUG_
  wiz_NetInfo netinfo;

  uint8_t tmpstr[6] = {
      0,
  };
  // Get Network information
  ctlnetwork(CN_GET_NETINFO, (void *)&netinfo);

  // Display Network Information
  ctlwizchip(CW_GET_ID, (void *)tmpstr);

  if (netinfo.dhcp == NETINFO_DHCP)
    printf("\n=== %s NET CONF : DHCP ===\n", (char *)tmpstr);
  else
    printf("\n=== %s NET CONF : Static ===\n", (char *)tmpstr);

  printf("MAC: %d:%d:%d:%d:%d:%d\n", netinfo.mac[0], netinfo.mac[1], netinfo.mac[2],
      netinfo.mac[3], netinfo.mac[4], netinfo.mac[5]);
  printf("SIP: %d.%d.%d.%d\n", netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);
  printf("GAR: %d.%d.%d.%d\n", netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]);
  printf("SUB: %d.%d.%d.%d\n", netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]);
  printf("DNS: %d.%d.%d.%d\n", netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]);
  printf("===========================\n");
#endif
}

static void my_ip_assign(void) {
  getIPfromDHCP(gWIZNETINFO.ip);
  getGWfromDHCP(gWIZNETINFO.gw);
  getSNfromDHCP(gWIZNETINFO.sn);
  getDNSfromDHCP(gWIZNETINFO.dns);
  gWIZNETINFO.dhcp = NETINFO_DHCP;
  /* Network initialization */
  network_init(); // apply from dhcp
#ifdef _MAIN_DEBUG_
  printf("DHCP LEASED TIME : %ld Sec.\n", getDHCPLeasetime());
#endif
}

static void my_ip_conflict(void) {
#ifdef _MAIN_DEBUG_
  printf("CONFLICT IP from DHCP\n");
#endif
  //halt or reset or any...
  while (1)
    ; // this example is halt.
}

uint8_t network_start() {
  uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
  uint8_t my_dhcp_retry = 0;
  uint8_t tmp;

  reg_wizchip_cs_cbfunc(w5500_select, w5500_deselect);
  reg_wizchip_spi_cbfunc(w5500_spi_readbyte, w5500_spi_writebyte);
  /* wizchip initialize*/
  if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1) {
    printf("WIZCHIP Initialized fail.\n");
    while (1)
      ;
  }

  /* PHY link status check */
  do {
    if (ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1)
      printf("Unknown PHY Link status.\n");
  } while (tmp == PHY_LINK_OFF);

  // must be set the default mac before DHCP started.
  setSHAR(gWIZNETINFO.mac);

  DHCP_init(SOCK_DHCP, gDATABUF);
  // if you want defiffent action instead defalut ip assign,update, conflict,
  // if cbfunc == 0, act as default.
  reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);

  while (1) {
    switch (DHCP_run()) {
    case DHCP_IP_ASSIGN:
    case DHCP_IP_CHANGED:
      /* If this block empty, act with default_ip_assign & default_ip_update */
      //
      // This example calls my_ip_assign in the two case.
      //
      // Add to ...
      //
      return 2;
      break;
    case DHCP_IP_LEASED:
      //
      // TO DO YOUR NETWORK APPs.
      //
      return 1;
      break;
    case DHCP_FAILED:
      /* ===== Example pseudo code =====  */
      // The below code can be replaced your code or omitted.
      // if omitted, retry to process DHCP
      my_dhcp_retry++;
      if (my_dhcp_retry > MY_MAX_DHCP_RETRY) {
#ifdef _MAIN_DEBUG_
        printf(">> DHCP %d Failed\n", my_dhcp_retry);
#endif
        my_dhcp_retry = 0;
        DHCP_stop();    // if restart, recall DHCP_init()
        network_init(); // apply the default static network and print out netinfo to serial
      }
      break;
    default:
      break;
    }
  }
}