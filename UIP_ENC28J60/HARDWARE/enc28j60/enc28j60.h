#ifndef  __enc28j60_H
#define  __enc28j60_H
#include "sys.h"


#define   ENC28J60_RSTL()  // (GPIOE->ODR &= ~(1<<1))//硬件复位引脚没有用到
#define   ENC28J60_RSTH()  // (GPIOE->ODR |= 1<<1)

#define GPIO_ENC28J60_INT    GPIOA                  //ENC28J60的中断引脚
#define ENC28J60_INT_PIN     GPIO_Pin_3
#define ENC28J60_INT_MODE    GPIO_Mode_IPU

void Enc28j60_INTPin_Init(void);



// ENC28J60 控制寄存器重新定义的联合地址包括了三个部分(这个联合地址是人为定义的)：
// Ethernet/MAC/PHY 的区分位|块号地址|寄存器地址，详见《ENC28J60数据手册》P14。
// - Register address           (bits 0-4)  //最低5位是寄存器地址
// - Bank number                (bits 5-6)  //块号位占两位，是ECON1寄存器中的存储区选择位 BSEL1:BSEL0 进行选择
// - MAC/PHY indicator          (bit 7)     //寄存器类型区分位，为0表示以E开头的寄存器，因为在操作(MAC/MII)类寄存器不同于ETH类寄存器；

#define ADDR_MASK        0x1F   //地址掩码
#define BANK_MASK        0x60   //块号掩码
#define SPRD_MASK        0x80   //寄存器掩码

