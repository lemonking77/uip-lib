#include "enc28j60.h"
#include "spi_enc28j60.h"
#include "usart.h"
#include "led.h"
/*
 * ����ENC28j60�Ĳ�����Ҫ��2+2+2����д���ƼĴ���+��дPHY�Ĵ���+��д��������
 * enc28j60ReadOp(u8 op, u8 address)��enc28j60WriteOp(u8 op, u8 address, u8 data) ��д���ƼĴ���
 * �����������ǶԼĴ�����ֱ�Ӳ����������޷����üĴ����Ŀ�ţ�
 
 * enc28j60Write(u8 address, u8 data)��enc28j60Read(u8 address)�����������������ϵķ�װ���ڲ�����
 * ����֮ǰ�����˼Ĵ����Ŀ����ã�
 
 * enc28j60PhyRead(u8 address)��enc28j60PhyWrite(u8 address, u16 data)�������Ƕ�PHY��Ĵ����Ķ�д��
 * �Ƚϲ������̱Ƚ����⣬��ͨ���н�Ĵ������еģ�
 
 * enc28j60WriteBuffer(u16 len, u8* data)��enc28j60ReadBuffer(u16 len, u8* data)�������ǶԻ�������
 * ��д���������Ķ�дָ�붼��Ӳ���Զ��ƶ��ģ����Կ��������Ķ�д��

 * enc28j60PacketReceive(u16 maxlen, u8* packet)��enc28j60PacketSend(u16 len, u8* packet)������������
 * �ڶ�д�����������ϵķ�װ�����ڽ��պͷ������ݰ���
   enc28j60PacketReceive()�������ݰ�������Ҫ���ж��Ƿ���յ����ݰ���Ȼ��д���ָ��λ�ã���ȡ��һ֡
   ���ݵĿ�ʼλ�ã����н���״̬�жϺ����ʹ�ö��������������������ݣ���������ý��ն�ָ���λ�ã�
   Ϊ����Ӳ��д��FIFO�ڳ�λ�ã����ѽ��յ������ݰ�������1��
   enc28j60PacketSend()�������ݰ���������Ҫд��дָ��λ�ã�Ȼ��ѷ��ͽ�����ַƫ��Ϊ��ʼ��ַ+���ݳ��ȣ�
   ��ʱ��Ҫд��һ���ֽڵ����ݰ������ֽڣ�Ȼ��������д��������������ָ�����ȵ�����д�룬д�����ݾ�ʹ��
   ���ͣ�����鷢��״̬������ͱ�־��
 
 * * ע��  ������������ݰ��Ĺ�����ҪŪ����ĸ�ָ������ã�
           �� ��ָ��(ERDPT)�����û�������ָ���Ľ��ջ����н��������õ�,Ĭ��Ӳ���Զ���λ
           �� дָ��(EWRPT)�����û�������ָ���ķ��ͻ�����д�������õģ�Ĭ����Ӳ���Զ���λ
           �� ���ն�ָ��(ERXRDPT)�����û�����ָ�����λ��֮����ڴ�Ӳ����������д����(���������ݲ�������)��λ���ƶ����û�����
           �� ����дָ��(ERXWRPT)������ָ���ײ�Ӳ����FIFO���ջ�����д�����ݵ�λ�ã���Ӳ���Զ���λ��һ�����ܳ������ն�ָ��

*/


static unsigned char Enc28j60Bank;  //���ڱ��浱ǰ��λ���ĸ������
static unsigned int NextPacketPtr;  //��һ�����ݵ�λ��ָ��


