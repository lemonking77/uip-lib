#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������:��ʼ��LED	
//ע��:�ڲ���IO��֮ǰһ��Ҫʹ������ʱ��
////////////////////////////////////////////////////////////////////////////////// 	   

//��ʼ��PE2��PE3��PE4��PA0Ϊ����.��ʹ����ЩIO�ڵ�ʱ��
void KEY_Init()
{
	//�����ʼ���ṹ��
	GPIO_InitTypeDef GPIO_InitStructure;
	//��GPIOE/GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOA,ENABLE);
	//ѡ������2��3��4
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4);
	//����Ϊ��������ģʽ
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	//��ʼ��GPIOE
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//ѡ������0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	//������������ģʽ
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
	//��ʼ��GPIOA
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}

/*����ɨ�����,֧�ֵ��ΰ��� mode=0���������� mode=1*/
//����0��û���κΰ�������
//����1��KEY0����
//����2��KEY1����
//����3��KEY2���� 
//����4��KEY3����
u8 KEY_Scan(u8 mode)
{
	static u8 key_upflag  = 1;		//�������±�־λ
	if(mode)											//mode=1֧�ְ��º����������mode=0���δ���ģʽ
		key_upflag = 1;
	if(key_upflag&&(KEY0==0||KEY1==0||KEY2==0||KEY3==1))//�ж��Ƿ�������һ����������
	{	
		delay_ms(5);																			//��ʱ����
		key_upflag = 0;
		if(KEY0==0) return KEY0_EFF;
		else if(KEY1==0) return KEY1_EFF;
		else if(KEY2==0) return KEY2_EFF;
		else if(KEY3==1) return KEY3_EFF;
	}
	else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==0)//���û�а�������
		key_upflag = 1;
	return 0;
}




