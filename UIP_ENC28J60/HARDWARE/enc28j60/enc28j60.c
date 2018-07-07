#include "enc28j60.h"
#include "spi_enc28j60.h"
#include "usart.h"
#include "led.h"
/*
 * 对于ENC28j60的操作主要是2+2+2，读写控制寄存器+读写PHY寄存器+读写缓冲区：
 * enc28j60ReadOp(u8 op, u8 address)、enc28j60WriteOp(u8 op, u8 address, u8 data) 读写控制寄存器
 * 这两个函数是对寄存器的直接操作，但是无法设置寄存器的块号；
 
 * enc28j60Write(u8 address, u8 data)、enc28j60Read(u8 address)是在以上两个函数上的封装，在操作寄
 * 存器之前进行了寄存器的块设置；
 
 * enc28j60PhyRead(u8 address)；enc28j60PhyWrite(u8 address, u16 data)这两个是对PHY类寄存器的读写，
 * 比较操作过程比较特殊，是通过中介寄存器进行的；
 
 * enc28j60WriteBuffer(u16 len, u8* data)、enc28j60ReadBuffer(u16 len, u8* data)这两个是对缓存区的
 * 读写，缓存区的读写指针都是硬件自动移动的，所以可以连续的读写；

 * enc28j60PacketReceive(u16 maxlen, u8* packet)、enc28j60PacketSend(u16 len, u8* packet)这两个函数是
 * 在读写缓冲区函数上的封装，用于接收和发送数据包；
   enc28j60PacketReceive()接收数据包函数中要先判断是否接收到数据包，然后写入读指针位置，获取下一帧
   数据的开始位置，进行接收状态判断后才能使用读缓冲区函数，接收数据，最后还有设置接收读指针的位置，
   为后面硬件写入FIFO腾出位置，最后把接收到的数据包数量减1；
   enc28j60PacketSend()发送数据包函数中需要写入写指针位置，然后把发送结束地址偏移为开始地址+数据长度，
   此时先要写入一个字节的数据包控制字节，然后在利用写缓冲区函数进行指定长度的数据写入，写完数据就使能
   发送，最后检查发送状态清除发送标志。
 
 * * 注意  ：这个接收数据包的过程重要弄清楚四个指针的作用：
           ① 读指针(ERDPT)：给用户用来从指定的接收缓冲中接收数据用的,默认硬件自动移位
           ② 写指针(EWRPT)：给用户用来向指定的发送缓冲中写入数据用的，默认由硬件自动移位
           ③ 接收读指针(ERXRDPT)：给用户用来指定这个位置之后的内存硬件不能向其写数据(保护旧数据不被覆盖)，位置移动由用户操作
           ④ 接收写指针(ERXWRPT)：用来指定底层硬件向FIFO接收缓冲中写入数据的位置，由硬件自动移位，一定不能超过接收读指针

*/


static unsigned char Enc28j60Bank;  //用于保存当前所位于哪个块号上
static unsigned int NextPacketPtr;  //下一包数据的位置指针


///*
// * 函数名：ENC28J60的中断引脚设为外部中断
// * 描述  ：ENC的中断情况下输出高电平
// * 输入  ：
// * 输出  ：
// * 返回  ：
// */
//void Enc28j60_INTPin_Init()
//{
//    GPIO_InitTypeDef GPIO_InitStructure;//ENC28J60_INT
//    EXTI_InitTypeDef EXTI_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);//使能AFIO时钟(在端口重映射、外部中断配置、事件寄存器操作前必须配置AFIO)
//	
//    GPIO_InitStructure.GPIO_Mode = ENC28J60_INT_MODE;
//    GPIO_InitStructure.GPIO_Pin = ENC28J60_INT_PIN;
//    GPIO_Init(GPIO_ENC28J60_INT,&GPIO_InitStructure);
//    
//    
//    //以下是外部中断和中断线映射的配置
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource3);  //IO口与中断线映射函数
//	EXTI_InitStructure.EXTI_Line = EXTI_Line3;				    //外部中断线3
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//中断模式
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//使能中断线
//	EXTI_Init(&EXTI_InitStructure);
//	
//	//以下是中断优先级初始化的配置
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			//中断通道
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//响应优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//使能中断优先级
//	NVIC_Init(&NVIC_InitStructure);
//}

