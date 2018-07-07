#include "delay.h"
#include "led.h"
static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在ucos下,代表每个节拍的ms数

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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);						//开启定时器时钟
	
	TIM_TimeBaseInitStructure.TIM_Period = arr-1;							//设置重装寄存器装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc-1;						//设置预分频系数
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;         //计数模式
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);						//初始化
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);								//清除更新中断标志位
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);								//设置中断为更新中断
	
    TIM2_NVICInit();
    
    TIM_Cmd(TIM2,ENABLE);																					 //开启定时器
    
}
extern unsigned int system_tick_num;
void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update))			//判断中断标志位
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //清除相应中断标志位
        system_tick_num += 20;
		LED1 = !LED1;
	}
}

//初始化延迟函数
//当使用OS的时候,此函数会初始化OS的时钟节拍
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8=9M VAL每减一次的时间是1/9us
	fac_us=SystemCoreClock/8000000;				//为系统时钟的1/8M=9     所以VAL从9减到0就是1us了 

	fac_ms=(u16)fac_us*1000;					//非OS下,代表每个ms需要的systick时钟数   

}								    


//延时nus
//nus为要延时的us数.	us的值不能超过nus<=( 2^24) /fac_us	    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//时间加载	  		 
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//判断在使能的情况下等待时间是否到达
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;      					 //清空计数器	 
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//判断在使能的情况下等待时间是否到达  
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器	  	    
} 

