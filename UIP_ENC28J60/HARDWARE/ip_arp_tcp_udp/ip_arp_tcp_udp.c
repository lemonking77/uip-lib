#include "ip_arp_tcp_udp.h"
#include "net.h"
#include "usart.h"
#include "enc28j60.h"


static uint8_t ipv4addr[4];
static uint8_t macaddr[6];
static uint16_t wwwport;
static unsigned int info_hdr_len=0; //TCP������ƫ��
static unsigned int info_data_len=0;//TCP�е����ݳ���
static unsigned char seqnum=0xa;    // �����ĳ�ʼ���

#define DEBUG 0

/*
 * ��������init_ip_arp_udp_tcp
 * ����  �����ⲿ������ʼ��ip��ַ��mac��ַ�Ͷ˿ں�
 * ����  ��mymac ����MAC��ַ��myip ����ip��ַ��wwwp ���ض˿ں�
 * ���  ����	
 */
void init_ip_arp_udp_tcp( unsigned char *mymac,unsigned char *myip,unsigned short wwwp )
{
    uint8_t i=0;
    
    wwwport = wwwp;
    for( i=0;i<6;i++ )
        macaddr[i]= *mymac++;
    i = 0;
    for( i=0;i<4;i++ )
        ipv4addr[i] = *myip;  
    wwwport = wwwp;
}

/*
 * ��������eth_type_is_arp_and_my_ip
 * ����  ���жϽ��յ�����̫��֡�Ƿ���ARP֡��������Ƿ��򱾻�ip���͵�
 * ����  ��buf ���յ�����̫��֡���ݵ�ַ��len ���ݵĳ��ȣ���������̫��֡��CRCУ���4���ֽڣ�
 * ���  ��0 ���Ƿ��򱾻�ip��ַ��ARP Э�飬1�Ƿ��򱾻��� 
 */
unsigned char eth_type_is_arp_and_my_ip(unsigned char *buf,unsigned int len)
{
    uint8_t i=0;
    /* ��������̫��֡�е�Э�������Ƿ���ARP */
    if( (buf[ETH_TYPE_H_P]!=ETHTYPE_ARP_H_V) || (buf[ETH_TYPE_L_P]!=ETHTYPE_ARP_L_V) )
        return 0;
    # if DEBUG
    printf("source ip is:");
    for( i=0;i<4;i++ )
    {
        printf("%d",buf[ETH_ARP_SRC_IP_P]);
        if( i != 3 )
            printf(".");
    }
    printf("\r\n");

    printf("source mac is:");
    for( i=0;i<6;i++ )
    {
        printf("%d",buf[ETH_ARP_SRC_MAC_P]);
        if( i != 5 )
            printf(" ");
    }
    printf("\r\n");
    #endif
    /* ���Ŀ��ip�Ƿ��Ǳ���ip */
    for( i=0;i<4;i++ )
    {
        if( buf[ETH_ARP_SRC_IP_P+i] != ipv4addr[i] )
            return 0;
    }
    
    return 1;
}

/*
 * ��������make_eth
 * ����  ���޸�ԴMAC��ַ��Ŀ��MAC��ַ
 * ����  ��buf ���յ�����̫��֡���ݵĴ洢��ַ
 * ���  ��
 * ˼·  ����Ŀ�ĵ�ַ�޸�ΪԴ��ַ��Դ��ַ�޸�Ϊ������mac��ַ
 * ע��  ����̫��֡�������Ҫ�������CRCУ�飬��Ϊenc28j60��Ӳ��У������ֽ����
 */
void make_eth(unsigned char *buf)
{
    uint8_t i;
    for( i=0;i<6;i++ )
    {
        buf[ETH_DST_MAC+i] = buf[ETH_SRC_MAC+i];
        buf[ETH_SRC_MAC+i] = macaddr[i];
    }
}


/*
 * ��������eth_type_is_ip_and_my_ip
 * ����  ���жϽ��յ�����̫��֡�е�Э���Ƿ���IPЭ�飬������Ƿ��򱾻�ip���͵�
 * ����  ��buf ���յ�����̫��֡���ݵ�ַ��len ���ݵĳ��ȣ���������̫��֡��CRCУ���4���ֽڣ�
 * ���  ��0 ������������1��������	
 */