//void EXTI3_IRQHandler()
//{
//    LED0 = !LED0;
//    printf("enter EXTI3_IRQ\r\n");
//    EXTI_ClearITPendingBit(EXTI_Line3);//清除挂起标志位
//}

/*
 * 函数名：enc28j60ReadOp
 * 描述  ：读控制寄存器 (这是操作寄存器最底层的函数)
 * 输入  ：op 操作指令；address 寄存器地址
 * 输出  ：寄存器内容
 * 步骤  ：① 写操作码和参数；② 发送OXFF读取寄存器数据(注意是否是(MAC/MII)类寄存器，如果是要多发一个伪字节)
 */
u8 enc28j60ReadOp(u8 op, u8 address)
{
    uint8_t dat = 0;
    ENC28J60_CSL();
    dat = op|(address&ADDR_MASK);
    SPI1_ReadWrite(dat);    //发送操作码和寄存器的地址
    
    // 操作的是(MAC/MII)类寄存器，需要多发送一个伪字节 (详见《ENC28J60数据手册 P29》)
    if(0x80==(address&SPRD_MASK))
    {
        SPI1_ReadWrite(0xff);
    }
    dat = SPI1_ReadWrite(0xff);//读取寄存器数据
    ENC28J60_CSH();
    return dat;
}

/*
 * 函数名：enc28j60WriteOp
 * 描述  ：向以太网控制器写入命令和数据(这是操作寄存器最底层的函数)
 * 输入  ：op SPI的控制命令；address 寄存器的联合地址；data 写入寄存器的数据
 * 输出  ：无	
 * 步骤  ：① 写操作码和寄存器地址；② 写数据(可能0字节，可能多字节)
 */
void enc28j60WriteOp( u8 op, u8 address, u8 data )
{
    uint8_t dat = 0;
    
    ENC28J60_CSL();
    dat = op|( address&ADDR_MASK );   //组合操作命令和5位寄存器地址成一个字节
    SPI1_ReadWrite( dat );    //写入命令和寄存器地址
    dat = data;
    SPI1_ReadWrite( dat );    //写入数据
    ENC28J60_CSH();
}

/*
 * 函数名：enc28j60SetBank
 * 描述  ：选定将要操作的bank
 * 输入  ：address 联合寄存器的地址,只是用来提取寄存器块号的
 * 输出  ：无	
 * 详见  ：《enc28j60数据手册》P29 位域清零 和位域置1 命令。
             清零和置1都只能用于ETH类控制寄存器中最多 8 个位的操作；
 * 注意  ：Enc28j60Bank为全局变量，用于保存当前的BANK编号，如果两次操作控制寄存器在同一个BANK时，
           该变量保持不变，若两次操作的控制寄存器位于不同的BANK，那么BANK的值会变为新的BANK编号。
* 步骤   ：① 清空块号位  ②设置块号位  这两步都是通过命令实现
 */
void enc28j60SetBank(u8 address)
{
    //检查上一次所在的块号是否与本次相同，如果相同就不必设置了
    if(Enc28j60Bank!=(address&BANK_MASK))
    {
        //先清除ECON1寄存器的BSEL0:BSEL1
        enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR,ECON1,ECON1_BSEL0|ECON1_BSEL1);
        
        //设置设置address联合寄存器地址所属的块号
        enc28j60WriteOp(ENC28J60_BIT_FIELD_SET,ECON1,(address&BANK_MASK)>>5);
        Enc28j60Bank=address&BANK_MASK;
    }
}
/*
 * 函数名：enc28j60Read
 * 描述  ：读取控制寄存器的内容
 * 输入  ：控制寄存器的地址
 * 输出  ：读取的寄存器内容	
 * 步骤  ：①选定寄存器的BANK编号  ②使用读命令读取相关地址的寄存器
 */
