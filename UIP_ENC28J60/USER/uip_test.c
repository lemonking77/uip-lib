#include "usart.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "timer.h"

void Polling(void);

uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24};

struct timer periodic_timer=0, arp_timer=0;
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

void main_test()
{
	uip_ipaddr_t ipaddr;

	timer_set(&periodic_timer, CLOCK_SECOND / 2);
	timer_set(&arp_timer, CLOCK_SECOND * 10);

	tapdev_init( mymac );
	uip_init();

	uip_ipaddr(ipaddr, 192,168,0,2);	/* 本机IP */
	uip_sethostaddr(ipaddr);
	uip_ipaddr(ipaddr, 255,255,255,0);	/* 子网掩码 */
	uip_setnetmask(ipaddr);
	uip_ipaddr(ipaddr, 192,168,0,1);	/* 网关地址，链接外网时使用 */
	uip_setdraddr(ipaddr);
	while(1)
	{
		Polling();
	}
	
}

void Polling()
{
	int i;
    uip_len = tapdev_read();
	if(uip_len > 0) 
	{
		if(BUF->type == htons(UIP_ETHTYPE_IP)) 
		{
			uip_arp_ipin();
			uip_input();
			/* If the above function invocation resulted in data that
			should be sent out on the network, the global variable
			uip_len is set to a value > 0. */
			if(uip_len > 0) 
			{
				uip_arp_out();
				tapdev_send();
			}
		} 
		else if(BUF->type == htons(UIP_ETHTYPE_ARP)) 
		{
			uip_arp_arpin();
			/* If the above function invocation resulted in data that
			should be sent out on the network, the global variable
			uip_len is set to a value > 0. */
			if(uip_len > 0)	/* 发送的是ARP应答 */
			{
				tapdev_send();
			}
		}

	}
	else if(timer_expired(&periodic_timer))
	{
		timer_reset(&periodic_timer);
		for(i = 0; i < UIP_CONNS; i++) 
		{
			uip_periodic(i);
			/* If the above function invocation resulted in data that
			should be sent out on the network, the global variable
			uip_len is set to a value > 0. */
			if(uip_len > 0)
			{
				uip_arp_out();
				tapdev_send();
			}
		}

#if UIP_UDP
		for(i = 0; i < UIP_UDP_CONNS; i++) 
		{
		uip_udp_periodic(i);
		/* If the above function invocation resulted in data that
		should be sent out on the network, the global variable
		uip_len is set to a value > 0. */
			if(uip_len > 0) 
			{
				uip_arp_out();
				tapdev_send();
			}
		}
#endif /* UIP_UDP */

	/* Call the ARP timer function every 10 seconds. */
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
    }
}

void uip_log(char *m)
{
  printf("uIP log message: %s\n", m);
}
