/**
  ******************************************************************************
  * @file    usart_common.c
  * @author  Mikk Su
  * @version V1.0.0
  * @date    4-Nov-2015
  * @brief   Support linux-style debug information output.
	*					 The Usart DMA is used to transmit contents.
  *
  @verbatim
  ==============================================================================
                     ##### How to use this module #####
  ==============================================================================
    [..]
    No more info !

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 AMC</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of AMC nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "debug.h"
#include "usart.h"
#include "usart_common.h"
#include <stdarg.h>


/* Exported functions ---------------------------------------------------------*/

/**
  * @brief  Output the debug informations
  * @param  info: the content of the debug information
  * @retval	null
  */
void printk(const char* info, ...)
{
#ifdef __KERNEL_DEBUG_
	const char* ptrstr = info;

	if(*ptrstr++ != '<' || *ptrstr < '0' || *ptrstr > '7' || *++ptrstr != '>')
			return;

	if(*--ptrstr >= DEFAULT_MESSAGE_LOGLEVEL)
	{
		va_list ap;
		va_start(ap, info);
//		vsprintf(dbgbuf, info, ap);
		USART_COMM_TransmitDebugInfo(info + 3, ap);
		va_end(ap);

//		HAL_UART_Transmit_DMA(&huart1, (uint8_t*)dbgbuf, strlen(dbgbuf) + 1);
//		while(huart1.State == HAL_UART_STATE_BUSY_TX || huart1.State == HAL_UART_STATE_BUSY_TX_RX);
	}
#else
	/* Output debug information to the Terminal I/O */
	va_list ap;
	va_start(ap, info);
	vprintf(info, ap);
	va_end(ap);
#endif
}

