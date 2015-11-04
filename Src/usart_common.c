/**
  ******************************************************************************
  * @file    usart_common.c
  * @author  Mikk Su
  * @version V1.0.0
  * @date    4-Nov-2015
  * @brief   Usart通用模块。
  *          该模块属于中间层，负责将HAL层封装，提供给应用层简单的接口来调用USART硬件资源。
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
#include "usart_common.h"

/** @addtogroup COMMON MODULE
  * @{
  */

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables -----------------------------------------------------------*/
osMessageQId *qUart;													/*!< Point to the messageQId which is used to post Rx buffer pointer to the task awaiting*/

uint8_t **uart_buf_using;											/*!< Point to the position of the buffer where is currently used to save the incoming byte */
uint8_t *uart_buf_row;												/*!< point to the row of the buffer which is in use */
uint8_t (*uart_buf)[MAX_DEPTH_UART1_BUF];			/*!< Point to the head of each buffer row */


uint8_t uart1_buf[MAX_COUNT_UART1_BUF][MAX_DEPTH_UART1_BUF];
uint8_t uart1_buf_row;
uint8_t *uart1_buf_using;


uint8_t uart2_buf[MAX_COUNT_UART2_BUF][MAX_DEPTH_UART2_BUF];
uint8_t uart2_buf_row;
uint8_t *uart2_buf_using;

