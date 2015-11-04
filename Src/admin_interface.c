/**
  ******************************************************************************
  * @file    usart_common.c
  * @author  Mikk Su
  * @version V1.0.0
  * @date    4-Nov-2015
  * @brief   Contains some logics to deal with admin commands
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

#include "admin_interface.h"
#include "usart.h"
#include "usart_common.h"
#include "configuration_manager.h"
#include "debug.h"

/* Private macro -------------------------------------------------------------*/
// Mac example: 00:00:00:00:00
#define IS_MAC(mac)	((*(mac + 2) == ':') && \
					(*(mac + 5) == ':') && \
					(*(mac + 8) == ':') && \
					(*(mac + 11) == ':'))			/*!< Check the format of the MAC string */
// IP example: 192.168.1.1
#define IS_IP(ip) 	((*(ip + 2) == ':') && \
					(*(ip + 5) == ':') && \
					(*(ip + 8) == ':') && \
					(*(ip + 11) == ':'))			/*!< Check the format of the IP string */

/* Private variables -----------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void ConvertMacStrToByteArray(const char* mac_in, uint8_t* mac_out);
static void ConvertIPStringToByteArray(const char* ip_in, uint8_t* ip_out);
static int ValidateIPv4(const char* str, uint16_t* ip);


/* Private functions ---------------------------------------------------------*/
/** @defgroup AdminIF_Private_Functions Admin Interface Private Functions
  * @{
  */

/**
  * @brief  Convert MAC string to byte array.
  * @param  mac_in: The string contains MAC address.
	* @param	mac_out: The byte array contains MAC address.
  * @retval	null
  */
static void ConvertMacStrToByteArray(const char* mac_in, uint8_t* mac_out)
{
	int i;
	uint16_t tmp;
	if(IS_MAC(mac_in))
	{
		for(i = 0; i < 6; i++)
		{
			tmp = strtol(mac_in, NULL, 16);
			*mac_out = (uint8_t)tmp;
			mac_out++;
			mac_in += 3;
		}
	}
}


/**
  * @brief  Convert IP string to byte array.
  * @param  mac_in: The string contains IP address.
	* @param	mac_out: The byte array contains IP address.
  * @retval	null
  */
static void ConvertIPStringToByteArray(const char* ip_in, uint8_t* ip_out)
{
	uint16_t ip[4];
	if(ValidateIPv4(ip_in, ip))	// Validate the input string
	{
		*ip_out++ = (uint8_t)ip[0];
		*ip_out++ = (uint8_t)ip[1];
		*ip_out++ = (uint8_t)ip[2];
		*ip_out = (uint8_t)ip[3];
	}
}

/**
  * @brief  Valitdate if the input string matches the IPv4 format.
  * @param  str: The pointer to the input string
  * @retval	0:not match; 1:match
  */
static int ValidateIPv4(const char* str, uint16_t* ip)
{
    int len = strlen(str);

    if (len < 7 || len > 15)
        return 0;

    char tail[16];
    tail[0] = 0;

    int c = sscanf(str, "%3u.%3u.%3u.%3u%s", &ip[0], &ip[1], &ip[2], &ip[3], tail);

    if (c != 4 || tail[0])
        return 0;

    for (int i = 0; i < 4; i++)
        if (ip[i] > 255)
            return 0;

    return 1;
}

/**
  *@}
  */


/* Exported functions ---------------------------------------------------------*/
/** @defgroup AdminIF_Exported_Functions Admin Interface Exported Functions
  * @{
  */


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
  * @brief  Initialize the admin management interface
  * @param  null
  * @retval	null
  */
void InitAdminManagementInterface(void)
{
	USART_COMM_Init_AdminManagementPort();
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
  * @param  parsedcommand: Pointer to the command which has been parsed.
  * @retval	null
  */
void ExcuteCommand(CommandTypeDef *parsedcommand)
{
	uint64_t tmp;

	if(strcmp((char*)parsedcommand->command, "SAVE") == 0)
	{
		HAL_StatusTypeDef retval = SaveConfiguration();
		if(retval != HAL_OK)
		{
			EchoCommand("Failed\r\n");
			printk(KERN_DEBUG "Config saving failed, retval = %d\r\n", retval);
		}
		else
		{
			EchoCommand("OK\r\n");
		}
	}
	else if(strcmp((char*)parsedcommand->command, "CELL0") == 0)
	{
		if(*parsedcommand->para == '?')
		{
			EchoCommand("Duty Room Phone:%lld\r\n", SystemConfig.DutyRoomNumber);
		}
		else
		{
			tmp = atoll((char*)parsedcommand->para);
			SystemConfig.DutyRoomNumber = tmp;
			EchoCommand("OK\r\n");
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
			EchoCommand("OK\r\n");
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
			EchoCommand("OK\r\n");
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
			ConvertMacStrToByteArray((char*)parsedcommand->para, SystemConfig.Mac);
			EchoCommand("OK\r\n");
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
			ConvertIPStringToByteArray((char*)parsedcommand->para, SystemConfig.IP);
			EchoCommand("OK\r\n");
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
			ConvertIPStringToByteArray((char*)parsedcommand->para, SystemConfig.Gateway);
			EchoCommand("OK\r\n");
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
			ConvertIPStringToByteArray((char*)parsedcommand->para, SystemConfig.SubMask);
			EchoCommand("OK\r\n");
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
			ConvertIPStringToByteArray((char*)parsedcommand->para, SystemConfig.Dns);
			EchoCommand("OK\r\n");
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

/**
	*@}
	*/
/************************ (C) COPYRIGHT AMC *****END OF FILE****/



