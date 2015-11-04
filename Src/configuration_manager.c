/**
  ******************************************************************************
  * @file    configuration_manager.c.c
  * @author  Mikk Su
  * @version V1.0.0
  * @date    4-Nov-2015
  * @brief   Use internal flash to save the system configuration
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
#include "configuration_manager.h"
#include <string.h>

/* Private variables -----------------------------------------------------------*/
static FLASH_EraseInitTypeDef EraseInitStruct;	/*!< parameters structure of erasing internal flash */

/* Global variables -----------------------------------------------------------*/
ConfigurationTypeDef SystemConfig;		/*!< Global variable of config storage, it should be used in whole project */


/* Exported functions ---------------------------------------------------------*/
/** @defgroup Configuratoin_Manager_Exported_Functions Configuration_Manager Exported Functions
  * @{
  */


/**
  * @brief  Read configurations from the flash to memory
  * @param	null
  * @retval null
  */
void ReadConfiguration(void)
{
	memcpy(&SystemConfig, (void const*)CONFIG_START_ADDRESS, sizeof(ConfigurationTypeDef));
}

/**
  * @brief  Method to save configurations
  * @param	null
  * @retval	null
  */
HAL_StatusTypeDef SaveConfiguration(void)
{
	uint32_t bytes_to_save;
	uint32_t address_to_save;
	uint8_t *src_config;

	uint32_t SectorError;
	HAL_StatusTypeDef flash_status = HAL_OK;

	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
	EraseInitStruct .VoltageRange =VOLTAGE_RANGE_3 ;
	EraseInitStruct.Sector = FLASH_SECTOR_11 ;
	EraseInitStruct.NbSectors = 1;
	/*
	* 以上,定义删除类型是SECTORS就是块,提供的电压是3.3v的所以选VOLTAGE_RANGE_3
	* 从FLASH_SECTOR_11也就是第十一块开始删除,删除的数量是1个
	*/
	flash_status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

	bytes_to_save = sizeof(ConfigurationTypeDef);
	address_to_save = CONFIG_START_ADDRESS;
	src_config = (uint8_t*)&SystemConfig;

	while(bytes_to_save > 0)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address_to_save, (uint64_t)*src_config);
		bytes_to_save--;
		src_config++;
		address_to_save++;
	}
	HAL_FLASH_Lock();
	return flash_status;
}

/**
  * @}
  */

/************************ (C) COPYRIGHT AMC *****END OF FILE****/

