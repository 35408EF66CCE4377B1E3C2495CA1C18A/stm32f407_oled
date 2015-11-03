#include "configuration_manager.h"
#include <string.h>

static FLASH_EraseInitTypeDef EraseInitStruct;
ConfigurationTypeDef SystemConfig;		/* Global variable of config storage, it should be used in whole project */

/**
  * @brief  Read configurations from the flash to memory
  * @param  
  * @retval
  */
void ReadConfiguration(void)
{
	memcpy(&SystemConfig, (void const*)CONFIG_START_ADDRESS, sizeof(ConfigurationTypeDef));
}

/**
  * @brief  Method to save configurations
  * @param  
  * @retval
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
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address_to_save, (uint64_t)(*(uint32_t*)src_config));
		bytes_to_save -= 4;
		src_config += 4;
		address_to_save += 4;
	}
	HAL_FLASH_Lock();
	return flash_status;
}

