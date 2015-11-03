#ifndef __ADMIN_OPERATION_H
#define __ADMIN_OPERATION_H

#include <stdint.h>
#include <stdlib.h>

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
void ExcuteCommand(CommandTypeDef *parsedcommand);
void EchoCommand(const char*, ...);
#endif