u8 enc28j60Read(u8 address)
{
    enc28j60SetBank(address);  
    return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}

/*
 * 函数名：enc28j60Write
 * 描述  ：写入数据到控制寄存器
 * 输入  ：address 联合寄存器地址；data 数据
 * 输出  ：无	
* 步骤  ：① 选定寄存器的BANK编号  ② 使用写命令读向地址的寄存器地址下写数据
 */
void enc28j60Write(u8 address, u8 data)
{
    enc28j60SetBank(address);
    enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}


/*
 * 函数名：enc28j60PhyWrite
 * 描述  ：向PHY类寄存器写入控制参数
 * 输入  ：address 联合寄存器地址；data 写入的数据
 * 输出  ：无	
 * 步骤  ：详细《enc28j60数据手册》P21 写PHY寄存器
 * 注意  ：对PHY类寄存器的写入与其他类型的寄存器不同，要先把寄存器地址写入到MIREGADR寄存器中，
           然后必须先向MIWRL寄存器写入低字节，在向MIWRH寄存器写入高字节，写完后还有等待写入完成。
 */
void enc28j60PhyWrite(u8 address, u32 data)
{
    enc28j60Write(MIREGADR,address&ADDR_MASK);  //把要写入的 PHY 寄存器的地址写入 MIREGADR寄存器
    
    enc28j60Write(MIWRL,data&0xff);             //将数据的低 8 位写入 MIWRL 寄存器，必须先写低位
    enc28j60Write(MIWRH,(data>>8));
    while (enc28j60Read(MISTAT) & MISTAT_BUSY); //写入高位后，硬件MISTAT.BUSY 位置 1，写入完成后自动清0.
}
/*
 * 函数名：enc28j60PhyRead
 * 描述  ：从PHY类寄存器读取数据
 * 输入  ：address 联合寄存器地址
 * 输出  ：读取的PHY类寄存器的数据
 * 步骤  ：详细《enc28j60数据手册》P21 读PHY寄存器
 * 注意  ：对PHY类寄存器的读取与其他类型的寄存器不同，要先把寄存器地址写入到MIREGADR寄存器中，
           然后启动读取，硬件会把数据从PHY类寄存器读到MIRDH|MIRDL中，等到读取完成，停止读，
           最后用户在手动的把数据从MIRDH|MIRDL读取出来。
 */
u16 enc28j60PhyRead(u8 address)
{
    u16 PHYdat = 0;
    enc28j60Write(MIREGADR,address&ADDR_MASK);  //把要读取的 PHY 寄存器的地址写入 MIREGADR寄存器
    
    enc28j60Write(MICMD,MICMD_MIIRD);           //MICMD.MIIRD 置 1 开始读操作
    while (enc28j60Read(MISTAT) & MISTAT_BUSY); //硬件MISTAT.BUSY 位置 1，读取完成后自动清0.
    enc28j60Write(MICMD,0);                     //MICMD.MIIRD 置 0 结束读操作
    
    PHYdat = (uint16_t)enc28j60Read(MIRDH);     //将数据从MIRD寄存器中读取出来
    PHYdat = (PHYdat<<8)|enc28j60Read(MIRDL);
    return PHYdat;
}

/*
 * 函数名：enc28j60clkout
 * 描述  ：设置CLKOUT引脚输出时钟
 * 输入  ：时钟参数
 * 输出  ：无	
101 = CLKOUT 输出主时钟的 8 分频 （3.125 MHz）
100 = CLKOUT 输出主时钟的 4 分频 （6.25 MHz）
011 = CLKOUT 输出主时钟的 3 分频 （8.333333 MHz）
010 = CLKOUT 输出主时钟的 2 分频 （12.5 MHz）
001 = CLKOUT 输出主时钟的 1 分频 （25 MHz）
 */
