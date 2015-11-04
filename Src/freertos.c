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
#include "ethernet_common.h"
#include "debug.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "loopback.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId uart1TaskHandle;
osThreadId ethInitTaskHandle;
osThreadId eth0TransTaskHandle;
osThreadId eth1TransTaskHandle;
osThreadId eth2TransTaskHandle;
osThreadId eth3TransTaskHandle;
osThreadId eth4TransTaskHandle;
osMessageQId qUart1Handle;
osMessageQId qUart2Handle;
osSemaphoreId semW5500TxCpltHandle;
osSemaphoreId semW5500RxCpltHandle;
osSemaphoreId semEthSpiIdleHandle;
osSemaphoreId semEth0IntHandle;
osSemaphoreId semEth1IntHandle;
osSemaphoreId semEth2IntHandle;
osSemaphoreId semEth3IntHandle;
osSemaphoreId semEth4IntHandle;

/* USER CODE BEGIN Variables */
////////////////////////////////////////////////
// Shared Buffer Definition for LOOPBACK TEST //
////////////////////////////////////////////////
#define DATA_BUF_SIZE   2048
uint8_t gDATABUF[DATA_BUF_SIZE];
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartTaskUart1(void const * argument);
void StartEthInitTask(void const * argument);
void StartEth0TransTask(void const * argument);
void StartEth1TransTask(void const * argument);
void StartEth2TransTask(void const * argument);
void StartEth3TransTask(void const * argument);
void StartEth4TransTask(void const * argument);

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
  /* definition and creation of semW5500TxCplt */
  osSemaphoreDef(semW5500TxCplt);
  semW5500TxCpltHandle = osSemaphoreCreate(osSemaphore(semW5500TxCplt), 1);

  /* definition and creation of semW5500RxCplt */
  osSemaphoreDef(semW5500RxCplt);
  semW5500RxCpltHandle = osSemaphoreCreate(osSemaphore(semW5500RxCplt), 1);

  /* definition and creation of semEthSpiIdle */
  osSemaphoreDef(semEthSpiIdle);
  semEthSpiIdleHandle = osSemaphoreCreate(osSemaphore(semEthSpiIdle), 1);

  /* definition and creation of semEth0Int */
  osSemaphoreDef(semEth0Int);
  semEth0IntHandle = osSemaphoreCreate(osSemaphore(semEth0Int), 1);

  /* definition and creation of semEth1Int */
  osSemaphoreDef(semEth1Int);
  semEth1IntHandle = osSemaphoreCreate(osSemaphore(semEth1Int), 1);

  /* definition and creation of semEth2Int */
  osSemaphoreDef(semEth2Int);
  semEth2IntHandle = osSemaphoreCreate(osSemaphore(semEth2Int), 1);

  /* definition and creation of semEth3Int */
  osSemaphoreDef(semEth3Int);
  semEth3IntHandle = osSemaphoreCreate(osSemaphore(semEth3Int), 1);

  /* definition and creation of semEth4Int */
  osSemaphoreDef(semEth4Int);
  semEth4IntHandle = osSemaphoreCreate(osSemaphore(semEth4Int), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  // Clear the semaphores
  osSemaphoreWait(semEth0IntHandle, osWaitForever);
  osSemaphoreWait(semW5500TxCpltHandle, osWaitForever);
  osSemaphoreWait(semW5500RxCpltHandle, osWaitForever);
	osSemaphoreWait(semEthSpiIdleHandle, osWaitForever);
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
  osThreadDef(uart1Task, StartTaskUart1, osPriorityNormal, 0, 256);
  uart1TaskHandle = osThreadCreate(osThread(uart1Task), NULL);

  /* definition and creation of ethInitTask */
  osThreadDef(ethInitTask, StartEthInitTask, osPriorityRealtime, 0, 128);
  ethInitTaskHandle = osThreadCreate(osThread(ethInitTask), NULL);

  /* definition and creation of eth0TransTask */
  osThreadDef(eth0TransTask, StartEth0TransTask, osPriorityNormal, 0, 64);
  eth0TransTaskHandle = osThreadCreate(osThread(eth0TransTask), NULL);

  /* definition and creation of eth1TransTask */
  osThreadDef(eth1TransTask, StartEth1TransTask, osPriorityNormal, 0, 64);
  eth1TransTaskHandle = osThreadCreate(osThread(eth1TransTask), NULL);

  /* definition and creation of eth2TransTask */
  osThreadDef(eth2TransTask, StartEth2TransTask, osPriorityNormal, 0, 64);
  eth2TransTaskHandle = osThreadCreate(osThread(eth2TransTask), NULL);

  /* definition and creation of eth3TransTask */
  osThreadDef(eth3TransTask, StartEth3TransTask, osPriorityNormal, 0, 64);
  eth3TransTaskHandle = osThreadCreate(osThread(eth3TransTask), NULL);

  /* definition and creation of eth4TransTask */
  osThreadDef(eth4TransTask, StartEth4TransTask, osPriorityNormal, 0, 64);
  eth4TransTaskHandle = osThreadCreate(osThread(eth4TransTask), NULL);

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
	static uint8_t *pcommand;
	static CommandTypeDef parsedcommand;
	aifStatus retval_parsecmd;

	// Initial the Admin Interface
	InitAdminManagementInterface();

  /* Infinite loop */
  for(;;)
  {
		// Wait for a message from the Uart1 ISR
		//printk("  Wait for next command....\r\n");
		WaitForCommand(&pcommand);
		//printk("  Parse command...\r\n");
		//printk("Command: %s\r\n", (uint8_t*)pCommand);

		retval_parsecmd = ParseCommand(pcommand, &parsedcommand);
		//printk("  Command parsed!\r\n");

		if(retval_parsecmd == aifOK)
		{
			ExcuteCommand(&parsedcommand);
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
			osDelay(10);
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
		}
		else
		{

			//printk("Command parsing error: %d\r\n", retval_parsecmd);

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

/* StartEthInitTask function */
void StartEthInitTask(void const * argument)
{
  /* USER CODE BEGIN StartEthInitTask */

  // hardware reset all w5500 modules
	HardwareResetAllChips();

  // Inititalize the network of ethernet modules
  InitEternetInterface(W5500_0);
  InitEternetInterface(W5500_1);

	printk("StartEthInitTask has been terminated\r\n");
	osThreadTerminate(NULL);

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartEthInitTask */
}

/* StartEth0TransTask function */
void StartEth0TransTask(void const * argument)
{
  /* USER CODE BEGIN StartEth0TransTask */
  osStatus sem_retval;

  /* Infinite loop */
  for(;;)
  {
    // Await the data reception interruption...
		sem_retval = osSemaphoreWait(semEth0IntHandle, osWaitForever);
    if(sem_retval == osOK)
    {
      // If the Spi bus is busy, await the semphore...
      sem_retval = osSemaphoreWait(semEthSpiIdleHandle, osWaitForever);
      if(sem_retval == osOK)
      {

      }
    }
  }
  /* USER CODE END StartEth0TransTask */
}

/* StartEth1TransTask function */
void StartEth1TransTask(void const * argument)
{
  /* USER CODE BEGIN StartEth1TransTask */
  osStatus sem_retval;

  /* Infinite loop */
  for(;;)
  {
    // Await the data reception interruption...
		sem_retval = osSemaphoreWait(semEth1IntHandle, osWaitForever);
    if(sem_retval == osOK)
    {
      // If the Spi bus is busy, await the semphore...
      sem_retval = osSemaphoreWait(semEthSpiIdleHandle, osWaitForever);
      if(sem_retval == osOK)
      {

      }
    }
  }
  /* USER CODE END StartEth1TransTask */
}

/* StartEth2TransTask function */
void StartEth2TransTask(void const * argument)
{
  /* USER CODE BEGIN StartEth2TransTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartEth2TransTask */
}

/* StartEth3TransTask function */
void StartEth3TransTask(void const * argument)
{
  /* USER CODE BEGIN StartEth3TransTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartEth3TransTask */
}

/* StartEth4TransTask function */
void StartEth4TransTask(void const * argument)
{
  /* USER CODE BEGIN StartEth4TransTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartEth4TransTask */
}

/* USER CODE BEGIN Application */



/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