//所有块在0x1B~0x1F的地址上有相同的寄存器，对这寄存器操作的时候不需要进行块号更改，因为芯片内部进行了映射
// All-bank registers
#define EIE              0x1B
#define EIR              0x1C
#define ESTAT            0x1D
#define ECON2            0x1E
#define ECON1            0x1F
//每个块的寄存器区分是通过地址8个bits中的地址位，存储块号位和寄存器位来区分的
//详见《ENC28J60数据手册》P12
//     寄存器名          (寄存器地址|块号|PHY和MAC区分位)
// Bank 0 registers
#define ERDPTL           (0x00|0x00)    //读指针(用户读接收缓冲区的指针，默认硬件移动)
#define ERDPTH           (0x01|0x00)
#define EWRPTL           (0x02|0x00)    //写指针(用户写发送缓冲区的指针，默认硬件移动)
#define EWRPTH           (0x03|0x00)
#define ETXSTL           (0x04|0x00)    //发送区起始地址
#define ETXSTH           (0x05|0x00)
#define ETXNDL           (0x06|0x00)    //发送区结束地址
#define ETXNDH           (0x07|0x00)
#define ERXSTL           (0x08|0x00)    //接收区开始地址
#define ERXSTH           (0x09|0x00)
#define ERXNDL           (0x0A|0x00)    //接收区结束地址 (以上4个地址是用户设定来划分8K的FIFO)
#define ERXNDH           (0x0B|0x00)
#define ERXRDPTL         (0x0C|0x00)    //接收读指针(是给用户来指定这个位置之后的FIFO内存硬件不能向其写数据，位置移动由用户操作，主要是保护数据不被覆盖)
#define ERXRDPTH         (0x0D|0x00)
#define ERXWRPTL         (0x0E|0x00)    //接收写指针(ENC向FIFO写入数据的位置，由硬件自动移动，一定不能超过接收读指针)
#define ERXWRPTH         (0x0F|0x00)
#define EDMASTL          (0x10|0x00)    //DMA 起始位置
#define EDMASTH          (0x11|0x00)
#define EDMANDL          (0x12|0x00)    //DMA 结束位置
#define EDMANDH          (0x13|0x00)
#define EDMADSTL         (0x14|0x00)    //DMA 目标位置
#define EDMADSTH         (0x15|0x00)
#define EDMACSL          (0x16|0x00)    //DMA 校验和
#define EDMACSH          (0x17|0x00)
// Bank 1 registers
#define EHT0             (0x00|0x20)    //哈希表字节 0
#define EHT1             (0x01|0x20)
#define EHT2             (0x02|0x20)
#define EHT3             (0x03|0x20)
#define EHT4             (0x04|0x20)
#define EHT5             (0x05|0x20)
#define EHT6             (0x06|0x20)
#define EHT7             (0x07|0x20)
#define EPMM0            (0x08|0x20)    //格式匹配屏蔽字节 0
#define EPMM1            (0x09|0x20)
#define EPMM2            (0x0A|0x20)
#define EPMM3            (0x0B|0x20)
#define EPMM4            (0x0C|0x20)
#define EPMM5            (0x0D|0x20)
#define EPMM6            (0x0E|0x20)
#define EPMM7            (0x0F|0x20)
#define EPMCSL           (0x10|0x20)    //格式匹配校验和
#define EPMCSH           (0x11|0x20)
#define EPMOL            (0x14|0x20)    //格式匹配偏移地址
#define EPMOH            (0x15|0x20)
#define EWOLIE           (0x16|0x20)    //以太网 LAN 唤醒中断允许寄存器
#define EWOLIR           (0x17|0x20)
#define ERXFCON          (0x18|0x20)    //接收过滤器控制寄存器
#define EPKTCNT          (0x19|0x20)    //以太网数据包计数
// Bank 2 registers
#define MACON1           (0x00|0x40|0x80)//控制寄存器
#define MACON2           (0x01|0x40|0x80)
#define MACON3           (0x02|0x40|0x80)
#define MACON4           (0x03|0x40|0x80)
#define MABBIPG          (0x04|0x40|0x80)//背对背包间间隔寄存器（最小包间隔时间）
#define MAIPGL           (0x06|0x40|0x80)//非背对背包间间隔
#define MAIPGH           (0x07|0x40|0x80)
#define MACLCON1         (0x08|0x40|0x80)//自动重发次数最大值(仅半双工模式)
#define MACLCON2         (0x09|0x40|0x80)//冲突窗口(仅半双工模式)
#define MAMXFLL          (0x0A|0x40|0x80)//最大帧长度 (一般不超过14+1500+4)
#define MAMXFLH          (0x0B|0x40|0x80)
#define MAPHSUP          (0x0D|0x40|0x80)//PHY 复位寄存器
#define MICON            (0x11|0x40|0x80)//MII 控制寄存器( MII复位 )
#define MICMD            (0x12|0x40|0x80)//MII 命令寄存器
#define MIREGADR         (0x14|0x40|0x80)//MII 地址寄存器(对PHY类寄存器操作前都要先把PHY的寄存器地址写入到这个里面)
#define MIWRL            (0x16|0x40|0x80)//MII 写数据 向PHY类寄存器写数据，就是往这个寄存器中写的，必须先写低字节在写高字节
#define MIWRH            (0x17|0x40|0x80)
#define MIRDL            (0x18|0x40|0x80)//MII 读数据 读取PHY类寄存器数据，就是读这个寄存器《ENC28J60数据手册》P21
#define MIRDH            (0x19|0x40|0x80)
// Bank 3 registers
#define MAADR1           (0x00|0x60|0x80)//MAC的源地址(在Enc8J60中的MAC地址是字节向后的,就是第一个要写在MAADR5)
#define MAADR0           (0x01|0x60|0x80)
#define MAADR3           (0x02|0x60|0x80)
#define MAADR2           (0x03|0x60|0x80)
#define MAADR5           (0x04|0x60|0x80)
#define MAADR4           (0x05|0x60|0x80)
#define EBSTSD           (0x06|0x60)
#define EBSTCON          (0x07|0x60)
#define EBSTCSL          (0x08|0x60)
#define EBSTCSH          (0x09|0x60)
#define MISTAT           (0x0A|0x60|0x80)//MII 状态寄存器
#define EREVID           (0x12|0x60)     //版本信息寄存器
#define ECOCON           (0x15|0x60)     //时钟输出控制寄存器
#define EFLOCON          (0x17|0x60)     //以太网流量控制寄存器
#define EPAUSL           (0x18|0x60)     //暂停定时器值
#define EPAUSH           (0x19|0x60)
// PHY registers (PHY寄存器不能通过 SPI 控制接口直接访问)详见《ENC28J60数据手册》P21
#define PHCON1           0x00   //PHY 控制寄存器
#define PHSTAT1          0x01   //物理层状态寄存器 1 只读
#define PHHID1           0x02
#define PHHID2           0x03
#define PHCON2           0x10   //PHY 控制寄存器
#define PHSTAT2          0x11   //物理层状态寄存器 2 只读
#define PHIE             0x12   //PHY 中断允许寄存器
#define PHIR             0x13   //PHY 中断请求标志寄存器
#define PHLCON           0x14   //PHY 模块 LED 控制寄存器

