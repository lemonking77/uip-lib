/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher 
 * Copyright: GPL V2
 *
 * Based on the net.h file from the AVRlib library by Pascal Stang.
 * For AVRlib See http://www.procyonengineering.com/
 * Used with explicit permission of Pascal Stang.
 *
 * Chip type           : ATMEGA88 with ENC28J60
 *********************************************/



/*********************************************
 * modified: 2007-08-08
 * Author  : awake
 * Copyright: GPL V2
 * http://www.icdev.com.cn/?2213/
 * Host chip: ADUC7026
**********************************************/





// notation: _P = position of a field
//           _V = value of a field

//@{

#ifndef NET_H
#define NET_H

// ******* ETH *******
#define ETH_HEADER_LEN	14
// values of certain bytes:
#define ETHTYPE_ARP_H_V 0x08    //MAC帧首中的类型表明是ARP数据帧
#define ETHTYPE_ARP_L_V 0x06
#define ETHTYPE_IP_H_V  0x08    //MAC帧首中的类型表明是IP数据帧
#define ETHTYPE_IP_L_V  0x00
// byte positions in the ethernet frame:
//
// Ethernet type field (2bytes):
//这个位置是相对于MAC帧开始位置的；但是要注意MAC帧前有6个字节的数据是enc28j60添加的
#define ETH_TYPE_H_P 12     //MAC帧首中的类型
#define ETH_TYPE_L_P 13
//
#define ETH_DST_MAC 0
#define ETH_SRC_MAC 6


// ******* ARP应答操作码 *******
#define ETH_ARP_OPCODE_REPLY_H_V 0x0
#define ETH_ARP_OPCODE_REPLY_L_V 0x02
//
#define ETHTYPE_ARP_L_V 0x06
//这里所有的位置是相对于以太网MAC帧的开始位置的
// arp.opcode
#define ETH_ARP_OPCODE_H_P 0x14 //操作码，用于表明是ARP应答还是ARP请求
#define ETH_ARP_OPCODE_L_P 0x15
// arp.src.mac
#define ETH_ARP_SRC_MAC_P 0x16  //源端MAC地址
#define ETH_ARP_SRC_IP_P 0x1c   //源端ip地址
#define ETH_ARP_DST_MAC_P 0x20  //目的MAC地址
#define ETH_ARP_DST_IP_P 0x26   //目的ip地址

// ******* IP *******
#define IP_HEADER_LEN	20      //IP报头长度
// ip.src                       //这里的IP报头中的位置是相对于MAC帧开始位置的
#define IP_HEADER_LEN_VER_P 0xe //IP首部
#define IP_SRC_P 0x1a           //源端IP
#define IP_DST_P 0x1e           //目的IP
#define IP_TOTLEN_H_P 0x10      //IP报文总长度
#define IP_TOTLEN_L_P 0x11
#define IP_FLAGS_P 0x14         //标志和片偏移
#define IP_TTL_P 0x16           //生存时间和协议类型
#define IP_PROTO_P 0x17         //IP数据包包含的是什么协议的数据
#define IP_CHECKSUM_P 0x18      //IP首部校验和
#define IP_P 0xe                //ip报文首部在MAC帧中位置

#define IP_PROTO_ICMP_V 1   //ICMP协议类型为1
#define IP_PROTO_TCP_V 6    //TCP协议类型为6
// 17=0x11
#define IP_PROTO_UDP_V 17   //UDP协议类型为17
// ******* ICMP *******
#define ICMP_TYPE_ECHOREPLY_V 0     //回显应答类型
#define ICMP_TYPE_ECHOREQUEST_V 8   //回显请求类型
//
#define ICMP_TYPE_P 0x22            //ICMP类型
#define ICMP_CHECKSUM_P 0x24        //校验和位置

// ******* UDP *******
#define UDP_HEADER_LEN	8       //UDP报头长度
//
#define UDP_SRC_PORT_H_P 0x22   //源端口位置
#define UDP_SRC_PORT_L_P 0x23
#define UDP_DST_PORT_H_P 0x24   //目的端口
#define UDP_DST_PORT_L_P 0x25
//
#define UDP_LEN_H_P 0x26        //长度
#define UDP_LEN_L_P 0x27
#define UDP_CHECKSUM_H_P 0x28   //校验和
#define UDP_CHECKSUM_L_P 0x29
#define UDP_DATA_P 0x2a         //数据

