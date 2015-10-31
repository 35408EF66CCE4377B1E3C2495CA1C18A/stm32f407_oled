#ifndef __USART_COMMON_H
#define __USART_COMMON_H

#include "cmsis_os.h"

#define MAX_COUNT_UART1_BUF	10
#define MAX_DEPTH_UART1_BUF	50

#define MAX_COUNT_UART2_BUF	5
#define MAX_DEPTH_UART2_BUF	50

// The osMessageQId was defined in the file 'freertos.c'
extern osMessageQId qUart1Handle;

extern uint8_t uart1_buf[MAX_COUNT_UART1_BUF][MAX_DEPTH_UART1_BUF];
extern uint8_t uart1_buf_row;
extern uint8_t *uart1_buf_using;


// The osMessageQId was defined in the file 'freertos.c'
extern osMessageQId qUart2Handle;

extern uint8_t uart2_buf[MAX_COUNT_UART2_BUF][MAX_DEPTH_UART2_BUF];
extern uint8_t uart2_buf_row;
extern uint8_t *uart2_buf_using;


void StartReceive(UART_HandleTypeDef *huart);
#endif
