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
#include "debug.h"
#include "stm32f4xx_hal.h"

/** @addtogroup COMMON MODULE
  * @{
  */

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables -----------------------------------------------------------*/
osMessageQId *qUart;													/*!< Point to the messageQId which is used to post Rx buffer pointer to the task awaiting*/

//uint8_t **uart_buf_using;											/*!< Point to the position of the buffer where is currently used to save the incoming byte */
//uint8_t *uart_buf_row;												/*!< point to the row of the buffer which is in use */
uint8_t (*uart_buf)[MAX_DEPTH_UART1_BUF];			/*!< Point to the head of each buffer row */


uint8_t uart1_buf[MAX_COUNT_UART1_BUF][MAX_DEPTH_UART1_BUF];
//uint8_t uart1_buf_row;
//uint8_t *uart1_buf_using;


uint8_t uart2_buf[MAX_COUNT_UART2_BUF][MAX_DEPTH_UART2_BUF];
//uint8_t uart2_buf_row;
//uint8_t *uart2_buf_using;

static char txbuf[512];

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup Private_Functions   Private Functions
  * @{
  */
static void UART_TransmitContent(UART_HandleTypeDef*);
static void USART_Start_Receive_DMA(UART_HandleTypeDef*);
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
	//uart1_buf_row = 0;
	//uart1_buf_using = &uart1_buf[uart1_buf_row][0];
	uart_buf = uart1_buf;

  //HAL_UART_Receive_IT(&huart1, uart1_buf_using, 1);
	USART_Start_Receive_DMA(&huart1);
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
  * @brief  The callback function of communication bus idle ISR.
  * @param  huart: the owner of ISR.
  * @retval null
  */
void My_HAL_UART_IdleCallback(UART_HandleTypeDef *huart)
{
	osStatus qretval;								/*!< The return value which indicates the osMessagePut() implementation result */
	uint8_t* ptrdata;								/*!< Pointer to any byte in the uart buffer */
	uint8_t (*ptr_bufhead)[1],			/*!< Pointer to the head of the uart buffer */
					(*ptr_buftail)[1];			/*!< Pointer to the tail of the uart buffer */

	if(huart->Instance == USART1)
	{
		uint32_t reclen = MAX_DEPTH_UART1_BUF - huart->hdmarx->Instance->NDTR;
		ptrdata = *uart_buf + reclen - 1;		// point to the last char received
		ptr_bufhead = (uint8_t(*)[1])uart1_buf[0];
		ptr_buftail = (uint8_t(*)[1])uart1_buf[MAX_COUNT_UART1_BUF - 2];
		qUart = &qUart1Handle;
	}
	else if(huart->Instance == USART2)
	{
		uint32_t reclen = MAX_DEPTH_UART2_BUF - huart->hdmarx->Instance->NDTR;
		ptrdata = *uart_buf + reclen - 1;		// point to the last char received
		ptr_bufhead = (uint8_t(*)[1])uart2_buf[0];
		ptr_buftail = (uint8_t(*)[1])uart2_buf[MAX_COUNT_UART2_BUF - 2];
		qUart = &qUart2Handle;
	}


	if(*ptrdata == '\n')
	{
		if(*(--ptrdata) == '\r')	// A command has been received
		{
			if(xQueueIsQueueFullFromISR(*qUart) == pdTRUE)	// if the uart1 queue was full, drop this command
			{
				//*uart_buf_using = &uart1_buf[uart1_buf_row][0];

			}
			else
			{
				/*
				* The current buffer has been used and post to the working thread
				* switch to the next uart1 queue buffer to recevie the furture data
				*/
				qretval = osMessagePut(*qUart, (uint32_t)(uart_buf), 100);	// Put the pointer of the data container to the queue
				if(qretval != osOK)
				{
						printk(KERN_ERR "It's failed to put the command into the message queue!\r\n");
				}

				/* Move to the next row of the buffer */
				uart_buf++;
				if(uart_buf > (uint8_t(*)[50])ptr_buftail)
				{
						uart_buf = (uint8_t(*)[50])ptr_bufhead;
				}
			}
		}

		/*!< Reset DMA_EN bit can result in the TCIF interrupt.
		The interrupt raises the HAL_UART_RxCpltCallback() event, the DMA_Rx will be restarted in it */
		HAL_DMA_Abort(huart->hdmarx);
	}
	else
	{
			/* Enable IDLE interrupt again */
			__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	}
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
  * @brief  It's LOW-LEVEL usart receive initialization function.
  * 				The DMA will be used to receive data.
  * @param  huart:Which usart port used to receive data
  * @retval null
  */
static void USART_Start_Receive_DMA(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_DMA(huart, (uint8_t*)uart_buf, MAX_DEPTH_UART1_BUF);

	/*!< Enable the Idle interrupt.
	Attention: REMEMBER to add IDLE Interrupt handler to stm324xx_it.c.

	[...] The Idle ISR code is following

				uint32_t tmp1, tmp2;
				tmp1 = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE);
				tmp2 = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_IDLE);

				if((tmp1 != RESET) && (tmp2 != RESET))
				{
					__HAL_UART_CLEAR_IDLEFLAG(&huart1);
					My_HAL_UART_IdleCallback(&huart1);
				}
	[...]
	*/
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
}

/**
  * @brief  The callback function of rx completion ISR
  * @param  huart: the owner of ISR
  * @retval	null
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		USART_Start_Receive_DMA(huart);
}



/**
  *@}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT AMC *****END OF FILE****/