void enc28j60clkout(u8 clk)
{
    enc28j60Write(ECOCON,clk&0x07);
}
// 读取芯片版本ID:
u8 enc28j60getrev(void)
{
    return enc28j60Read(EREVID);
}

/*
 * 函数名：enc28j60ReadBuffer
 * 描述  ：从硬件的FIFO接收区读取数据
 * 输入  ：len 数据长度，data接收数据存放的地方
 * 输出  ：无
 */
void enc28j60ReadBuffer(u32 len, u8* data)
{
    ENC28J60_CSL();
    SPI1_ReadWrite(ENC28J60_READ_BUF_MEM);//项模块写入操作读缓冲区操作指令,之后再写入数据就会读取数据了
    while(len--)        //读取缓冲区中的数据，数据指针会自动的移位
    {
        *data++ = SPI1_ReadWrite(0);
    }
    //*data = '\0';
    ENC28J60_CSH();
}
/*
 * 函数名：enc28j60WriteBuffer
 * 描述  ：向硬件的FIFO发送区写入数据
 * 输入  ：len 数据长度，data待写入的数据指针
 * 输出  ：无	
 */
void enc28j60WriteBuffer(u32 len, u8* data)
{
    ENC28J60_CSL();
    SPI1_ReadWrite(ENC28J60_WRITE_BUF_MEM);//项模块写入操作写缓冲区操作指令
    while(len--)                //写缓冲区中的数据，数据指针会自动的移位
    {
        SPI1_ReadWrite(*data++);
    }
    ENC28J60_CSH();
}

/*
 * 函数名：enc28j60PacketSend
 * 描述  ：发送一个数据包
 * 输入  ：len 数据包长度，packet 指向要发送的数据缓存位置
 * 输出  ：无	
 * 步骤  ：1、设置发送缓冲的开始和结束地址；
           2、写入一个字节的控制字；
           3、向FIFO发送缓冲中写入数据；
           4、请求发送；
           5、检查发送错误；
 * 详见  ：《enc28j60数据手册》P19 P41
 * 注意  ：1、写指针(EWRPT)：给用户用来向指定的发送缓冲中写入数据用的，默认由硬件自动移位；
           2、正真的MAC帧前有一个字节的控制字节；

字节偏移：      0        1~N               N~N+8
数据包格式：  控制   MAC完整数据帧    状态向量(硬件写入)  
 */
void enc28j60PacketSend(u32 len, u8* packet)
{
    while((enc28j60Read(ECON1) & ECON1_TXRTS)!=0); // 检查发送如果真正执行，发送就需要等待
    // 设置写指针起始地址
   enc28j60Write(EWRPTL, TXSTART_INIT & 0xFF);
   enc28j60Write(EWRPTH, TXSTART_INIT >> 8);

   // 设置发送缓存区的结束地址为开始地址加上数据长度
   enc28j60Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
   enc28j60Write(ETXNDH, (TXSTART_INIT + len) >> 8);

   // 给每个数据包的包控制字节预留一个单元存储数据包控制字这里控制字位0
   enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

   // 拷贝数据到enc28j60的内部缓存
   enc28j60WriteBuffer(len, packet);

    // 使能发送请求，把数据从buff发送到network
   enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

    // 发送完数据之后要检查发送错误中断是否有错误发送(即使禁止了这个中断，也可以查询发现这个错误)
   if ((enc28j60Read(EIR) & EIR_TXERIF))
   {
       enc28j60SetBank(ECON1);  
       enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);//清除请求发送位
   }
}


