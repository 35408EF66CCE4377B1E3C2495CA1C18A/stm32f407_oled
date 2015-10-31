#include "admin_interface.h"
#include "usart.h"
#include "string.h"
#include "cmsis_os.h"
#include "usart_common.h"


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
			strcpy((char*)cmdstructure->para, "?");
			return aifOK;
		}
		else				// Unknown command
		{
			return aifBadPara;
		}
	}
}




