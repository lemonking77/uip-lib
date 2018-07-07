#include "sys.h"
#include "delay.h"
#include "led.h"
#include "usart.h"
#include "spi_enc28j60.h"
#include "enc28j60.h"
#include "ip_arp_tcp_udp.h"
/* mac地址和ip地址在局域网内必须唯一，否则将与其他主机冲突，导致连接不成功 */
//static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24};
//static uint8_t myip[4] = {192,168,1,129};
//static uint16_t myport = 252;

/* 发送数据缓冲区 */
//#define BUFFER_SIZE 1500
//static uint8_t ethbuf[BUFFER_SIZE+1];

unsigned int system_tick_num = 0;
unsigned int sys_now(void)
{
    return system_tick_num;
}
#define paste(front, back) front ## back 
int main(void)
{
//    u8 res;
//    u32 i;
//	uint32_t ethlen=0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
    TIME2_Init(7200,100);   //10ms定时
	LED_Init();		//LED初始化
	uart_init(115200);
    printf("***********************************\r\n");
    Enc28j60_Spi_Init();
	
#if UIP   
    res = enc28j60Init( mymac );
    printf("resss = %d\r\n",res);
    init_ip_arp_udp_tcp( mymac,myip,myport );
	while(1)
	{
		ethlen = enc28j60PacketReceive( BUFFER_SIZE,ethbuf);
        if(ethlen)
        {
            for( i=0;i<ethlen;i++ )
                printf( "%x ", ethbuf[i] );
            printf( "\r\n");
        }
        if(ethlen==0)
        {
            continue;
        }
	}
#endif
}

