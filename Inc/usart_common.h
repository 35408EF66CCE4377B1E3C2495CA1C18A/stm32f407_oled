/**
  ******************************************************************************
  * @file    usart_common.h
  * @author  Mikk Su
  * @version V1.0.0
  * @date    4-Nov-2015
  * @brief   Head file of usart_common.c
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

#ifndef __USART_COMMON_H
#define __USART_COMMON_H

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "usart.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

/* Private variables -----------------------------------------------------------*/
#define MAX_COUNT_UART1_BUF	10		/*!< rows in the uart1 buffer */
#define MAX_DEPTH_UART1_BUF	50		/*!< length of each row of the uart1 buffer */

#define MAX_COUNT_UART2_BUF	5			/*!< rows in the uart2 buffer */
#define MAX_DEPTH_UART2_BUF	50		/*!< length of each row of the uart2 buffer */

extern osMessageQId qUart1Handle;	/*!< the uart1 message queue handler which is used to notify
																			the task that a command has been reccived via usart1 port.
																			the osMessageQId is defined in the file "freertos.c" */

extern osMessageQId qUart2Handle;	/*!< the uart2 message queue handler which is used to notify
																			the task that a AT command has been reccived via usart2 port.
																			the osMessageQId is defined in the file "freertos.c" */

/* Exported functions ---------------------------------------------------------*/
void USART_COMM_Init_AdminManagementPort(void);
void USART_COMM_TransmitDebugInfo(const char* content, va_list ap);
void USART_COMM_TransmitCommand(const char* command, va_list ap);
#endif