//以下是个别寄存中的位定义，便于实现对某个位的单独操作，例如：清零和置1等。
// ENC28J60 ERXFCON Register Bit Definitions
#define ERXFCON_UCEN     0x80
#define ERXFCON_ANDOR    0x40
#define ERXFCON_CRCEN    0x20
#define ERXFCON_PMEN     0x10
#define ERXFCON_MPEN     0x08
#define ERXFCON_HTEN     0x04
#define ERXFCON_MCEN     0x02
#define ERXFCON_BCEN     0x01
// ENC28J60 EIE Register Bit Definitions
#define EIE_INTIE        0x80
#define EIE_PKTIE        0x40
#define EIE_DMAIE        0x20
#define EIE_LINKIE       0x10
#define EIE_TXIE         0x08
#define EIE_WOLIE        0x04
#define EIE_TXERIE       0x02
#define EIE_RXERIE       0x01
// ENC28J60 EIR Register Bit Definitions
#define EIR_PKTIF        0x40
#define EIR_DMAIF        0x20
#define EIR_LINKIF       0x10
#define EIR_TXIF         0x08
#define EIR_WOLIF        0x04
#define EIR_TXERIF       0x02
#define EIR_RXERIF       0x01
// ENC28J60 ESTAT Register Bit Definitions
#define ESTAT_INT        0x80
#define ESTAT_LATECOL    0x10
#define ESTAT_RXBUSY     0x04
#define ESTAT_TXABRT     0x02
#define ESTAT_CLKRDY     0x01
// ENC28J60 ECON2 Register Bit Definitions
#define ECON2_AUTOINC    0x80
#define ECON2_PKTDEC     0x40
#define ECON2_PWRSV      0x20
#define ECON2_VRPS       0x08
// ENC28J60 ECON1 Register Bit Definitions
#define ECON1_TXRST      0x80
#define ECON1_RXRST      0x40
#define ECON1_DMAST      0x20
#define ECON1_CSUMEN     0x10
#define ECON1_TXRTS      0x08
#define ECON1_RXEN       0x04
#define ECON1_BSEL1      0x02   //BSEL1:BSEL0： 存储块区选择位
#define ECON1_BSEL0      0x01
// ENC28J60 MACON1 Register Bit Definitions
#define MACON1_LOOPBK    0x10
#define MACON1_TXPAUS    0x08
#define MACON1_RXPAUS    0x04
#define MACON1_PASSALL   0x02
#define MACON1_MARXEN    0x01
// ENC28J60 MACON2 Register Bit Definitions
#define MACON2_MARST     0x80
#define MACON2_RNDRST    0x40
#define MACON2_MARXRST   0x08
#define MACON2_RFUNRST   0x04
#define MACON2_MATXRST   0x02
#define MACON2_TFUNRST   0x01
// ENC28J60 MACON3 Register Bit Definitions
#define MACON3_PADCFG2   0x80
#define MACON3_PADCFG1   0x40
#define MACON3_PADCFG0   0x20
#define MACON3_TXCRCEN   0x10
#define MACON3_PHDRLEN   0x08
#define MACON3_HFRMLEN   0x04
#define MACON3_FRMLNEN   0x02
#define MACON3_FULDPX    0x01
// ENC28J60 MICMD Register Bit Definitions
#define MICMD_MIISCAN    0x02
#define MICMD_MIIRD      0x01
// ENC28J60 MISTAT Register Bit Definitions
#define MISTAT_NVALID    0x04
#define MISTAT_SCAN      0x02
#define MISTAT_BUSY      0x01
// ENC28J60 PHY PHCON1 Register Bit Definitions
#define PHCON1_PRST      0x8000
#define PHCON1_PLOOPBK   0x4000
#define PHCON1_PPWRSV    0x0800
#define PHCON1_PDPXMD    0x0100
// ENC28J60 PHY PHSTAT1 Register Bit Definitions
#define PHSTAT1_PFDPX    0x1000
#define PHSTAT1_PHDPX    0x0800
#define PHSTAT1_LLSTAT   0x0004
#define PHSTAT1_JBSTAT   0x0002
// ENC28J60 PHY PHCON2 Register Bit Definitions
#define PHCON2_FRCLINK   0x4000
#define PHCON2_TXDIS     0x2000
#define PHCON2_JABBER    0x0400
#define PHCON2_HDLDIS    0x0100

