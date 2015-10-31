#include "stm32f4xx_hal.h"
#include "usart_common.h"
#include "usart.h"


uint8_t **uart_buf_using;
uint8_t *uart_buf_row;
uint8_t (*uart_buf)[MAX_DEPTH_UART1_BUF];
osMessageQId *qUart;

uint8_t uart1_buf[MAX_COUNT_UART1_BUF][MAX_DEPTH_UART1_BUF];
uint8_t uart1_buf_row;
uint8_t *uart1_buf_using;


uint8_t uart2_buf[MAX_COUNT_UART2_BUF][MAX_DEPTH_UART2_BUF];
uint8_t uart2_buf_row;
uint8_t *uart2_buf_using;

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



			//printf("  1, RXNEIE = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE));
			uartretval = HAL_UART_Receive_IT(huart, *uart_buf_using, 1);
			//printf("  2, RXNEIE = %d, retval = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE), uartretval);
		}
		else
		{
			// Running here means an error command was got OR a too long command was transmitted into the Buffer
			// Simply drop the received command and reuse the buffer
			*uart_buf_using = (uint8_t*)(uart_buf + *uart_buf_row);

			//printf("  8, RXNEIE = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE));
			HAL_UART_Receive_IT(huart, *uart_buf_using, 1);
			//printf("  9, RXNEIE = %d\r\n", __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE));
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