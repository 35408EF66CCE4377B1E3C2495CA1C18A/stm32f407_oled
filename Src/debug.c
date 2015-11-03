#include "debug.h"
#include "usart.h"
#include "usart_common.h"
#include <stdarg.h>


void printk(const char* info, ...)
{
#ifdef __KERNEL_DEBUG_
	const char* ptrstr = info;

	if(*ptrstr++ != '<' || *ptrstr < '0' || *ptrstr > '7' || *++ptrstr != '>')
			return;

	if(*--ptrstr >= DEFAULT_MESSAGE_LOGLEVEL)
	{
		va_list ptr_arg;
		va_start(ptr_arg, info);
//		vsprintf(dbgbuf, info, ptr_arg);
		USART_COMM_TransmitDebugInfo(info, ptr_arg);
		va_end(ptr_arg);

//		HAL_UART_Transmit_DMA(&huart1, (uint8_t*)dbgbuf, strlen(dbgbuf) + 1);
//		while(huart1.State == HAL_UART_STATE_BUSY_TX || huart1.State == HAL_UART_STATE_BUSY_TX_RX);
	}
#endif
}

