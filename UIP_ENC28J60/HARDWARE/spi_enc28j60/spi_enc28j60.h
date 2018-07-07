#ifndef  __spi_enc28j60_H
#define  __spi_enc28j60_H
#include "sys.h"

#define SPI_MOSI    GPIO_Pin_7
#define SPI_MISO    GPIO_Pin_6
#define SPI_SCK     GPIO_Pin_5

#define 	ENC28J60_CS	 		GPIO_Pin_4								/* ENC28J60Ƭѡ�� */
#define 	ENC28J60_CSL()		GPIOA->BRR = ENC28J60_CS;				/* ����Ƭѡ ��Ч */
#define 	ENC28J60_CSH()		GPIOA->BSRR = ENC28J60_CS;				/* ����Ƭѡ ʧЧ*/
//#define  ENC28J60_CSL()    (GPIOA->ODR &= ~(1<<4))
//#define  ENC28J60_CSH()    (GPIOA->ODR |= 1<<4)

void Enc28j60_Spi_Init(void);
uint8_t SPI1_ReadWrite(uint8_t data);


# endif