/*
 * 函数名：enc28j60PacketReceive
 * 描述  ：接收一个数据包
 * 输入  ：maxlen 数据包的最大可接收长度，packet 指向接收数据的存储位置
 * 输出  ：接收到的数据包长度(不包括4个字节的CRC)	
 * 步骤  ：1、检查有无接收到数据，如果没有就返回；
           2、写入读指针位置(开始的位置是接收缓冲区的首地址，之后根据数据包中的下一包数据开始位置设定)
           3、获取数据包中MAC帧长度和状态字段，把接收长度-4，不把CRC接收到RAM内存，节省内存；
           4、从接收缓冲中读取数据到用户内存；
           5、把接收读指针位置移动到下一个数据包的首地址，因为之前的数据包被接收，空余出来内存给硬件写入新数据；
           6、把数据包数量减1；

 * 详见  ：《enc28j60数据手册》P19 P45
 * 注意  ：这个接收数据包的过程重要弄清楚三个指针的作用：
           ① 读指针(ERDPT)：给用户用来从指定的接收缓冲中接收数据用的,默认硬件自动移位
           ② 接收读指针(ERXRDPT)：给用户用来指定这个位置之后的内存硬件不能向其写数据(保护旧数据不被覆盖)，位置移动由用户操作
           ③ 接收写指针(ERXWRPT)：用来指定底层硬件向FIFO接收缓冲中写入数据的位置，由硬件自动移位，一定不能超过接收读指针

字节偏移：          0~2                  4~5             6~N
数据包格式： 下一包数据开始指针   MAC帧长度/状态位    MAC完整数据帧   

*/
u32 enc28j60PacketReceive(u32 maxlen, u8* packet)
{
    u16 rxstat;
    u16 len=0;

    // 检查接收接收数据包待处理中断标志位是否置位，且检查接收到完整数据包的个数是否为0,然后退出查询模式
    if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
    // The above does not work. See Rev. B4 Silicon Errata point 6.
        if (enc28j60Read(EPKTCNT) == 0)//以太网数据包计数个数
        {
            return(0);
        }
    }
    // 设置读指针开始的位置(硬件自动移动，注意区别ERXRDPTL(接收读指针)，这个指针是用来保护
    // 旧数据不被冲刷的，是硬件向FIFO中写入数据位置的一个界限，需要手动移动)
    enc28j60Write(ERDPTL, (NextPacketPtr));
    enc28j60Write(ERDPTH, (NextPacketPtr) >> 8);

    /* 以下是通过命令读取enc28j60的缓存区内容，每次读取自动递增读指针(读指针递增使能位AUTOINC默认是使能的) */
   
    // 从数据包中获取下一个数据包在enc28j60中缓冲中的位置(接收到的数据包前两个字节存放了下一包数据的指针地址)
    NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
    NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    // 读取MAC帧的数据长度(存在状态位的前16个字节中)
    len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
    len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    len -= 4; //长度减去CRC校验字节(不接收CRC到主控RAM，为了剩内存)
    // 读取数据包中状态字段(状态位的后16个字节)
    rxstat = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
    rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
    // limit retrieve length
    if (len > maxlen - 1)
    {
        len = maxlen - 1;
    }

    // check CRC and symbol errors (see datasheet page 44, table 7-3):
    // The ERXFCON.CRCEN is set by default. Normally we should notneed to check this.
    if ((rxstat & 0x80) == 0)//检查接收是否成功
    {
        len = 0;
    }
    else //硬件接收成功，这里接收的数据是正真的MAC数据帧，但是去除了CRC校验部分
    {
        enc28j60ReadBuffer(len, packet);
    }
    // 手动移动 接收读指针的位置到下一个数据包的起始位置，因为上一个数据包我们读取了，硬件可以向那里写入新数据了
    enc28j60Write(ERXRDPTL, (NextPacketPtr));
    enc28j60Write(ERXRDPTH, (NextPacketPtr) >> 8);

    // 接收完一个数据包后把PKTDEC(数据包递减位)递减一个
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    return(len);
}

/*
 * 函数名：enc28j60Init
 * 描述  ：初始化以太网控制器
 * 输入  ：无
 * 输出  ：无	
 * 步骤  ：详见《enc28j60数据手册》P35
 */
