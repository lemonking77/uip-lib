#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������:��ʼ��LED	
//ע��:�ڲ���IO��֮ǰһ��Ҫʹ������ʱ��
////////////////////////////////////////////////////////////////////////////////// 	   

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
	//����һ����ʼ���ṹ��
	GPIO_InitTypeDef GPIO_InitStructure;
	//ʹ��GPIOB������ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	//ʹ��GPIOE������ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	//ѡ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	//��������ģʽΪ�������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//�������ŵ�����ٶ�
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//��ʼ���˿�
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_5);//�ر�С��
	GPIO_SetBits(GPIOE,GPIO_Pin_5);//�ر�С��
}
 
