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
#define ETHTYPE_ARP_H_V 0x08    //MAC֡���е����ͱ�����ARP����֡
#define ETHTYPE_ARP_L_V 0x06
#define ETHTYPE_IP_H_V  0x08    //MAC֡���е����ͱ�����IP����֡
#define ETHTYPE_IP_L_V  0x00
// byte positions in the ethernet frame:
//
// Ethernet type field (2bytes):
//���λ���������MAC֡��ʼλ�õģ�����Ҫע��MAC֡ǰ��6���ֽڵ�������enc28j60��ӵ�
#define ETH_TYPE_H_P 12     //MAC֡���е�����
#define ETH_TYPE_L_P 13
//
#define ETH_DST_MAC 0
#define ETH_SRC_MAC 6


// ******* ARPӦ������� *******
#define ETH_ARP_OPCODE_REPLY_H_V 0x0
#define ETH_ARP_OPCODE_REPLY_L_V 0x02
//
#define ETHTYPE_ARP_L_V 0x06
//�������е�λ�����������̫��MAC֡�Ŀ�ʼλ�õ�
// arp.opcode
#define ETH_ARP_OPCODE_H_P 0x14 //�����룬���ڱ�����ARPӦ����ARP����
#define ETH_ARP_OPCODE_L_P 0x15
// arp.src.mac
#define ETH_ARP_SRC_MAC_P 0x16  //Դ��MAC��ַ
#define ETH_ARP_SRC_IP_P 0x1c   //Դ��ip��ַ
#define ETH_ARP_DST_MAC_P 0x20  //Ŀ��MAC��ַ
#define ETH_ARP_DST_IP_P 0x26   //Ŀ��ip��ַ

// ******* IP *******
#define IP_HEADER_LEN	20      //IP��ͷ����
// ip.src                       //�����IP��ͷ�е�λ���������MAC֡��ʼλ�õ�
#define IP_HEADER_LEN_VER_P 0xe //IP�ײ�
#define IP_SRC_P 0x1a           //Դ��IP
#define IP_DST_P 0x1e           //Ŀ��IP
#define IP_TOTLEN_H_P 0x10      //IP�����ܳ���
#define IP_TOTLEN_L_P 0x11
#define IP_FLAGS_P 0x14         //��־��Ƭƫ��
#define IP_TTL_P 0x16           //����ʱ���Э������
#define IP_PROTO_P 0x17         //IP���ݰ���������ʲôЭ�������
#define IP_CHECKSUM_P 0x18      //IP�ײ�У���
#define IP_P 0xe                //ip�����ײ���MAC֡��λ��

#define IP_PROTO_ICMP_V 1   //ICMPЭ������Ϊ1
#define IP_PROTO_TCP_V 6    //TCPЭ������Ϊ6
// 17=0x11
#define IP_PROTO_UDP_V 17   //UDPЭ������Ϊ17
// ******* ICMP *******
#define ICMP_TYPE_ECHOREPLY_V 0     //����Ӧ������
#define ICMP_TYPE_ECHOREQUEST_V 8   //������������
//
#define ICMP_TYPE_P 0x22            //ICMP����
#define ICMP_CHECKSUM_P 0x24        //У���λ��

// ******* UDP *******
#define UDP_HEADER_LEN	8       //UDP��ͷ����
//
#define UDP_SRC_PORT_H_P 0x22   //Դ�˿�λ��
#define UDP_SRC_PORT_L_P 0x23
#define UDP_DST_PORT_H_P 0x24   //Ŀ�Ķ˿�
#define UDP_DST_PORT_L_P 0x25
//
#define UDP_LEN_H_P 0x26        //����
#define UDP_LEN_L_P 0x27
#define UDP_CHECKSUM_H_P 0x28   //У���
#define UDP_CHECKSUM_L_P 0x29
#define UDP_DATA_P 0x2a         //����

