#ifndef __ADMIN_OPERATION_H
#define __ADMIN_OPERATION_H

#include <stdint.h>

#define MAX_COUNT_UART1_QUEUE	5
#define MAX_DEPTH_UART1_QUEUE	50

#define MAX_LEN_CMD_NAME	15
#define MAX_LEN_CMD_PARA	35


typedef struct
{
	uint8_t command[MAX_LEN_CMD_NAME];
	uint8_t para[MAX_LEN_CMD_PARA];
}CommandTypeDef;

typedef enum
{
	aifOK,
	aifBadName,
	aifBadPara
} aifStatus;

void InitAdminInterface(void);
void WaitForCommand(uint8_t **Command);
aifStatus ParseCommand(uint8_t *command, CommandTypeDef *cmdstructure);

#endif