unsigned char eth_type_is_ip_and_my_ip(unsigned char *buf,unsigned int len)
{
    uint8_t i=0;
    /* ��������̫��֡�е�Э�������Ƿ���IP���� */
    if( (buf[ETH_TYPE_H_P]!=ETHTYPE_IP_H_V) || (buf[ETH_TYPE_L_P]!=ETHTYPE_IP_L_V) )
        return 0;
    /* ���IP�����е�Э��汾ipv4�ͱ���ͷ����5*4=20 */
    if( buf[IP_HEADER_LEN_VER_P]!=0x45 ) 
        return 0;
    /* ���Ŀ��ip�Ƿ��Ǳ���ip */
    for( i=0;i<4;i++ )
    {
        if( buf[IP_DST_P+i] != ipv4addr[i] )
            return 0;
    }
    return 1;
}
/*
 * ��������make_arp_answer_from_request
 * ����  ����ӦARP�㲥�����˴���
 * ����  ��buf ���յ�����̫��֡���ݵĴ洢��ַ
 * ���  ��
 * ˼·  ���ѽ��յ������ݽ��в��������ֽڵ��޸�Ȼ��ԭ�����أ�ARP��������û��У��͵ģ�
           ֱ�ӷ���MAC֡�з���ȥ��MAC֡��У�����ENC28J60�Զ���ӡ�
 */
void make_arp_answer_from_request(unsigned char *buf)
{
    uint8_t i;
    /* �޸Ĳ�������ΪARP��Ӧ */
    buf[ETH_ARP_OPCODE_H_P] = ETH_ARP_OPCODE_REPLY_H_V;
    buf[ETH_ARP_OPCODE_L_P] = ETH_ARP_OPCODE_REPLY_L_V;
    
    make_eth( buf );
    /* �޸�ARPЭ��֡�е�Դ��Ŀ��MAC��ַ */
    for( i=0;i<6;i++ )
    {
        buf[ETH_ARP_DST_IP_P+i] = buf[ETH_ARP_SRC_IP_P+i];
        buf[ETH_ARP_SRC_IP_P+i] = macaddr[i];
    }
    /* �޸�ARPЭ��֡�е�Դ��Ŀ��IP��ַ */
    for( i=0;i<4;i++ )
    {
        buf[ETH_ARP_DST_IP_P+i] =  buf[ETH_ARP_SRC_IP_P+i];
        buf[ETH_ARP_SRC_IP_P+i] = ipv4addr[i];
    }
    /* �������ݣ�ARP֡�̶���СΪ14+28=42 */
    enc28j60WriteBuffer( 42, buf );
}
/*
 * ��������chack_sum
 * ����  ��IP��TCP��UDP��У��ͼ���
 * ����  ��buf ����IP��˵��IP�����׵�ַ������TCP/UDP��˵�Ǵ�IP���ĵ�ԴIP��ַ�������Գ�����Ҫ+8
 *         len ��ҪУ������ݳ��ȣ�ע�����TCP/UDP��˵����Ҫ���8�ģ�����ѭ��У�飬Ȼ���ڼ����ʱ��Ҫ��ȥ8
 *         type 0��IPУ�� 1��UDPУ�� 2��TCPУ��
 * ���  ��CRC16У���
 * ˼·  ����У��������ϳ�2�ֽڵ����ݽ����ۼ�У�飬���������Ҫ��0��Ȼ���ٰѽ�λ�ӻ�ȥ�����������ȡ��
 */
unsigned int chack_sum(unsigned char *buf,unsigned int len,char type)
{
    uint8_t i;
    uint64_t sum=0;
    
    if( type==1 )           /* ����UDPУ��ʱ����Э������ */
    {
        sum += IP_PROTO_UDP_V;
        sum += len-8;
    }
    else if( type==2 )      /* ����TCPУ��ʱ����Э������ */
    {
        sum += IP_PROTO_TCP_V;
        sum += len-8;       /* ��ȥ8 ��ԭ������Ϊ���Ǹ��������Ĳ�������TCP���ĳ���֮��+8�� */
    }                       /* ԭ����Ҫ��Դ�˺�Ŀ��˵�IPһ������ۼ� */
    
    for( i=0;i<len-1;i+=2 ) /* ת����2�ֽڽ����ۼ���� */
    {
        sum += ( buf[i]<<8 )|buf[1+i];
    }
    if( len%2 != 0 )        /* ����������Ļ���Ҫ���в�λ */
        sum += ( buf[len-1]<<8 )&0xFF00;
    
    while(sum>>16)          /* �ѽ�λ��ֵ�ڼӻ�ȥ */
    {
        sum = ( sum&0xFFFF )+( sum>>16 ) ;
    }
    return ( uint32_t )( sum^0xFFFF );
}

