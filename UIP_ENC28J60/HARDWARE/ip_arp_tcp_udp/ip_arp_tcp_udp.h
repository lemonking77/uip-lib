#ifndef  __ip_arp_tcp_udp_H
#define  __ip_arp_tcp_udp_H
#include "sys.h"

//≥ı ºªØ
extern void init_ip_arp_udp_tcp(unsigned char *mymac,unsigned char *myip,unsigned short wwwp);
//
extern unsigned char eth_type_is_arp_and_my_ip(unsigned char *buf,unsigned int len);
extern unsigned char eth_type_is_ip_and_my_ip(unsigned char *buf,unsigned int len);
extern void make_arp_answer_from_request(unsigned char *buf);
extern void make_echo_reply_from_request(unsigned char *buf,unsigned int len);
extern void make_udp_reply_from_request(unsigned char *buf,char *data,unsigned int datalen,unsigned int port);


extern void make_tcp_synack_from_syn(unsigned char *buf);
extern void init_len_info(unsigned char *buf);
extern unsigned int get_tcp_data_pointer(void);
extern unsigned int fill_tcp_data_p(unsigned char *buf,unsigned int pos, const unsigned char *progmem_s);
extern unsigned int fill_tcp_data(unsigned char *buf,unsigned int pos, const char *s);
extern void make_tcp_ack_from_any(unsigned char *buf);
extern void make_tcp_ack_with_data(unsigned char *buf,unsigned int dlen);

extern unsigned int chack_sum(unsigned char *buf,unsigned int len,char type);

#endif