// ENC28J60 Packet Control Byte Bit Definitions
#define PKTCTRL_PHUGEEN  0x08
#define PKTCTRL_PPADEN   0x04
#define PKTCTRL_PCRCEN   0x02
#define PKTCTRL_POVERRIDE 0x01

// SPI 指令集  详见《ENC28J60手册 P28》
/* 读控制寄存器 */  
#define ENC28J60_READ_CTRL_REG         0x00  
/* 读缓冲区 */  
#define ENC28J60_READ_BUF_MEM          0x3A  
/* 写控制寄存器 */  
#define ENC28J60_WRITE_CTRL_REG        0x40  
/* 写缓冲区 */  
#define ENC28J60_WRITE_BUF_MEM         0x7A  
/* 位域置位 */  
#define ENC28J60_BIT_FIELD_SET         0x80  
/* 位域清零 */  
#define ENC28J60_BIT_FIELD_CLR         0xA0  
/* 系统复位 */  
#define ENC28J60_SOFT_RESET            0xFF  


/* 对8K(0x00~0x1FF)的缓冲区进行划分，0x00~0x1FFF-0x0600-1为接收buff的空间，大小为：6654Bytes
0x1FFF-0x0600~0x1FFF为发送buff的空间，大小为1536Byts;
*/
// start with recbuf at 0/
#define RXSTART_INIT     0x0
// receive buffer end
#define RXSTOP_INIT      (0x1FFF-0x0600-1)
// start TX buffer at 0x1FFF-0x0600, pace for one full ethernet frame (~1500 bytes)
#define TXSTART_INIT     (0x1FFF-0x0600)
// stp TX buffer at end of mem
#define TXSTOP_INIT      0x1FFF

// MAC的最大数据长度MTU，MAC帧的最大长度是14+1500+4
#define MAX_FRAMELEN        1500        // (note: maximum ethernet frame length would be 1518)


u8   enc28j60ReadOp(u8 op, u8 address);
void enc28j60WriteOp(u8 op, u8 address, u8 data);
void enc28j60ReadBuffer(u32 len, u8* data);
void enc28j60WriteBuffer(u32 len, u8* data);
void enc28j60SetBank(u8 address);
u8   enc28j60Read(u8 address);
void enc28j60Write(u8 address, u8 data);
void enc28j60PhyWrite(u8 address, u32 data);
u16  enc28j60PhyRead(u8 address);
void enc28j60clkout(u8 clk);
u8 enc28j60Init(u8* macaddr);
u8   enc28j60getrev(void);

void enc28j60PacketSend(u32 len, u8* packet);
u32  enc28j60PacketReceive(u32 maxlen, u8* packet);

#endif