/*
 * ��������make_ip
 * ����  ���޸ġ������һ���µ�IP�ײ�
 * ����  ��buf ���յ�����̫��֡���ݵĴ洢��ַ
 * ���  ��
 * ˼·  ���޸�Դ�˺�Ŀ�Ķ�IP�����ñ�־��Ƭƫ�ơ�����ʱ�䡢�Լ�����IP�ײ�У��
 * ע��  ����IP������Ҫ�������CRCУ�飬У�����IP�����ײ�
 */
void make_ip(unsigned char *buf)
{
    uint8_t i;
    uint32_t chacksum = 0;
    /* У�����0 */
    buf[IP_CHECKSUM_P] = 0;
    buf[IP_CHECKSUM_P+1] = 0;
    
    /* ���ñ�־��ƫ�� */
    buf[IP_FLAGS_P] = 0x40; //���ñ�־λ���ܶԱ��ķ�Ƭ���������һ������
    buf[IP_FLAGS_P+1] = 0;  //ƫ��λ0
    /* ��������ʱ��TTL */
    buf[IP_TTL_P] = 64;
    /* �޸�IP */
    for( i=0;i<4;i++ )
    {
        buf[IP_DST_P+i] = buf[IP_SRC_P+i];
        buf[IP_SRC_P+i] = ipv4addr[i];
    }
    chacksum = chack_sum( buf,IP_HEADER_LEN,0 );
    buf[IP_CHECKSUM_P] = chacksum&0xFF;
    buf[IP_CHECKSUM_P+1] = (chacksum>>8)&0xFF;
}

/*
 * ��������make_echo_reply_from_request
 * ����  ��ICMP��ping�����Ӧ
 * ����  ��buf ���յ�����̫��֡���ݵĴ洢��ַ
 * ���  ��
 * ˼·  ���ѽ��յ������ݽ����޸�Ȼ��ԭ������
 */
void make_echo_reply_from_request(unsigned char *buf,unsigned int len)
{

    make_eth(buf);
    make_ip(buf);
    /* �޸�ICMP�Ļ������� */
    buf[ICMP_TYPE_P] = ICMP_TYPE_ECHOREPLY_V;
    
    if( buf[ICMP_CHECKSUM_P]>(0xff-0x08) )  //�����0x08�Ǹ��ֽڣ�����Ҫ��У��͵ĸ��ֽڱȽ�
    {
        buf[ICMP_CHECKSUM_P+1]++;           //ʵ�ֽ�λ
    }
    buf[ICMP_CHECKSUM_P] += 0x08;           //�����ֱ�Ӱ�У���λ+8
    enc28j60PacketSend( len, buf);
    
}

void make_udp_reply_from_request(unsigned char *buf,char *data,unsigned int datalen,unsigned int port)
{
    
}

/*
 * ��������make_tcphead
 * ����  ������TCP�����е�һ����(��һ���������κ�TCP���Ķ���ı��)
 * ����  ��buf ���յ�����̫��֡���ݵĴ洢��ַ
           rel_ack_num ȷ�Ϻ�
           mss �Ƿ�ʹ�ÿ�ѡ�ֶ� 0������ 1����
           cp_seq ���
 * ���  ��
 * ˼·  ���ѽ��յ������ݽ����޸�Ȼ��ԭ������
 */
