#include "extis.h"
#include "key.h"
#include "delay.h"
#include "led.h"
#include "beep.h"

/*��ʼ���ⲿ�жϵĲ���:���ȳ�ʼ����Ӧ�ⲿ�ж�GPIO��Ϊ����/��������ģʽ,
Ȼ����AFIOʱ��,���úͳ�ʼ���ж���,���úͳ�ʼ��NVIC,���д�жϺ���

ע��:��ӦIO�ڵ��ⲿ�ж�����16��,�����ⲿ�ж�ͨ��ֻ��7����
0..4�е������ж�ͨ��,����5-9,10-15����һ���ж�ͨ��,���
5��6ͬʱ�����ж���������˭���жϱ������������EXTI9_5_IRQHandler()
��ʱ����Ҫ���ж���ͨ���ж�EXTI_GetITStatus(EXTI_Linex)���жϵ������Ǵ������ж�
*/
void EXTIS_Init()
{
	EXTI_InitTypeDef EXTI_InitStrue;
	NVIC_InitTypeDef NVIC_InitStrue;
	
	KEY_Init();//������ʼ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//ʹ��AFIOʱ��(�ڶ˿���ӳ�䡢�ⲿ�ж����á��¼��Ĵ�������ǰ��������AFIO)
	//�������ⲿ�жϺ��ж���ӳ�������
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);//IO�����ж���ӳ�亯�� (KEY1)
	EXTI_InitStrue.EXTI_Line = EXTI_Line4;										//�ⲿ�ж���4
	EXTI_InitStrue.EXTI_Mode = EXTI_Mode_Interrupt;						//�ж�ģʽ
	EXTI_InitStrue.EXTI_Trigger = EXTI_Trigger_Rising;				//�����ش���(��Ϊ����ʱ��ü�������)
	EXTI_InitStrue.EXTI_LineCmd = ENABLE;											//ʹ���ж���
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
	
	//�������ж����ȼ���ʼ��������
	NVIC_InitStrue.NVIC_IRQChannel = EXTI4_IRQn;					//�ж�ͨ��
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 2;	//��ռ���ȼ�
	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 0;				//��Ӧ���ȼ�
	NVIC_InitStrue.NVIC_IRQChannelCmd = ENABLE;						//ʹ���ж����ȼ�
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
	EXTI_ClearITPendingBit(EXTI_Line4);//��������־λ
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