///*
// * ��������ENC28J60���ж�������Ϊ�ⲿ�ж�
// * ����  ��ENC���ж����������ߵ�ƽ
// * ����  ��
// * ���  ��
// * ����  ��
// */
//void Enc28j60_INTPin_Init()
//{
//    GPIO_InitTypeDef GPIO_InitStructure;//ENC28J60_INT
//    EXTI_InitTypeDef EXTI_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);//ʹ��AFIOʱ��(�ڶ˿���ӳ�䡢�ⲿ�ж����á��¼��Ĵ�������ǰ��������AFIO)
//	
//    GPIO_InitStructure.GPIO_Mode = ENC28J60_INT_MODE;
//    GPIO_InitStructure.GPIO_Pin = ENC28J60_INT_PIN;
//    GPIO_Init(GPIO_ENC28J60_INT,&GPIO_InitStructure);
//    
//    
//    //�������ⲿ�жϺ��ж���ӳ�������
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource3);  //IO�����ж���ӳ�亯��
//	EXTI_InitStructure.EXTI_Line = EXTI_Line3;				    //�ⲿ�ж���3
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//�ж�ģʽ
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//ʹ���ж���
//	EXTI_Init(&EXTI_InitStructure);
//	
//	//�������ж����ȼ���ʼ��������
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			//�ж�ͨ��
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//��ռ���ȼ�
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//��Ӧ���ȼ�
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ���ж����ȼ�
//	NVIC_Init(&NVIC_InitStructure);
//}

//void EXTI3_IRQHandler()
//{
//    LED0 = !LED0;
//    printf("enter EXTI3_IRQ\r\n");
//    EXTI_ClearITPendingBit(EXTI_Line3);//��������־λ
//}

/*
 * ��������enc28j60ReadOp
 * ����  �������ƼĴ��� (���ǲ����Ĵ�����ײ�ĺ���)
 * ����  ��op ����ָ�address �Ĵ�����ַ
 * ���  ���Ĵ�������
 * ����  ���� д������Ͳ������� ����OXFF��ȡ�Ĵ�������(ע���Ƿ���(MAC/MII)��Ĵ����������Ҫ�෢һ��α�ֽ�)
 */
u8 enc28j60ReadOp(u8 op, u8 address)
{
    uint8_t dat = 0;
    ENC28J60_CSL();
    dat = op|(address&ADDR_MASK);
    SPI1_ReadWrite(dat);    //���Ͳ�����ͼĴ����ĵ�ַ
    
    // ��������(MAC/MII)��Ĵ�������Ҫ�෢��һ��α�ֽ� (�����ENC28J60�����ֲ� P29��)
    if(0x80==(address&SPRD_MASK))
    {
        SPI1_ReadWrite(0xff);
    }
    dat = SPI1_ReadWrite(0xff);//��ȡ�Ĵ�������
    ENC28J60_CSH();
    return dat;
}

/*
 * ��������enc28j60WriteOp
 * ����  ������̫��������д�����������(���ǲ����Ĵ�����ײ�ĺ���)
 * ����  ��op SPI�Ŀ������address �Ĵ��������ϵ�ַ��data д��Ĵ���������
 * ���  ����	
 * ����  ���� д������ͼĴ�����ַ���� д����(����0�ֽڣ����ܶ��ֽ�)
 */
void enc28j60WriteOp( u8 op, u8 address, u8 data )
{
    uint8_t dat = 0;
    
    ENC28J60_CSL();
    dat = op|( address&ADDR_MASK );   //��ϲ��������5λ�Ĵ�����ַ��һ���ֽ�
    SPI1_ReadWrite( dat );    //д������ͼĴ�����ַ
    dat = data;
    SPI1_ReadWrite( dat );    //д������
    ENC28J60_CSH();
}

