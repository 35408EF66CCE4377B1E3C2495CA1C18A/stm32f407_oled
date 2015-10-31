#ifndef __SIM900A_H__
#define __SIM900A_H__
#include <stdint.h>


#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8		//高低字节交换宏定义

void sim900a_test(void);//sim900a主测试程序
void sim_send_sms(uint8_t*phonenumber,uint8_t*msg);
void sim_at_response(uint8_t mode);
uint8_t* sim900a_check_cmd(uint8_t *str);
uint8_t sim900a_send_cmd(uint8_t *cmd,uint8_t *ack,uint16_t waittime);
uint8_t sim900a_chr2hex(uint8_t chr);
uint8_t sim900a_hex2chr(uint8_t hex);
void sim900a_unigbk_exchange(uint8_t *src,uint8_t *dst,uint8_t mode);
void sim900a_load_keyboard(uint16_t x,uint16_t y,uint8_t **kbtbl);
void sim900a_key_staset(uint16_t x,uint16_t y,uint8_t keyx,uint8_t sta);
uint8_t sim900a_get_keynum(uint16_t x,uint16_t y);
uint8_t sim900a_call_test(void);			//拨号测试
void sim900a_sms_read_test(void);	//读短信测试
void sim900a_sms_send_test(void);	//发短信测试
void sim900a_sms_ui(uint16_t x,uint16_t y);	//短信测试UI界面函数
uint8_t sim900a_sms_test(void);			//短信测试
void sim900a_mtest_ui(uint16_t x,uint16_t y);	//SIM900A主测试UI
uint8_t sim900a_gsminfo_show(uint16_t x,uint16_t y);//显示GSM模块信息
void sim900a_test(void);			//SIM900A主测试函数
#endif