u8 enc28j60Init(u8* macaddr)
{
//    ENC28J60_RSTH();
    ENC28J60_CSH();
    // 软件复位
    enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    // 查询ESTAT.CLKRDY位查看复位后时钟是否就位
    while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));

    // do bank 0 stuff
    // 设置接收缓冲区起始地址 该变量用于每次读取缓冲区时保留下一个包的首地址
    NextPacketPtr = RXSTART_INIT;
    // 设置缓存区中接收缓冲的 开始地址 RX start
    enc28j60Write(ERXSTL, RXSTART_INIT & 0xFF);
    enc28j60Write(ERXSTH, RXSTART_INIT >> 8);
    // 设置接收读指针地址
    enc28j60Write(ERXRDPTL, RXSTART_INIT & 0xFF);
    enc28j60Write(ERXRDPTH, RXSTART_INIT >> 8);
    // 设置缓存区中接收缓冲的 结束地址 RX end
    enc28j60Write(ERXNDL, RXSTOP_INIT & 0xFF);
    enc28j60Write(ERXNDH, RXSTOP_INIT >> 8);
    // 设置缓存区中发送缓冲的 开始地址 TX start
    enc28j60Write(ETXSTL, TXSTART_INIT & 0xFF);
    enc28j60Write(ETXSTH, TXSTART_INIT >> 8);
    // 设置缓存区中发送缓冲的 结束地址 TX end
    enc28j60Write(ETXNDL, TXSTOP_INIT & 0xFF);
    enc28j60Write(ETXNDH, TXSTOP_INIT >> 8);

    // do bank 1 stuff, packet filter:
    // For broadcast packets we allow only ARP packtets 对于广播数据包，我们只允许ARP数据包
    // All other packets should be unicast only for our mac (MAADR) 所有其他数据包应该仅对我们的mac（MAADR）进行单播

    // The pattern to match on is therefore
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
    // in binary these poitions are:11 0000 0011 1111
    // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
    enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);//这里的这个格式过滤设置没看懂
    enc28j60Write(EPMM0, 0x3f);
    enc28j60Write(EPMM1, 0x30);
    enc28j60Write(EPMCSL, 0xf9);
    enc28j60Write(EPMCSH, 0xf7);

    // do bank 2 stuff
    // 使能MAC接收 使能全双工流量控制
    enc28j60Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    // 使 MAC 退出复位状态
    enc28j60Write(MACON2, 0x00);
    // 应使能自动填充（达到至少 60 字节），自动追加有效的 CRC校验
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX );//MACON3_HFRMLEN
    // set inter-frame gap (non-back-to-back)
    enc28j60Write(MAIPGL, 0x12);
    enc28j60Write(MAIPGH, 0x0C);
    // 设置背对背数据报间隔时间9.6us
    enc28j60Write(MABBIPG, 0x15);
    // 设置最大帧长度为1500(以太网帧大小)
    enc28j60Write(MAMXFLL, MAX_FRAMELEN & 0xFF); 
    enc28j60Write(MAMXFLH, MAX_FRAMELEN >> 8);

    // do bank 3 stuff
    // 写 MAC 地址
    // 在Enc8J60中的MAC地址是字节向后的。
    enc28j60Write(MAADR5, macaddr[0]);  
    enc28j60Write(MAADR4, macaddr[1]);
    enc28j60Write(MAADR3, macaddr[2]);
    enc28j60Write(MAADR2, macaddr[3]);
    enc28j60Write(MAADR1, macaddr[4]);
    enc28j60Write(MAADR0, macaddr[5]);


    //配置PHY为全双工  LEDB为拉电流
    enc28j60PhyWrite(PHCON1, PHCON1_PDPXMD);//MACON3.FULDPX=1和 PHCON1.PDPXMD=1同时为1时才是全双工模式

    // 禁止发送数据loopback
    enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);

    // 上面的设置完后切换到 bank 0去设置
    enc28j60SetBank(ECON1);
    // 使能全局 INT 中断和接收数据包待处理中断已经接收错误中断
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE|EIE_RXERIE );
    // 数据包接收使能
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    /* PHY LED 配置,LED用来指示通信的状态 */	
    enc28j60PhyWrite(PHLCON,0x476);	

    if(enc28j60Read(MAADR5)== macaddr[0])return 0;//初始化成功
    else return 1;
}