/*
 * ��������enc28j60SetBank
 * ����  ��ѡ����Ҫ������bank
 * ����  ��address ���ϼĴ����ĵ�ַ,ֻ��������ȡ�Ĵ�����ŵ�
 * ���  ����	
 * ���  ����enc28j60�����ֲᡷP29 λ������ ��λ����1 ���
             �������1��ֻ������ETH����ƼĴ�������� 8 ��λ�Ĳ�����
 * ע��  ��Enc28j60BankΪȫ�ֱ��������ڱ��浱ǰ��BANK��ţ�������β������ƼĴ�����ͬһ��BANKʱ��
           �ñ������ֲ��䣬�����β����Ŀ��ƼĴ���λ�ڲ�ͬ��BANK����ôBANK��ֵ���Ϊ�µ�BANK��š�
* ����   ���� ��տ��λ  �����ÿ��λ  ����������ͨ������ʵ��
 */
void enc28j60SetBank(u8 address)
{
    //�����һ�����ڵĿ���Ƿ��뱾����ͬ�������ͬ�Ͳ���������
    if(Enc28j60Bank!=(address&BANK_MASK))
    {
        //�����ECON1�Ĵ�����BSEL0:BSEL1
        enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR,ECON1,ECON1_BSEL0|ECON1_BSEL1);
        
        //��������address���ϼĴ�����ַ�����Ŀ��
        enc28j60WriteOp(ENC28J60_BIT_FIELD_SET,ECON1,(address&BANK_MASK)>>5);
        Enc28j60Bank=address&BANK_MASK;
    }
}
/*
 * ��������enc28j60Read
 * ����  ����ȡ���ƼĴ���������
 * ����  �����ƼĴ����ĵ�ַ
 * ���  ����ȡ�ļĴ�������	
 * ����  ����ѡ���Ĵ�����BANK���  ��ʹ�ö������ȡ��ص�ַ�ļĴ���
 */
u8 enc28j60Read(u8 address)
{
    enc28j60SetBank(address);  
    return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}

/*
 * ��������enc28j60Write
 * ����  ��д�����ݵ����ƼĴ���
 * ����  ��address ���ϼĴ�����ַ��data ����
 * ���  ����	
* ����  ���� ѡ���Ĵ�����BANK���  �� ʹ��д��������ַ�ļĴ�����ַ��д����
 */
void enc28j60Write(u8 address, u8 data)
{
    enc28j60SetBank(address);
    enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}


/*
 * ��������enc28j60PhyWrite
 * ����  ����PHY��Ĵ���д����Ʋ���
 * ����  ��address ���ϼĴ�����ַ��data д�������
 * ���  ����	
 * ����  ����ϸ��enc28j60�����ֲᡷP21 дPHY�Ĵ���
 * ע��  ����PHY��Ĵ�����д�����������͵ļĴ�����ͬ��Ҫ�ȰѼĴ�����ַд�뵽MIREGADR�Ĵ����У�
           Ȼ���������MIWRL�Ĵ���д����ֽڣ�����MIWRH�Ĵ���д����ֽڣ�д����еȴ�д����ɡ�
 */
void enc28j60PhyWrite(u8 address, u32 data)
{
    enc28j60Write(MIREGADR,address&ADDR_MASK);  //��Ҫд��� PHY �Ĵ����ĵ�ַд�� MIREGADR�Ĵ���
    
    enc28j60Write(MIWRL,data&0xff);             //�����ݵĵ� 8 λд�� MIWRL �Ĵ�����������д��λ
    enc28j60Write(MIWRH,(data>>8));
    while (enc28j60Read(MISTAT) & MISTAT_BUSY); //д���λ��Ӳ��MISTAT.BUSY λ�� 1��д����ɺ��Զ���0.
}
/*
 * ��������enc28j60PhyRead
 * ����  ����PHY��Ĵ�����ȡ����
 * ����  ��address ���ϼĴ�����ַ
 * ���  ����ȡ��PHY��Ĵ���������
 * ����  ����ϸ��enc28j60�����ֲᡷP21 ��PHY�Ĵ���
 * ע��  ����PHY��Ĵ����Ķ�ȡ���������͵ļĴ�����ͬ��Ҫ�ȰѼĴ�����ַд�뵽MIREGADR�Ĵ����У�
           Ȼ��������ȡ��Ӳ��������ݴ�PHY��Ĵ�������MIRDH|MIRDL�У��ȵ���ȡ��ɣ�ֹͣ����
           ����û����ֶ��İ����ݴ�MIRDH|MIRDL��ȡ������
 */
