#ifndef __OLED_H
#define __OLED_H
#include "sys.h"
#include "stdlib.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//OLED 驱动代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/4
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////


//OLED模式设置
//0: 4线串行模式  （模块的BS1，BS2均接GND）
//1: 并行8080模式 （模块的BS1，BS2均接VCC）
#define OLED_MODE 	0

/*
//-----------------OLED端口定义----------------
#define OLED_CS 	PBout(7)
#define OLED_RST  PGout(15)
#define OLED_RS 	PDout(6)
#define OLED_WR 	PAout(4)
#define OLED_RD 	PDout(7)

//使用4线串行接口时使用
#define OLED_SCLK 	PCout(6)
#define OLED_SDIN 	PCout(7)
*/

//-----------------OLED端口定义----------------
#define OLED_CS 	PBout(7)
#define OLED_RST  PGout(15)
#define OLED_RS 	PBout(5)
//#define OLED_WR 	PAout(4)
//#define OLED_RD 	PDout(7)

//使用4线串行接口时使用
//#define OLED_SCLK 	PCout(10)
//#define OLED_SDIN 	PCout(12)

#define OLED_CMD  	0		//写命令
#define OLED_DATA 	1		//写数据

//OLED控制用函数
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);

void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size);
void OLED_ShowIcon(uint8_t x, uint8_t y, const uint8_t *icon);
#endif