// ******* TCP *******
#define TCP_SRC_PORT_H_P 0x22   //源端口位置
#define TCP_SRC_PORT_L_P 0x23   
#define TCP_DST_PORT_H_P 0x24   //目的端口位置
#define TCP_DST_PORT_L_P 0x25
// the tcp seq number is 4 bytes 0x26-0x29
#define TCP_SEQ_H_P 0x26        //报文序号
#define TCP_SEQACK_H_P 0x2a     //应答序号
// flags: SYN=2
#define TCP_FLAGS_P 0x2f        //6个帧表示位，从低到高依次为FIN,SYN,RST,PSH,ACK,URG
#define TCP_FLAGS_SYN_V 2
#define TCP_FLAGS_FIN_V 1
#define TCP_FLAGS_PUSH_V 8
#define TCP_FLAGS_SYNACK_V 0x12//ACK+SYN
#define TCP_FLAGS_ACK_V 0x10
#define TCP_FLAGS_PSHACK_V 0x18
//  plain len without the options:
#define TCP_HEADER_LEN_PLAIN 20 //TCP首部长度
#define TCP_HEADER_LEN_P 0x2e   //数据偏移(TCP报头大小)
#define TCP_WINDOW_H_P 0x30     //窗口大小
#define TCP_WINDOW_L_P 0x31     
#define TCP_CHECKSUM_H_P 0x32   //校验和
#define TCP_CHECKSUM_L_P 0x33   
#define TCP_OPTIONS_P 0x36      //可选项位置
//


#if 0

#ifndef LITTLE_ENDIAN       //小端模式
#define LITTLE_ENDIAN   (1)   //BYTE ORDER   
#else   
#error Redefine LITTLE_ORDER   
#endif   

//Mac头部，总长度14字节   
typedef __packed  struct _eth_hdr  
{  
    unsigned char dstmac[6]; //目标mac地址   
    unsigned char srcmac[6]; //源mac地址   
    unsigned short eth_type; //以太网类型   
}eth_hdr;  
//IP头部，总长度20字节   
typedef __packed  struct _ip_hdr  
{  
    #if LITTLE_ENDIAN   
    unsigned char ihl:4;     //首部长度   
    unsigned char version:4; //版本    
    #else   
    unsigned char version:4, //版本   
    unsigned char ihl:4;     //首部长度   
    #endif   
    unsigned char tos;       //服务类型   
    unsigned short tot_len;  //总长度   
    unsigned short id;       //标志   
    unsigned short frag_off; //分片偏移   
    unsigned char ttl;       //生存时间   
    unsigned char protocol;  //协议   
    unsigned short chk_sum;  //检验和   
    unsigned char srcaddr[4];  //源IP地址   
    unsigned char dstaddr[4];  //目的IP地址   
}ip_hdr;  
//TCP头部，总长度20字节   
typedef __packed struct _tcp_hdr  
{  
    unsigned short src_port;    //源端口号   
    unsigned short dst_port;    //目的端口号   
    unsigned int seq_no;        //序列号   
    unsigned int ack_no;        //确认号   
    #if LITTLE_ENDIAN   
    unsigned char reserved_1:4; //保留6位中的4位首部长度   
    unsigned char thl:4;        //tcp头部长度   
    unsigned char flag:6;       //6位标志   
    unsigned char reseverd_2:2; //保留6位中的2位   
    #else   
    unsigned char thl:4;        //tcp头部长度   
    unsigned char reserved_1:4; //保留6位中的4位首部长度   
    unsigned char reseverd_2:2; //保留6位中的2位   
    unsigned char flag:6;       //6位标志    
    #endif   
    unsigned short wnd_size;    //16位窗口大小   
    unsigned short chk_sum;     //16位TCP检验和   
    unsigned short urgt_p;      //16为紧急指针   
}tcp_hdr;  
//UDP头部，总长度8字节   
typedef __packed struct _udp_hdr  
{  
    unsigned short src_port; //远端口号   
    unsigned short dst_port; //目的端口号   
    unsigned short uhl;      //udp头部长度   
    unsigned short chk_sum;  //16位udp检验和   
}udp_hdr;  
//ICMP头部，总长度4字节   
typedef __packed struct _icmp_hdr  
{  
    unsigned char icmp_type;   //类型   
    unsigned char code;        //代码   
    unsigned short chk_sum;    //16位检验和   
}icmp_hdr;   
#endif

#endif
//@}

