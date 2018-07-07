#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
//程序功能:初始化LED	
//注意:在操作IO口之前一定要使能外设时钟
////////////////////////////////////////////////////////////////////////////////// 	   

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
	//定义一个初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	//使能GPIOB的外设时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	//使能GPIOE的外设时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	//选择引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	//设置引脚模式为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//设置引脚的输出速度
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//初始化端口
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_5);//关闭小灯
	GPIO_SetBits(GPIOE,GPIO_Pin_5);//关闭小灯
}
 