u16 enc28j60PhyRead(u8 address)
{
    u16 PHYdat = 0;
    enc28j60Write(MIREGADR,address&ADDR_MASK);  //��Ҫ��ȡ�� PHY �Ĵ����ĵ�ַд�� MIREGADR�Ĵ���
    
    enc28j60Write(MICMD,MICMD_MIIRD);           //MICMD.MIIRD �� 1 ��ʼ������
    while (enc28j60Read(MISTAT) & MISTAT_BUSY); //Ӳ��MISTAT.BUSY λ�� 1����ȡ��ɺ��Զ���0.
    enc28j60Write(MICMD,0);                     //MICMD.MIIRD �� 0 ����������
    
    PHYdat = (uint16_t)enc28j60Read(MIRDH);     //�����ݴ�MIRD�Ĵ����ж�ȡ����
    PHYdat = (PHYdat<<8)|enc28j60Read(MIRDL);
    return PHYdat;
}

/*
 * ��������enc28j60clkout
 * ����  ������CLKOUT�������ʱ��
 * ����  ��ʱ�Ӳ���
 * ���  ����	
101 = CLKOUT �����ʱ�ӵ� 8 ��Ƶ ��3.125 MHz��
100 = CLKOUT �����ʱ�ӵ� 4 ��Ƶ ��6.25 MHz��
011 = CLKOUT �����ʱ�ӵ� 3 ��Ƶ ��8.333333 MHz��
010 = CLKOUT �����ʱ�ӵ� 2 ��Ƶ ��12.5 MHz��
001 = CLKOUT �����ʱ�ӵ� 1 ��Ƶ ��25 MHz��
 */
void enc28j60clkout(u8 clk)
{
    enc28j60Write(ECOCON,clk&0x07);
}
// ��ȡоƬ�汾ID:
u8 enc28j60getrev(void)
{
    return enc28j60Read(EREVID);
}

/*
 * ��������enc28j60ReadBuffer
 * ����  ����Ӳ����FIFO��������ȡ����
 * ����  ��len ���ݳ��ȣ�data�������ݴ�ŵĵط�
 * ���  ����
 */
void enc28j60ReadBuffer(u32 len, u8* data)
{
    ENC28J60_CSL();
    SPI1_ReadWrite(ENC28J60_READ_BUF_MEM);//��ģ��д�����������������ָ��,֮����д�����ݾͻ��ȡ������
    while(len--)        //��ȡ�������е����ݣ�����ָ����Զ�����λ
    {
        *data++ = SPI1_ReadWrite(0);
    }
    //*data = '\0';
    ENC28J60_CSH();
}
/*
 * ��������enc28j60WriteBuffer
 * ����  ����Ӳ����FIFO������д������
 * ����  ��len ���ݳ��ȣ�data��д�������ָ��
 * ���  ����	
 */
void enc28j60WriteBuffer(u32 len, u8* data)
{
    ENC28J60_CSL();
    SPI1_ReadWrite(ENC28J60_WRITE_BUF_MEM);//��ģ��д�����д����������ָ��
    while(len--)                //д�������е����ݣ�����ָ����Զ�����λ
    {
        SPI1_ReadWrite(*data++);
    }
    ENC28J60_CSH();
}

