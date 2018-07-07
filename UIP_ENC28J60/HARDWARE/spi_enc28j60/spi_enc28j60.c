#include "spi_enc28j60.h"

/*
 * ��������SPI1�ĳ�ʼ��
 * ����  ��ENC28j60�Ŀ���ʱ���ǵ͵�ƽ�����������ǵ�һ��ʱ����
 * ����  ��
 * ���  ��
 * ����  ��
 */
void Enc28j60_Spi_Init()
{
    GPIO_InitTypeDef GPIO_InitTypeStructure;
    SPI_InitTypeDef  SPI_InitTypeStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    /*
     * PA5-SPI1-SCK :ENC28J60_SCK
	 * PA6-SPI1-MISO:ENC28J60_SO
	 * PA7-SPI1-MOSI:ENC28J60_SI
     */
    GPIO_InitTypeStructure.GPIO_Pin = SPI_SCK|SPI_MISO|SPI_MOSI;
    GPIO_InitTypeStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitTypeStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA,&GPIO_InitTypeStructure);
    /*
     * PA4-CS-PIN:ENC28J60_CS
     */
    GPIO_InitTypeStructure.GPIO_Pin = ENC28J60_CS;
    GPIO_InitTypeStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitTypeStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA,&GPIO_InitTypeStructure);

    
    SPI_InitTypeStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//SPI1ʱ��λAPB2=72Mhz,8��Ƶ��Ϊ9Mhz
    SPI_InitTypeStructure.SPI_CPOL = SPI_CPOL_Low;  //����ʱ��λ��
    SPI_InitTypeStructure.SPI_CPHA = SPI_CPHA_1Edge;//�����ڵ�һ�����ز���
    SPI_InitTypeStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitTypeStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitTypeStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitTypeStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitTypeStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitTypeStructure.SPI_CRCPolynomial = 7;
    
    SPI_Init(SPI1,&SPI_InitTypeStructure);
    SPI_Cmd(SPI1, ENABLE); 
    
    GPIO_ResetBits(GPIOA,SPI_SCK);  //����ʱ��
    ENC28J60_CSH();                 //����Ƭѡ��Ч
}
uint8_t SPI1_ReadWrite(uint8_t data)
{
    //�ȴ���һ�η������
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)==RESET);
        
    SPI_I2S_SendData(SPI1, (uint8_t)data);
    //�ȴ���һ�ν������
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE )==RESET);
        
    return SPI_I2S_ReceiveData(SPI1);
}



