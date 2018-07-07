#include "extis.h"
#include "key.h"
#include "delay.h"
#include "led.h"
#include "beep.h"

/*初始化外部中断的步骤:首先初始化对应外部中断GPIO口为上拉/下拉输入模式,
然后开启AFIO时钟,配置和初始化中断线,配置和初始化NVIC,最后写中断函数

注意:对应IO口的外部中断线有16个,但是外部中断通道只有7个，
0..4有单独的中断通道,其中5-9,10-15共用一个中断通道,如果
5、6同时设置中断他们无论谁的中断被触发都会进入EXTI9_5_IRQHandler()
此时就需要在中断中通过判断EXTI_GetITStatus(EXTI_Linex)来判断到底是是触发了中断
*/
void EXTIS_Init()
{
	EXTI_InitTypeDef EXTI_InitStrue;
	NVIC_InitTypeDef NVIC_InitStrue;
	
	KEY_Init();//按键初始化
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//使能AFIO时钟(在端口重映射、外部中断配置、事件寄存器操作前必须配置AFIO)
	//以下是外部中断和中断线映射的配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);//IO口与中断线映射函数 (KEY1)
	EXTI_InitStrue.EXTI_Line = EXTI_Line4;										//外部中断线4
	EXTI_InitStrue.EXTI_Mode = EXTI_Mode_Interrupt;						//中断模式
	EXTI_InitStrue.EXTI_Trigger = EXTI_Trigger_Rising;				//上升沿触发(作为按键时最好检测放手沿)
	EXTI_InitStrue.EXTI_LineCmd = ENABLE;											//使能中断线
	EXTI_Init(&EXTI_InitStrue);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource3);//(KEY2)
	EXTI_InitStrue.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStrue.EXTI_Line = EXTI_Line3;//exti3
	EXTI_Init(&EXTI_InitStrue);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource2);//(KEY3)
	EXTI_InitStrue.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStrue.EXTI_Line = EXTI_Line2;//exti2
	EXTI_Init(&EXTI_InitStrue);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);//(KEY4)
	EXTI_InitStrue.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStrue.EXTI_Line = EXTI_Line0;//exti0
	EXTI_Init(&EXTI_InitStrue);
	
	//以下是中断优先级初始化的配置
	NVIC_InitStrue.NVIC_IRQChannel = EXTI4_IRQn;					//中断通道
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 2;	//抢占优先级
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 0;				//响应优先级
	NVIC_InitStrue.NVIC_IRQChannelCmd = ENABLE;						//使能中断优先级
	NVIC_Init(&NVIC_InitStrue);
	
	NVIC_InitStrue.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStrue.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStrue);
	
	NVIC_InitStrue.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStrue.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStrue);
	
	NVIC_InitStrue.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStrue.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStrue);
	
}

void EXTI4_IRQHandler()
{
	delay_ms(5);
	LED0 = !LED0;
	EXTI_ClearITPendingBit(EXTI_Line4);//清除挂起标志位
}	
#if 0
void EXTI3_IRQHandler()
{
	delay_ms(5);
	LED1 = !LED1;
	EXTI_ClearITPendingBit(EXTI_Line3);
}
#endif
void EXTI2_IRQHandler()
{
	delay_ms(5);
	LED0 = !LED0;
	LED1 = !LED1;
	EXTI_ClearITPendingBit(EXTI_Line2);
}

void EXTI0_IRQHandler()
{
	delay_ms(5);
	BEEP = !BEEP;
	EXTI_ClearITPendingBit(EXTI_Line0);
}


