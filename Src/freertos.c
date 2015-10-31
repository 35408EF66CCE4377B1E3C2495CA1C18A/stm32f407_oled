/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <string.h>
#include "oled.h"
#include "icons.h"
#include "gpio.h"
#include "admin_interface.h"
#include "configuration_manager.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId uart1TaskHandle;
osMessageQId qUart1Handle;
osMessageQId qUart2Handle;
osSemaphoreId sem_startfftHandle;
osSemaphoreId semWaitW550SpiDmaHandle;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartTaskUart1(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of sem_startfft */
  osSemaphoreDef(sem_startfft);
  sem_startfftHandle = osSemaphoreCreate(osSemaphore(sem_startfft), 1);

  /* definition and creation of semWaitW550SpiDma */
  osSemaphoreDef(semWaitW550SpiDma);
  semWaitW550SpiDmaHandle = osSemaphoreCreate(osSemaphore(semWaitW550SpiDma), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of uart1Task */
  osThreadDef(uart1Task, StartTaskUart1, osPriorityIdle, 0, 256);
  uart1TaskHandle = osThreadCreate(osThread(uart1Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of qUart1 */
  osMessageQDef(qUart1, 5, uint32_t);
  qUart1Handle = osMessageCreate(osMessageQ(qUart1), NULL);

  /* definition and creation of qUart2 */
  osMessageQDef(qUart2, 5, uint32_t);
  qUart2Handle = osMessageCreate(osMessageQ(qUart2), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */



  	OLED_Init();
//  OLED_ShowString(0,0,"123",24);
//	OLED_ShowString(0,24, "0.96' OLED TEST",16);
// 	OLED_ShowString(0,40,"ATOM 2014/5/4",12);
// 	OLED_ShowString(0,52,"ASCII:",12);
// 	OLED_ShowString(64,52,"CODE:",12);
	OLED_ShowIcon(0, 0, signal_none);
	OLED_Refresh_Gram();//更新显示到OLED




  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
	osDelay(5);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
  }
  /* USER CODE END StartDefaultTask */
}

/* StartTaskUart1 function */
void StartTaskUart1(void const * argument)
{
  /* USER CODE BEGIN StartTaskUart1 */
	static uint8_t *pCommand;
	static CommandTypeDef ParsedCommand;
	uint64_t tmp;
	aifStatus retval_parsecmd;

	// Initial the Admin Interface
	InitAdminInterface();

  /* Infinite loop */
  for(;;)
  {
	// Wait for a message from the Uart1 ISR
	//printf("  Wait for next command....\r\n");
	WaitForCommand(&pCommand);
	//printf("  Parse command...\r\n");
	//printf("Command: %s\r\n", (uint8_t*)pCommand);

	retval_parsecmd = ParseCommand(pCommand, &ParsedCommand);
	//printf("  Command parsed!\r\n");

	if(retval_parsecmd == aifOK)
	{

		//printf("Command Name:%s\r\nCommand Para:%s\r\n", ParsedCommand.command, ParsedCommand.para);

		if(strcmp((char*)ParsedCommand.command, "CELL0") == 0)
		{
			if(*ParsedCommand.para == '?')
			{
				printf("Duty Room Phone:%lld\r\n", SystemConfig.DutyRoomNumber);
			}
			else
			{
				tmp = atoll((char*)ParsedCommand.para);
				SystemConfig.DutyRoomNumber = tmp;
			}
		}
		else if(strcmp((char*)ParsedCommand.command, "CELL1") == 0)
		{
			if(*ParsedCommand.para == '?')
			{
				printf("Watch-Keeper_1 Phone:%lld\r\n", SystemConfig.WatchKeeper1);
			}
			else
			{
				tmp = atoll((char*)ParsedCommand.para);
				SystemConfig.WatchKeeper1 = tmp;
			}
		}
		else if(strcmp((char*)ParsedCommand.command, "CELL2") == 0)
		{
			if(*ParsedCommand.para == '?')
			{
				printf("Watch-Keeper_2 Phone:%lld\r\n", SystemConfig.WatchKeeper2);
			}
			else
			{
				tmp = atoll((char*)ParsedCommand.para);
				SystemConfig.WatchKeeper2 = tmp;
			}
		}

		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
		osDelay(10);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
	}
	else
	{

		//printf("Command parsing error: %d\r\n", retval_parsecmd);

		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
		osDelay(10);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
		osDelay(50);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
		osDelay(10);
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
	}
  }
  /* USER CODE END StartTaskUart1 */
}

/* USER CODE BEGIN Application */



/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