// ******* TCP *******
#define TCP_SRC_PORT_H_P 0x22   //Դ�˿�λ��
#define TCP_SRC_PORT_L_P 0x23   
#define TCP_DST_PORT_H_P 0x24   //Ŀ�Ķ˿�λ��
#define TCP_DST_PORT_L_P 0x25
// the tcp seq number is 4 bytes 0x26-0x29
#define TCP_SEQ_H_P 0x26        //�������
#define TCP_SEQACK_H_P 0x2a     //Ӧ�����
// flags: SYN=2
#define TCP_FLAGS_P 0x2f        //6��֡��ʾλ���ӵ͵�������ΪFIN,SYN,RST,PSH,ACK,URG
#define TCP_FLAGS_SYN_V 2
#define TCP_FLAGS_FIN_V 1
#define TCP_FLAGS_PUSH_V 8
#define TCP_FLAGS_SYNACK_V 0x12//ACK+SYN
#define TCP_FLAGS_ACK_V 0x10
#define TCP_FLAGS_PSHACK_V 0x18
//  plain len without the options:
#define TCP_HEADER_LEN_PLAIN 20 //TCP�ײ�����
#define TCP_HEADER_LEN_P 0x2e   //����ƫ��(TCP��ͷ��С)
#define TCP_WINDOW_H_P 0x30     //���ڴ�С
#define TCP_WINDOW_L_P 0x31     
#define TCP_CHECKSUM_H_P 0x32   //У���
#define TCP_CHECKSUM_L_P 0x33   
#define TCP_OPTIONS_P 0x36      //��ѡ��λ��
//


#if 0

#ifndef LITTLE_ENDIAN       //С��ģʽ
#define LITTLE_ENDIAN   (1)   //BYTE ORDER   
#else   
#error Redefine LITTLE_ORDER   
#endif   

//Macͷ�����ܳ���14�ֽ�   
typedef __packed  struct _eth_hdr  
{  
    unsigned char dstmac[6]; //Ŀ��mac��ַ   
    unsigned char srcmac[6]; //Դmac��ַ   
    unsigned short eth_type; //��̫������   
}eth_hdr;  
//IPͷ�����ܳ���20�ֽ�   
typedef __packed  struct _ip_hdr  
{  
    #if LITTLE_ENDIAN   
    unsigned char ihl:4;     //�ײ�����   
    unsigned char version:4; //�汾    
    #else   
    unsigned char version:4, //�汾   
    unsigned char ihl:4;     //�ײ�����   
    #endif   
    unsigned char tos;       //��������   
    unsigned short tot_len;  //�ܳ���   
    unsigned short id;       //��־   
    unsigned short frag_off; //��Ƭƫ��   
    unsigned char ttl;       //����ʱ��   
    unsigned char protocol;  //Э��   
    unsigned short chk_sum;  //�����   
    unsigned char srcaddr[4];  //ԴIP��ַ   
    unsigned char dstaddr[4];  //Ŀ��IP��ַ   
}ip_hdr;  
//TCPͷ�����ܳ���20�ֽ�   
typedef __packed struct _tcp_hdr  
{  
    unsigned short src_port;    //Դ�˿ں�   
    unsigned short dst_port;    //Ŀ�Ķ˿ں�   
    unsigned int seq_no;        //���к�   
    unsigned int ack_no;        //ȷ�Ϻ�   
    #if LITTLE_ENDIAN   
    unsigned char reserved_1:4; //����6λ�е�4λ�ײ�����   
    unsigned char thl:4;        //tcpͷ������   
    unsigned char flag:6;       //6λ��־   
    unsigned char reseverd_2:2; //����6λ�е�2λ   
    #else   
    unsigned char thl:4;        //tcpͷ������   
    unsigned char reserved_1:4; //����6λ�е�4λ�ײ�����   
    unsigned char reseverd_2:2; //����6λ�е�2λ   
    unsigned char flag:6;       //6λ��־    
    #endif   
    unsigned short wnd_size;    //16λ���ڴ�С   
    unsigned short chk_sum;     //16λTCP�����   
    unsigned short urgt_p;      //16Ϊ����ָ��   
}tcp_hdr;  
//UDPͷ�����ܳ���8�ֽ�   
typedef __packed struct _udp_hdr  
{  
    unsigned short src_port; //Զ�˿ں�   
    unsigned short dst_port; //Ŀ�Ķ˿ں�   
    unsigned short uhl;      //udpͷ������   
    unsigned short chk_sum;  //16λudp�����   
}udp_hdr;  
//ICMPͷ�����ܳ���4�ֽ�   
typedef __packed struct _icmp_hdr  
{  
    unsigned char icmp_type;   //����   
    unsigned char code;        //����   
    unsigned short chk_sum;    //16λ�����   
}icmp_hdr;   
#endif

#endif
//@}