static char txbuf[512];

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup Private_Functions   Private Functions
  * @{
  */
static void UART_TransmitContent(UART_HandleTypeDef*);

/**
  *@}
  */

/* Exported functions ---------------------------------------------------------*/
/** @defgroup USART_COMMON_Exported_Functions USART_Common Exported Functions
  * @{
  */

/**
  * @brief  Initialize the usart port as the admin management interface
  * @param  huart: Pointer to the instance of the USART
  * @retval	null
  */
void USART_COMM_Init_AdminManagementPort(void)
{
  uart1_buf_row = 0;
  uart1_buf_using = &uart1_buf[uart1_buf_row][0];
  HAL_UART_Receive_IT(&huart1, uart1_buf_using, 1);

}

/**
  * @brief  Transmit the debug information through the specified uart port (in this case it's Uart1)
  * @param  content: debug information content
  * @param	ap: pointer of the arguments list
  * @retval	null
  */
void USART_COMM_TransmitDebugInfo(const char* content, va_list ap)
{
	vsprintf(txbuf, content, ap);
	UART_TransmitContent(&huart1);
}

/**
  * @brief  Transmit the control command echoes through the specified uart port (in this case it's Uart1)
  * @param  command: echoes of control command
  * @param	ap: pointer of the arguments list
  * @retval null
  */
void USART_COMM_TransmitCommand(const char* command, va_list ap)
{
//	va_list ap;
//	va_start(ap, command);
	vsprintf(txbuf, command, ap);
//	va_end(ap);
	UART_TransmitContent(&huart1);
}

/**
  *@}
  */

/* Private functions ---------------------------------------------------------*/

/** @defgroup USART_COMMON_Private_Functions USART_Common Private Functions
  * @{
  */

/**
  * @brief  It's LOW-LEVEL usart transmition function
  * 				it indicates where and how the data should be transmitted
  * @param  huart:Which usart port used to transmit the data
  * @retval null
  */
static void UART_TransmitContent(UART_HandleTypeDef *huart)
{
	HAL_UART_Transmit_DMA(&huart1, (uint8_t*)txbuf, strlen(txbuf) + 1);
	while(huart1.State == HAL_UART_STATE_BUSY_TX || huart1.State == HAL_UART_STATE_BUSY_TX_RX);
}

/**
  * @brief  The callback function of rx completion ISR
  * @param  huart: the owner of ISR
  * @retval	null
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /*
	NOTE: This function Should not be modified, when the callback is needed,
           the HAL_UART_TxCpltCallback could be implemented in the user file
   */
	uint8_t *pUartData;
	osStatus qretval;
	HAL_StatusTypeDef uartretval;

	if(huart->Instance == USART1)
	{
		uart_buf = uart1_buf;
		uart_buf_using = &uart1_buf_using;
		uart_buf_row = &uart1_buf_row;

		qUart = &qUart1Handle;
	}
	else if(huart->Instance == USART2)
	{
		uart_buf = uart1_buf;
		uart_buf_using = &uart2_buf_using;
		uart_buf_row = &uart2_buf_row;

		qUart = &qUart2Handle;
	}

	pUartData = *uart_buf_using;

	if(*pUartData == '\n')
	{
		if(*(--pUartData) == '\r')
		{
			// Replace the \r to 0x0, which indicates the terminal of the command string
			*pUartData = 0x0;

			if(xQueueIsQueueFullFromISR(*qUart) == pdTRUE)	// if the uart1 queue was full, drop this command
			{
				//*uart_buf_using = &uart1_buf[uart1_buf_row][0];
				*uart_buf_using = (uint8_t*)(uart_buf + *uart_buf_row);
			}
			else
			{
				/*
				* The current buffer has been used and post to the working thread
				* switch to the next uart1 queue buffer to recevie the furture data
				*/
				// Put the pointer of the data container to the queue
				qretval = osMessagePut(*qUart, (uint32_t)(uart_buf + *uart_buf_row), 100);

				//uart1_buf_row ++;
				(*uart_buf_row)++;

				if(*uart_buf_row >= MAX_COUNT_UART1_BUF)
					*uart_buf_row = 0;
				//uart1_buf_using = &uart1_buf[uart1_buf_row][0];
				*uart_buf_using = (uint8_t*)(uart_buf + *uart_buf_row);

			}

			//printk("  1, RXNEIE = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE));
			uartretval = HAL_UART_Receive_IT(huart, *uart_buf_using, 1);
			//printk("  2, RXNEIE = %d, retval = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE), uartretval);
		}
		else
		{
			// Running here means an error command was got OR a too long command was transmitted into the Buffer
			// Simply drop the received command and reuse the buffer
			*uart_buf_using = (uint8_t*)(uart_buf + *uart_buf_row);

			//printk("  8, RXNEIE = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE));
			HAL_UART_Receive_IT(huart, *uart_buf_using, 1);
			//printk("  9, RXNEIE = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE));
		}
	}
	else
	{
		// Move the pointer to the next byte of the uart1 recipient
		//uart1_buf_using++;
		(*uart_buf_using)++;

		if((*uart_buf_using) > (uint8_t*)((uint32_t)(uart_buf + *uart_buf_row) + MAX_DEPTH_UART1_BUF - 1))
		{
			*uart_buf_using = (uint8_t*)((uint32_t)(uart_buf + *uart_buf_row) + MAX_DEPTH_UART1_BUF - 1);
		}

		HAL_UART_Receive_IT(huart, *uart_buf_using, 1);
	}
}


/**
  * @brief  The callback function of communication error ISR
  * @param  huart:  the owner of ISR
  * @retval
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_UART_ErrorCallback could be implemented in the user file
   */
	if(huart->ErrorCode & HAL_UART_ERROR_ORE)
	{
		__HAL_UART_FLUSH_DRREGISTER(huart);
	}

	if(huart->ErrorCode & HAL_UART_ERROR_PE)
	{

	}

	if(huart->ErrorCode & HAL_UART_ERROR_NE)
	{

	}

	if(huart->ErrorCode & HAL_UART_ERROR_FE)
	{

	}

	if(huart->ErrorCode & HAL_UART_ERROR_DMA)
	{

	}

	uart1_buf_using = &uart1_buf[uart1_buf_row][0];
	HAL_UART_Receive_IT(huart, uart1_buf_using, 1);
}

/**
  *@}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT AMC *****END OF FILE****/
