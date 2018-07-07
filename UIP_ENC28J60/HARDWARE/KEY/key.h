
#ifndef __KEY_H
#define __KEY_H
#include "sys.h"

//#define KEY0 PEin(4)	//PE4
//#define KEY1 PEin(3)	//PE3
//#define KEY2 PEin(2)	//PE2
//#define KEY3 PAin(0)	//PA0

#define KEY0 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
#define KEY1 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
#define KEY2 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)
#define KEY3 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)

#define KEY0_EFF 1	//按键按下返回的对应值
#define KEY1_EFF 2
#define KEY2_EFF 3
#define KEY3_EFF 4

void KEY_Init(void);
u8 KEY_Scan(u8 mode);

#endif