/*
 * ��������enc28j60PacketSend
 * ����  ������һ�����ݰ�
 * ����  ��len ���ݰ����ȣ�packet ָ��Ҫ���͵����ݻ���λ��
 * ���  ����	
 * ����  ��1�����÷��ͻ���Ŀ�ʼ�ͽ�����ַ��
           2��д��һ���ֽڵĿ����֣�
           3����FIFO���ͻ�����д�����ݣ�
           4�������ͣ�
           5����鷢�ʹ���
 * ���  ����enc28j60�����ֲᡷP19 P41
 * ע��  ��1��дָ��(EWRPT)�����û�������ָ���ķ��ͻ�����д�������õģ�Ĭ����Ӳ���Զ���λ��
           2�������MAC֡ǰ��һ���ֽڵĿ����ֽڣ�

�ֽ�ƫ�ƣ�      0        1~N               N~N+8
���ݰ���ʽ��  ����   MAC��������֡    ״̬����(Ӳ��д��)  
 */
void enc28j60PacketSend(u32 len, u8* packet)
{
    while((enc28j60Read(ECON1) & ECON1_TXRTS)!=0); // ��鷢���������ִ�У����;���Ҫ�ȴ�
    // ����дָ����ʼ��ַ
   enc28j60Write(EWRPTL, TXSTART_INIT & 0xFF);
   enc28j60Write(EWRPTH, TXSTART_INIT >> 8);

   // ���÷��ͻ������Ľ�����ַΪ��ʼ��ַ�������ݳ���
   enc28j60Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
   enc28j60Write(ETXNDH, (TXSTART_INIT + len) >> 8);

   // ��ÿ�����ݰ��İ������ֽ�Ԥ��һ����Ԫ�洢���ݰ����������������λ0
   enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

   // �������ݵ�enc28j60���ڲ�����
   enc28j60WriteBuffer(len, packet);

    // ʹ�ܷ������󣬰����ݴ�buff���͵�network
   enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

    // ����������֮��Ҫ��鷢�ʹ����ж��Ƿ��д�����(��ʹ��ֹ������жϣ�Ҳ���Բ�ѯ�����������)
   if ((enc28j60Read(EIR) & EIR_TXERIF))
   {
       enc28j60SetBank(ECON1);  
       enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);//���������λ
   }
}


