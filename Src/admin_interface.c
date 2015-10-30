#include "admin_interface.h"
#include "usart.h"
#include "string.h"
#include "cmsis_os.h"


// The osMessageQId was defined in the file 'freertos.c'
extern osMessageQId qUart1Handle;

uint8_t uart1_buf[MAX_COUNT_UART1_QUEUE][MAX_DEPTH_UART1_QUEUE];
uint8_t uart1_buf_row;
uint8_t *uart1_buf_using;



void InitAdminInterface(void)
{
	uart1_buf_row = 0;
	uart1_buf_using = &uart1_buf[uart1_buf_row][0];
	HAL_UART_Receive_IT(&huart1, uart1_buf_using, 1);
}

void WaitForCommand(uint8_t **command)
{
	osEvent qUart1_reval;
	qUart1_reval = osMessageGet(qUart1Handle, osWaitForever);
	if(qUart1_reval.status == osEventMessage)
		*command = qUart1_reval.value.p;
	else
		*command = NULL;
}


aifStatus ParseCommand(uint8_t *command, CommandTypeDef *cmdstructure)
{
	int cmd_cnt = 0;
	uint8_t* p_cmd_char = command;

	memset(cmdstructure, 0x0, 1);

	// Search the name of the command
	while(*p_cmd_char != 0x0)
	{
		// Search the seperator...( ?=)
		if(*p_cmd_char == ' ' || *p_cmd_char == '?' || *p_cmd_char == '=')
		{
			break;	// The character space has been found
		}
		else
		{
			cmd_cnt++;
			p_cmd_char++;
		}
	}

	if(cmd_cnt >= MAX_LEN_CMD_NAME)	// cmd_name_len is out of range means the format of the command is error, OR the name of the command is to long (excced MAX_LEN_CMD_NAME)
	{
		return aifBadName;
	}
	else
	{
		memcpy(cmdstructure->command, command, cmd_cnt); 	// Copy the name to the structure

		if(*p_cmd_char == ' ' || *p_cmd_char == '=') 	// Value setting command
		{
			// Search the parameter of the command
			p_cmd_char++;			// Move the pointer to the start of the parameter
			command = p_cmd_char;	// Move the pointer in order to copy the parameter section
			cmd_cnt = 0;			// Reset the character counter
			while(*p_cmd_char != 0x0)
			{
				cmd_cnt++;
				p_cmd_char++;
			}

			memcpy(cmdstructure->para, command, cmd_cnt); 	// Copy the parameter to the structure
			return aifOK;
		}
		else if(*p_cmd_char == '?')						// Query commnd
		{
			strcpy(cmdstructure->para, "?");
			return aifOK;
		}
		else				// Unknown command
		{
			return aifBadPara;
		}
	}
}

//uint64_t atoll(char* string)
//{
//	uint64_t retval = 0;
//	uint64_t digt = 1;
//	char* p_char = string;
//	while(p_char != 0x0)
//		p_char++;
//
//	while(p_char >= string)
//	{
//		retval += (*(--p_char) * digt);
//		digt *= 10;
//	}
//
//	return digt;
//}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_UART_TxCpltCallback could be implemented in the user file
   */
	uint8_t *pUartData;

	pUartData = uart1_buf_using;

	if(*pUartData == '\n')
	{
		if(*(--pUartData) == '\r')
		{
			// Replace the \r to 0x0, which indicates the terminal of the command string
			*pUartData = 0x0;

			osMessagePut(qUart1Handle, (uint32_t)&uart1_buf[uart1_buf_row][0], 100);

			// The current buffer has been used and post to the working thread
			// switch to the next uart1 queue buffer to recevie the data
			uart1_buf_row ++;
			if(uart1_buf_row >= MAX_COUNT_UART1_QUEUE)
				uart1_buf_row = 0;

			uart1_buf_using = &uart1_buf[uart1_buf_row][0];
			HAL_UART_Receive_IT(&huart1, uart1_buf_using, 1);
		}
		else
		{
			// Running here means an error command was got OR a too long command was transmitted into the Buffer
			// Simply drop the received command and reuse the buffer
			uart1_buf_using = &uart1_buf[uart1_buf_row][0];
			HAL_UART_Receive_IT(&huart1, uart1_buf_using, 1);
		}
	}
	else
	{
		// Move the pointer to the next byte of the uart1 recipient
		uart1_buf_using++;

		// Check wether the 'uart1_buf_using' pointer was out of range
		if(uart1_buf_using > &uart1_buf[uart1_buf_row][MAX_DEPTH_UART1_QUEUE])
		{
			// The buffer was full, drop the following bytes..
			uart1_buf_using = &uart1_buf[uart1_buf_row][MAX_DEPTH_UART1_QUEUE];
		}

		HAL_UART_Receive_IT(&huart1, uart1_buf_using, 1);
	}
}