void make_tcphead(unsigned char *buf,unsigned  int rel_ack_num,unsigned char mss,unsigned char cp_seq)
{
    uint8_t i;
    uint16_t tcp_seq;
    
        /* У�����0 */
    buf[TCP_CHECKSUM_H_P] = 0;
    buf[TCP_CHECKSUM_L_P] = 0;
    
    /* �޸�Ŀ�Ķ˿� */
    buf[TCP_DST_PORT_L_P] = buf[TCP_SRC_PORT_L_P];
    buf[TCP_DST_PORT_H_P] = buf[TCP_SRC_PORT_H_P];
    /* �޸�Դ�˿� */
    buf[TCP_SRC_PORT_L_P] = wwwport&0xff;
    buf[TCP_SRC_PORT_H_P] = wwwport>>8;

    /* 
    ��ţ�TCP����ʽ�Ķ�TCP���������е�ÿ���ֽڽ��б�ţ���ô�����ž�����Ҫ���͵����TCP����Ƭ�����������е��ֽ�ƫ�ƣ�
          ������Ҫ�����˷�һ������3000�ֽڣ���һ�η���1500���ֽڣ������0����ô�ڶ��η�����ž���1500��������������
                ��ʾ�ֽڱ���Ƭ�����������е��ֽ�ƫ�ơ�
    ȷ�Ϻţ�ȷ�Ϻ���������Ӧ�����Ƿ��յ����ݣ�ͬʱ���������ڴ��յ��Է���һ��TCP���ݱ����������ֽڵ���ţ�
           �������Է��ȸ��ҷ���һ��100�ֽڵı��ģ��Է������е����к�Ϊ50����ô��һ�����������յ����
                 ��ž���50+100��
    */
    
    for( i=4;i>0;i-- )
    {
        rel_ack_num =  buf[TCP_SEQ_H_P+i-1]+rel_ack_num;/* ��ȡ������ȷ�Ϻ�+���յ����ݳ���=ȷ�Ϻ�(�Է���һ�η�����TCP���������ֽڱ��) */
        tcp_seq = buf[TCP_SEQACK_H_P+i-1];              /* ��ȡ������ȷ�Ϻſ������ֽڵ���Ű� */
        buf[TCP_SEQACK_H_P+i-1] = rel_ack_num&0xff;
        rel_ack_num >>= 8;
        if( cp_seq )    /* ���˷�������Ӧ��ž����Լ����������ֱ�ӿ����Ϳ����� */
        {
            buf[TCP_SEQ_H_P+i-1] = tcp_seq; 
        }
        else{            /* �����0��ʹ��Ĭ����� */
            buf[TCP_SEQ_H_P+i-1] = 0;
        }
    }
    if( cp_seq==0 )/* �����ʹ�öԷ�������ȷ�Ϻ���Ϊ��ŵĻ���ʹ�ó�ʼ��� */
    {
        /* ��������Ĭ����� */
        buf[TCP_SEQ_H_P+0] = 0;
        buf[TCP_SEQ_H_P+1] = 0;
        buf[TCP_SEQ_H_P+2] = seqnum;
        buf[TCP_SEQ_H_P+3] = 0;
        seqnum += 2;
    }
    if( mss )/* �����TCP�Ŀ�ѡ��ֻ����������Ķγ���=MTU-40 */
    {
        buf[TCP_OPTIONS_P+0] = 2;
        buf[TCP_OPTIONS_P+1] = 4;
        buf[TCP_OPTIONS_P+2] = 1408&0xff;
        buf[TCP_OPTIONS_P+3] = 1408>>8;
        
        /* ����������ƫ��(TCP��ͷ����) 6*4=24�ֽ� ���Ϊ15*4=60�ֽ�*/
        buf[TCP_HEADER_LEN_P] &= 0x60;
    }
    else/* Ĭ��TCP��ͷ��СΪ5*4=20�ֽ� */
        buf[TCP_HEADER_LEN_P] &= 0x50;
}
/*
 * ��������make_tcp_synack_from_syn
 * ����  ����ӦTCP��������
 * ����  ��buf ���յ�����̫��֡���ݵĴ洢��ַ
 * ���  ��
 * ˼·  ��
 */
void make_tcp_synack_from_syn(unsigned char *buf)
{
    uint32_t chacknum=0;
    
    make_eth(buf);
    /* ����IP�������ܳ��� = 20(ip_h)+tp_h(20)+tp_op(4) */
    buf[IP_TOTLEN_H_P] = 0;
    buf[IP_TOTLEN_L_P] = IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4;
    
    make_ip(buf);
    
    /* ����TCP�ı�־λ ACK+SYN */
    buf[TCP_FLAGS_P] = TCP_FLAGS_SYNACK_V;

    make_tcphead(buf,1,1,0);
    
    chacknum = chack_sum( &buf[IP_SRC_P],8+TCP_HEADER_LEN_PLAIN,2 );
    buf[TCP_CHECKSUM_H_P] = ( chacknum>>8 )&0xff;
    buf[TCP_CHECKSUM_L_P] = chacknum&0xff;
    
    enc28j60PacketSend( ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN,buf );
}
void init_len_info(unsigned char *buf)
{


}