/*
 * ��������enc28j60PacketReceive
 * ����  ������һ�����ݰ�
 * ����  ��maxlen ���ݰ������ɽ��ճ��ȣ�packet ָ��������ݵĴ洢λ��
 * ���  �����յ������ݰ�����(������4���ֽڵ�CRC)	
 * ����  ��1��������޽��յ����ݣ����û�оͷ��أ�
           2��д���ָ��λ��(��ʼ��λ���ǽ��ջ��������׵�ַ��֮��������ݰ��е���һ�����ݿ�ʼλ���趨)
           3����ȡ���ݰ���MAC֡���Ⱥ�״̬�ֶΣ��ѽ��ճ���-4������CRC���յ�RAM�ڴ棬��ʡ�ڴ棻
           4���ӽ��ջ����ж�ȡ���ݵ��û��ڴ棻
           5���ѽ��ն�ָ��λ���ƶ�����һ�����ݰ����׵�ַ����Ϊ֮ǰ�����ݰ������գ���������ڴ��Ӳ��д�������ݣ�
           6�������ݰ�������1��

 * ���  ����enc28j60�����ֲᡷP19 P45
 * ע��  ������������ݰ��Ĺ�����ҪŪ�������ָ������ã�
           �� ��ָ��(ERDPT)�����û�������ָ���Ľ��ջ����н��������õ�,Ĭ��Ӳ���Զ���λ
           �� ���ն�ָ��(ERXRDPT)�����û�����ָ�����λ��֮����ڴ�Ӳ����������д����(���������ݲ�������)��λ���ƶ����û�����
           �� ����дָ��(ERXWRPT)������ָ���ײ�Ӳ����FIFO���ջ�����д�����ݵ�λ�ã���Ӳ���Զ���λ��һ�����ܳ������ն�ָ��

�ֽ�ƫ�ƣ�          0~2                  4~5             6~N
���ݰ���ʽ�� ��һ�����ݿ�ʼָ��   MAC֡����/״̬λ    MAC��������֡   

*/
u32 enc28j60PacketReceive(u32 maxlen, u8* packet)
{
    u16 rxstat;
    u16 len=0;

    // �����ս������ݰ��������жϱ�־λ�Ƿ���λ���Ҽ����յ��������ݰ��ĸ����Ƿ�Ϊ0,Ȼ���˳���ѯģʽ
    if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
    // The above does not work. See Rev. B4 Silicon Errata point 6.
        if (enc28j60Read(EPKTCNT) == 0)//��̫�����ݰ���������
        {
            return(0);
        }
    }
    // ���ö�ָ�뿪ʼ��λ��(Ӳ���Զ��ƶ���ע������ERXRDPTL(���ն�ָ��)�����ָ������������
    // �����ݲ�����ˢ�ģ���Ӳ����FIFO��д������λ�õ�һ�����ޣ���Ҫ�ֶ��ƶ�)
    enc28j60Write(ERDPTL, (NextPacketPtr));
    enc28j60Write(ERDPTH, (NextPacketPtr) >> 8);

    /* ������ͨ�������ȡenc28j60�Ļ��������ݣ�ÿ�ζ�ȡ�Զ�������ָ��(��ָ�����ʹ��λAUTOINCĬ����ʹ�ܵ�) */
   
    // �����ݰ��л�ȡ��һ�����ݰ���enc28j60�л����е�λ��(���յ������ݰ�ǰ�����ֽڴ������һ�����ݵ�ָ���ַ)
    NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
    NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    // ��ȡMAC֡�����ݳ���(����״̬λ��ǰ16���ֽ���)
    len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
    len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    len -= 4; //���ȼ�ȥCRCУ���ֽ�(������CRC������RAM��Ϊ��ʣ�ڴ�)
    // ��ȡ���ݰ���״̬�ֶ�(״̬λ�ĺ�16���ֽ�)
    rxstat = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
    rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
    // limit retrieve length
    if (len > maxlen - 1)
    {
        len = maxlen - 1;
    }

    // check CRC and symbol errors (see datasheet page 44, table 7-3):
    // The ERXFCON.CRCEN is set by default. Normally we should notneed to check this.
    if ((rxstat & 0x80) == 0)//�������Ƿ�ɹ�
    {
        len = 0;
    }
    else //Ӳ�����ճɹ���������յ������������MAC����֡������ȥ����CRCУ�鲿��
    {
        enc28j60ReadBuffer(len, packet);
    }
    // �ֶ��ƶ� ���ն�ָ���λ�õ���һ�����ݰ�����ʼλ�ã���Ϊ��һ�����ݰ����Ƕ�ȡ�ˣ�Ӳ������������д����������
    enc28j60Write(ERXRDPTL, (NextPacketPtr));
    enc28j60Write(ERXRDPTH, (NextPacketPtr) >> 8);

    // ������һ�����ݰ����PKTDEC(���ݰ��ݼ�λ)�ݼ�һ��
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    return(len);
}

/*
 * ��������enc28j60Init
 * ����  ����ʼ����̫��������
 * ����  ����
 * ���  ����	
 * ����  �������enc28j60�����ֲᡷP35
 */
