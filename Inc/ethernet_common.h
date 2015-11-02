#ifndef __ETHERNET_COMMON_H
#define __ETHERNET_COMMON_H

#include <stdint.h>

#define ETH_IF			SPI1

#define SOCK_TCPS        0
#define SOCK_UDPS        1

#define W5500_0			0
#define W5500_1			1
#define W5500_2			2
#define W5500_3			3
#define W5500_4			4



extern uint8_t gW5500_ch_sel;

void HardwareResetAllChips(void);
void InitEternetInterface(uint8_t);

#endif