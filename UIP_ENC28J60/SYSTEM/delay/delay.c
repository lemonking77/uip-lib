#include "delay.h"
#include "led.h"
static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������,��ucos��,����ÿ�����ĵ�ms��

static void TIM2_NVICInit()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

void TIME2_Init(u16 arr,u16 psc)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);						//������ʱ��ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Period = arr-1;							//������װ�Ĵ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc-1;						//����Ԥ��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;         //����ģʽ
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);						//��ʼ��
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);								//��������жϱ�־λ
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);								//�����ж�Ϊ�����ж�
	
    TIM2_NVICInit();
    
    TIM_Cmd(TIM2,ENABLE);																					 //������ʱ��
    
}
extern unsigned int system_tick_num;
void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update))			//�ж��жϱ�־λ
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //�����Ӧ�жϱ�־λ
        system_tick_num += 20;
		LED1 = !LED1;
	}
}

//��ʼ���ӳٺ���
//��ʹ��OS��ʱ��,�˺������ʼ��OS��ʱ�ӽ���
//SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
//SYSCLK:ϵͳʱ��
void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//ѡ���ⲿʱ��  HCLK/8=9M VALÿ��һ�ε�ʱ����1/9us
	fac_us=SystemCoreClock/8000000;				//Ϊϵͳʱ�ӵ�1/8M=9     ����VAL��9����0����1us�� 

	fac_ms=(u16)fac_us*1000;					//��OS��,����ÿ��ms��Ҫ��systickʱ����   

}								    


//��ʱnus
//nusΪҪ��ʱ��us��.	us��ֵ���ܳ���nus<=( 2^24) /fac_us	    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//ʱ�����	  		 
	SysTick->VAL=0x00;        					//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//�ж���ʹ�ܵ�����µȴ�ʱ���Ƿ񵽴�
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;      					 //��ռ�����	 
}
//��ʱnms
//ע��nms�ķ�Χ
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK��λΪHz,nms��λΪms
//��72M������,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL =0x00;							//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//�ж���ʹ�ܵ�����µȴ�ʱ���Ƿ񵽴�  
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;       					//��ռ�����	  	    
} 