u8 enc28j60Init(u8* macaddr)
{
//    ENC28J60_RSTH();
    ENC28J60_CSH();
    // �����λ
    enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    // ��ѯESTAT.CLKRDYλ�鿴��λ��ʱ���Ƿ��λ
    while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));

    // do bank 0 stuff
    // ���ý��ջ�������ʼ��ַ �ñ�������ÿ�ζ�ȡ������ʱ������һ�������׵�ַ
    NextPacketPtr = RXSTART_INIT;
    // ���û������н��ջ���� ��ʼ��ַ RX start
    enc28j60Write(ERXSTL, RXSTART_INIT & 0xFF);
    enc28j60Write(ERXSTH, RXSTART_INIT >> 8);
    // ���ý��ն�ָ���ַ
    enc28j60Write(ERXRDPTL, RXSTART_INIT & 0xFF);
    enc28j60Write(ERXRDPTH, RXSTART_INIT >> 8);
    // ���û������н��ջ���� ������ַ RX end
    enc28j60Write(ERXNDL, RXSTOP_INIT & 0xFF);
    enc28j60Write(ERXNDH, RXSTOP_INIT >> 8);
    // ���û������з��ͻ���� ��ʼ��ַ TX start
    enc28j60Write(ETXSTL, TXSTART_INIT & 0xFF);
    enc28j60Write(ETXSTH, TXSTART_INIT >> 8);
    // ���û������з��ͻ���� ������ַ TX end
    enc28j60Write(ETXNDL, TXSTOP_INIT & 0xFF);
    enc28j60Write(ETXNDH, TXSTOP_INIT >> 8);

    // do bank 1 stuff, packet filter:
    // For broadcast packets we allow only ARP packtets ���ڹ㲥���ݰ�������ֻ����ARP���ݰ�
    // All other packets should be unicast only for our mac (MAADR) �����������ݰ�Ӧ�ý������ǵ�mac��MAADR�����е���

    // The pattern to match on is therefore
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
    // in binary these poitions are:11 0000 0011 1111
    // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
    enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);//����������ʽ��������û����
    enc28j60Write(EPMM0, 0x3f);
    enc28j60Write(EPMM1, 0x30);
    enc28j60Write(EPMCSL, 0xf9);
    enc28j60Write(EPMCSH, 0xf7);

    // do bank 2 stuff
    // ʹ��MAC���� ʹ��ȫ˫����������
    enc28j60Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    // ʹ MAC �˳���λ״̬
    enc28j60Write(MACON2, 0x00);
    // Ӧʹ���Զ���䣨�ﵽ���� 60 �ֽڣ����Զ�׷����Ч�� CRCУ��
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX );//MACON3_HFRMLEN
    // set inter-frame gap (non-back-to-back)
    enc28j60Write(MAIPGL, 0x12);
    enc28j60Write(MAIPGH, 0x0C);
    // ���ñ��Ա����ݱ����ʱ��9.6us
    enc28j60Write(MABBIPG, 0x15);
    // �������֡����Ϊ1500(��̫��֡��С)
    enc28j60Write(MAMXFLL, MAX_FRAMELEN & 0xFF); 
    enc28j60Write(MAMXFLH, MAX_FRAMELEN >> 8);

    // do bank 3 stuff
    // д MAC ��ַ
    // ��Enc8J60�е�MAC��ַ���ֽ����ġ�
    enc28j60Write(MAADR5, macaddr[0]);  
    enc28j60Write(MAADR4, macaddr[1]);
    enc28j60Write(MAADR3, macaddr[2]);
    enc28j60Write(MAADR2, macaddr[3]);
    enc28j60Write(MAADR1, macaddr[4]);
    enc28j60Write(MAADR0, macaddr[5]);


    //����PHYΪȫ˫��  LEDBΪ������
    enc28j60PhyWrite(PHCON1, PHCON1_PDPXMD);//MACON3.FULDPX=1�� PHCON1.PDPXMD=1ͬʱΪ1ʱ����ȫ˫��ģʽ

    // ��ֹ��������loopback
    enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);

    // �������������л��� bank 0ȥ����
    enc28j60SetBank(ECON1);
    // ʹ��ȫ�� INT �жϺͽ������ݰ��������ж��Ѿ����մ����ж�
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE|EIE_RXERIE );
    // ���ݰ�����ʹ��
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    /* PHY LED ����,LED����ָʾͨ�ŵ�״̬ */	
    enc28j60PhyWrite(PHLCON,0x476);	

    if(enc28j60Read(MAADR5)== macaddr[0])return 0;//��ʼ���ɹ�
    else return 1;
}





