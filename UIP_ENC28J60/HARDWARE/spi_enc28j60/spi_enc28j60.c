#include "spi_enc28j60.h"

/*
 * 函数名：SPI1的初始化
 * 描述  ：ENC28j60的空闲时钟是低电平，采样边沿是第一个时钟沿
 * 输入  ：
 * 输出  ：
 * 返回  ：
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

    
    SPI_InitTypeStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//SPI1时钟位APB2=72Mhz,8分频后为9Mhz
    SPI_InitTypeStructure.SPI_CPOL = SPI_CPOL_Low;  //空闲时钟位低
    SPI_InitTypeStructure.SPI_CPHA = SPI_CPHA_1Edge;//采样在第一个边沿采样
    SPI_InitTypeStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitTypeStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitTypeStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitTypeStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitTypeStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitTypeStructure.SPI_CRCPolynomial = 7;
    
    SPI_Init(SPI1,&SPI_InitTypeStructure);
    SPI_Cmd(SPI1, ENABLE); 
    
    GPIO_ResetBits(GPIOA,SPI_SCK);  //拉低时钟
    ENC28J60_CSH();                 //拉高片选无效
}
uint8_t SPI1_ReadWrite(uint8_t data)
{
    //等待上一次发送完成
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)==RESET);
        
    SPI_I2S_SendData(SPI1, (uint8_t)data);
    //等待上一次接收完成
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE )==RESET);
        
    return SPI_I2S_ReceiveData(SPI1);
}



