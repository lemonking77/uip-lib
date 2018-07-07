#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//程序功能:初始化LED	
//注意:在操作IO口之前一定要使能外设时钟
////////////////////////////////////////////////////////////////////////////////// 	   

//初始化PE2、PE3、PE4、PA0为输入.并使能这些IO口的时钟
void KEY_Init()
{
	//定义初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	//开GPIOE/GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOA,ENABLE);
	//选定引脚2、3、4
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4);
	//设置为输入上拉模式
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	//初始化GPIOE
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//选定引脚0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	//设置输入下拉模式
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
	//初始化GPIOA
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}

/*按键扫描程序,支持单次按键 mode=0，连续按键 mode=1*/
//返回0，没有任何按键按下
//返回1，KEY0按下
//返回2，KEY1按下
//返回3，KEY2按下 
//返回4，KEY3按下
u8 KEY_Scan(u8 mode)
{
	static u8 key_upflag  = 1;		//按键按下标志位
	if(mode)											//mode=1支持按下后持续触发，mode=0单次触发模式
		key_upflag = 1;
	if(key_upflag&&(KEY0==0||KEY1==0||KEY2==0||KEY3==1))//判断是否有其中一个按键按下
	{	
		delay_ms(5);																			//延时消抖
		key_upflag = 0;
		if(KEY0==0) return KEY0_EFF;
		else if(KEY1==0) return KEY1_EFF;
		else if(KEY2==0) return KEY2_EFF;
		else if(KEY3==1) return KEY3_EFF;
	}
	else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==0)//如果没有按键按下
		key_upflag = 1;
	return 0;
}




