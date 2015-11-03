#include "admin_interface.h"
#include "usart.h"
#include "string.h"
#include "cmsis_os.h"
#include "usart_common.h"
#include "configuration_manager.h"
#include "debug.h"

/**
  * @brief  respond to the control command
  * @param  echo: response string, terminated by “\r\n"
  * @retval
  */
void EchoCommand(const char* echo, ...)
{
	va_list ap;
	va_start(ap, echo);
	USART_COMM_TransmitCommand(echo, ap);
	va_end(ap);
}

/**
  * @brief  Initialize the admin interface
  * @param  null
  * @retval	null
  */
void InitAdminInterface(void)
{
	uart1_buf_row = 0;
	uart1_buf_using = &uart1_buf[uart1_buf_row][0];
	HAL_UART_Receive_IT(&huart1, uart1_buf_using, 1);
}

/**
  * @brief  Await the next command reception.
  *			There is a special task awaiting and handling the command,
  *			the task is suspended most of the time except a command received.
  * @param  command:	Pointer which point to the address of the command received,
  *						it transmit the address of the head of the command to the caller.
  * @retval	null
  */
void WaitForCommand(uint8_t **command)
{
	osEvent qUart1_reval;
	qUart1_reval = osMessageGet(qUart1Handle, osWaitForever);
	if(qUart1_reval.status == osEventMessage)
		*command = qUart1_reval.value.p;
	else
		*command = NULL;
}

/**
  * @brief  Command excution
  * @param  null
  * @retval	null
  */
void ExcuteCommand(CommandTypeDef *parsedcommand)
{
	uint64_t tmp;

	if(strcmp((char*)parsedcommand->command, "CELL0") == 0)
	{
		if(*parsedcommand->para == '?')
		{
			EchoCommand("Duty Room Phone:%lld\r\n", SystemConfig.DutyRoomNumber);
		}
		else
		{
			tmp = atoll((char*)parsedcommand->para);
			SystemConfig.DutyRoomNumber = tmp;
		}
	}
	else if(strcmp((char*)parsedcommand->command, "CELL1") == 0)
	{
		if(*parsedcommand->para == '?')
		{
			EchoCommand("Watch-Keeper_1 Phone:%lld\r\n", SystemConfig.WatchKeeper1);
		}
		else
		{
			tmp = atoll((char*)parsedcommand->para);
			SystemConfig.WatchKeeper1 = tmp;
		}
	}
	else if(strcmp((char*)parsedcommand->command, "CELL2") == 0)
	{
		if(*parsedcommand->para == '?')
		{
			EchoCommand("Watch-Keeper_2 Phone:%lld\r\n", SystemConfig.WatchKeeper2);
		}
		else
		{
			tmp = atoll((char*)parsedcommand->para);
			SystemConfig.WatchKeeper2 = tmp;
		}
	}
	else if(strcmp((char*)parsedcommand->command, "MAC") == 0)
	{
		if(*parsedcommand->para == '?')
		{
				EchoCommand("MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
								SystemConfig.Mac[0],
								SystemConfig.Mac[1],
								SystemConfig.Mac[2],
								SystemConfig.Mac[3],
								SystemConfig.Mac[4],
								SystemConfig.Mac[5]);
		}
		else
		{
			//tmp = atoll((char*)parsedcommand.para);
			//SystemConfig.WatchKeeper2 = tmp;
		}
	}
	else if(strcmp((char*)parsedcommand->command, "IP") == 0)
	{
		if(*parsedcommand->para == '?')
		{
				EchoCommand("IP: %d.%d.%d.%d\r\n",
								SystemConfig.IP[0],
								SystemConfig.IP[1],
								SystemConfig.IP[2],
								SystemConfig.IP[3]);
		}
		else
		{
			//tmp = atoll((char*)parsedcommand.para);
			//SystemConfig.WatchKeeper2 = tmp;
		}
	}
	else if(strcmp((char*)parsedcommand->command, "GW") == 0)
	{
		if(*parsedcommand->para == '?')
		{
				EchoCommand("GW: %d.%d.%d.%d\r\n",
								SystemConfig.Gateway[0],
								SystemConfig.Gateway[1],
								SystemConfig.Gateway[2],
								SystemConfig.Gateway[3]);
		}
		else
		{
			//tmp = atoll((char*)parsedcommand.para);
			//SystemConfig.WatchKeeper2 = tmp;
		}
	}
	else if(strcmp((char*)parsedcommand->command, "SM") == 0)
	{
		if(*parsedcommand->para == '?')
		{
				EchoCommand("SUB: %d.%d.%d.%d\r\n",
								SystemConfig.SubMask[0],
								SystemConfig.SubMask[1],
								SystemConfig.SubMask[2],
								SystemConfig.SubMask[3]);
		}
		else
		{
			//tmp = atoll((char*)parsedcommand.para);
			//SystemConfig.WatchKeeper2 = tmp;
		}
	}
	else if(strcmp((char*)parsedcommand->command, "DNS") == 0)
	{
		if(*parsedcommand->para == '?')
		{
				EchoCommand("DNS: %d.%d.%d.%d\r\n",
								SystemConfig.Dns[0],
								SystemConfig.Dns[1],
								SystemConfig.Dns[2],
								SystemConfig.Dns[3]);
		}
		else
		{
			//tmp = atoll((char*)parsedcommand.para);
			//SystemConfig.WatchKeeper2 = tmp;
		}
	}
}

/**
  * @brief  Split the command string to two parts, which is made up of name and parameters
  * @param  command: pointer of the command string
  *			cmdstructure: structure of command
  * @retval	null
  */
aifStatus ParseCommand(uint8_t *command, CommandTypeDef *cmdstructure)
{
	int cmd_cnt = 0;
	uint8_t* p_cmd_char = command;

	memset(cmdstructure, 0x0, sizeof(CommandTypeDef));

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




