#include <stdio.h>
#include "usart.h"

#ifdef _PRINTF_2_UART_

#if   defined ( __CC_ARM )		/* ARM Compiler (Keil) */



#elif defined ( __ICCARM__ )	/* IAR Compiler */

int fputc(int ch, FILE *f)
{
   HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 5);
   return ch;
}

#elif defined ( __GNUC__ )		/* GNU Compiler */


#endif

#endif