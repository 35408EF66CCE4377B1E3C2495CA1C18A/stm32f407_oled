#include "ethernet_common.h"
#include "socket.h"
#include "wizchip_conf.h"
#include <string.h>
#include "cmsis_os.h"


wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},
                            .ip = {192, 168, 2, 110},
                            .sn = {255,255,255,0},
                            .gw = {192, 168, 2, 1},
                            .dns = {0,0,0,0},
                            .dhcp = NETINFO_STATIC };

static void network_init(void)
{
	uint8_t tmpstr[6];

	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

	memset(&gWIZNETINFO, 0x0, sizeof(wiz_NetInfo));

	ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);

	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);
	printf("\r\n=== %s NET CONF ===\r\n",(char*)tmpstr);
	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
		gWIZNETINFO.mac[0],
		gWIZNETINFO.mac[1],
		gWIZNETINFO.mac[2],
		gWIZNETINFO.mac[3],
		gWIZNETINFO.mac[4],
		gWIZNETINFO.mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
	printf("======================\r\n");
}

/*
* Because all the reset pins of w5500 chips are connect to one pin of the MCU together,
* if the RESET pin is set, all the chips will be reseted
* so we need a special function to hardware reset chips
*/
void HardwareResetAllChips(void)
{
	wizchip_hw_reset();
}


void InitEternetInterface(uint8_t W5500_Index)
{
	uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
	intr_kind sirm = IK_SOCK_1;
	uint8_t tmp;
	int32_t ret = 0;

	gW5500_ch_sel = W5500_Index;

	printf("\r\n+++++++++++++++++++++++++\r\n");
	printf("Start W5500(%d) initialization!\r\n", gW5500_ch_sel);

	reg_wizchip_spi_cbfunc(0x0, 0x0);
	reg_wizchip_spiburst_cbfunc(0x0, 0x0);

	/* WIZCHIP SOCKET Buffer initialize */
	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
	{
		 printf("WIZCHIP Initialized fail.\r\n");
		 while(1);
	}

	/* WIZCHIP SOCKET interrupt mask initialize */
	if(ctlwizchip(CW_SET_INTRMASK, &sirm))
	{
		printf("WIZCHIP Socket Interrupt Mask Initialized fail.\r\n");
		 while(1);
	}
	else
	{
		sirm = (intr_kind)0x0;
		ctlwizchip(CW_GET_INTRMASK, &sirm);
		printf("IMR: %d\r\n", sirm);
	}

	/* Set SnIMR */
	setSn_IMR(SOCK_UDPS, SIK_RECEIVED);

	/* PHY link status check */
	tmp = 0;
	do
	{
		if(tmp == 0)
			printf("Wait for PHY Link...\r\n");
		tmp = 1;
		 if(ctlwizchip(CW_GET_PHYLINK, (void*)&ret) == -1)
				printf("Unknown PHY Link stauts.\r\n");

		 osDelay(5);
	}while(ret == PHY_LINK_OFF);

	/* Network initialization */
	network_init();

}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	extern osSemaphoreId semW5500RxCpltHandle;

	if(hspi->Instance == ETH_IF)
	{
		osSemaphoreRelease(semW5500RxCpltHandle);
	}

}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	extern osSemaphoreId semW5500TxCpltHandle;

	if(hspi->Instance == ETH_IF)
	{
		osSemaphoreRelease(semW5500TxCpltHandle);
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	extern osSemaphoreId semW5500Int0Handle;

	if(GPIO_Pin == GPIO_PIN_0)
	{
		// TODO Socket Interrupt
		osSemaphoreRelease(semW5500Int0Handle);
	}
}