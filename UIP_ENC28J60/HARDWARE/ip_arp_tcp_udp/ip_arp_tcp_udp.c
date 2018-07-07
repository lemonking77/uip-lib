#include "ip_arp_tcp_udp.h"
#include "net.h"
#include "usart.h"
#include "enc28j60.h"


static uint8_t ipv4addr[4];
static uint8_t macaddr[6];
static uint16_t wwwport;
static unsigned int info_hdr_len=0; //TCP中数据偏移
static unsigned int info_data_len=0;//TCP中的数据长度
static unsigned char seqnum=0xa;    // 本机的初始序号

#define DEBUG 0

/*
 * 函数名：init_ip_arp_udp_tcp
 * 描述  ：从外部函数初始化ip地址，mac地址和端口号
 * 输入  ：mymac 本机MAC地址，myip 本机ip地址，wwwp 本地端口号
 * 输出  ：无	
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
 * 函数名：eth_type_is_arp_and_my_ip
 * 描述  ：判断接收到的以太网帧是否是ARP帧，并检查是否向本机ip发送的
 * 输入  ：buf 接收到的以太网帧数据地址，len 数据的长度（不包括以太网帧的CRC校验的4个字节）
 * 输出  ：0 不是发向本机ip地址的ARP 协议，1是发向本机的 
 */
unsigned char eth_type_is_arp_and_my_ip(unsigned char *buf,unsigned int len)
{
    uint8_t i=0;
    /* 检查接收以太网帧中的协议类型是否是ARP */
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
    /* 检查目的ip是否是本机ip */
    for( i=0;i<4;i++ )
    {
        if( buf[ETH_ARP_SRC_IP_P+i] != ipv4addr[i] )
            return 0;
    }
    
    return 1;
}

/*
 * 函数名：make_eth
 * 描述  ：修改源MAC地址和目的MAC地址
 * 输入  ：buf 接收到的以太网帧数据的存储地址
 * 输出  ：
 * 思路  ：把目的地址修改为源地址，源地址修改为本机的mac地址
 * 注意  ：以太网帧软件不需要进行软件CRC校验，因为enc28j60会硬件校验进行字节填充
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
 * 函数名：eth_type_is_ip_and_my_ip
 * 描述  ：判断接收到的以太网帧中的协议是否是IP协议，并检查是否向本机ip发送的
 * 输入  ：buf 接收到的以太网帧数据地址，len 数据的长度（不包括以太网帧的CRC校验的4个字节）
 * 输出  ：0 不符合条件，1符合条件	
 */
unsigned char eth_type_is_ip_and_my_ip(unsigned char *buf,unsigned int len)
{
    uint8_t i=0;
    /* 检查接收以太网帧中的协议类型是否是IP类型 */
    if( (buf[ETH_TYPE_H_P]!=ETHTYPE_IP_H_V) || (buf[ETH_TYPE_L_P]!=ETHTYPE_IP_L_V) )
        return 0;
    /* 检查IP报文中的协议版本ipv4和报文头长度5*4=20 */
    if( buf[IP_HEADER_LEN_VER_P]!=0x45 ) 
        return 0;
    /* 检查目的ip是否是本机ip */
    for( i=0;i<4;i++ )
    {
        if( buf[IP_DST_P+i] != ipv4addr[i] )
            return 0;
    }
    return 1;
}
/*
 * 函数名：make_arp_answer_from_request
 * 描述  ：回应ARP广播，单端传输
 * 输入  ：buf 接收到的以太网帧数据的存储地址
 * 输出  ：
 * 思路  ：把接收到的数据进行操作类型字节的修改然后原样返回，ARP报文中是没有校验和的，
           直接放在MAC帧中发出去，MAC帧的校验和由ENC28J60自动添加。
 */
void make_arp_answer_from_request(unsigned char *buf)
{
    uint8_t i;
    /* 修改操作类型为ARP回应 */
    buf[ETH_ARP_OPCODE_H_P] = ETH_ARP_OPCODE_REPLY_H_V;
    buf[ETH_ARP_OPCODE_L_P] = ETH_ARP_OPCODE_REPLY_L_V;
    
    make_eth( buf );
    /* 修改ARP协议帧中的源和目的MAC地址 */
    for( i=0;i<6;i++ )
    {
        buf[ETH_ARP_DST_IP_P+i] = buf[ETH_ARP_SRC_IP_P+i];
        buf[ETH_ARP_SRC_IP_P+i] = macaddr[i];
    }
    /* 修改ARP协议帧中的源和目的IP地址 */
    for( i=0;i<4;i++ )
    {
        buf[ETH_ARP_DST_IP_P+i] =  buf[ETH_ARP_SRC_IP_P+i];
        buf[ETH_ARP_SRC_IP_P+i] = ipv4addr[i];
    }
    /* 发送数据，ARP帧固定大小为14+28=42 */
    enc28j60WriteBuffer( 42, buf );
}
/*
 * 函数名：chack_sum
 * 描述  ：IP，TCP，UDP的校验和计算
 * 输入  ：buf 对于IP来说是IP报文首地址，对于TCP/UDP来说是从IP报文的源IP地址处，所以长度需要+8
 *         len 需要校验的数据长度，注意对于TCP/UDP来说是是要多加8的，便于循环校验，然而在计算的时候要减去8
 *         type 0：IP校验 1：UDP校验 2：TCP校验
 * 输出  ：CRC16校验和
 * 思路  ：把校验数据组合成2字节的数据进行累加校验，如果是奇数要补0，然后再把进位加回去，最后进行异或取反
 */
unsigned int chack_sum(unsigned char *buf,unsigned int len,char type)
{
    uint8_t i;
    uint64_t sum=0;
    
    if( type==1 )           /* 进行UDP校验时加上协议类型 */
    {
        sum += IP_PROTO_UDP_V;
        sum += len-8;
    }
    else if( type==2 )      /* 进行TCP校验时加上协议类型 */
    {
        sum += IP_PROTO_TCP_V;
        sum += len-8;       /* 减去8 的原因是因为我们给传过来的参数是在TCP报文长度之上+8的 */
    }                       /* 原因是要把源端和目标端的IP一起进行累加 */
    
    for( i=0;i<len-1;i+=2 ) /* 转换成2字节进行累加求和 */
    {
        sum += ( buf[i]<<8 )|buf[1+i];
    }
    if( len%2 != 0 )        /* 如果有奇数的化需要进行补位 */
        sum += ( buf[len-1]<<8 )&0xFF00;
    
    while(sum>>16)          /* 把进位的值在加回去 */
    {
        sum = ( sum&0xFFFF )+( sum>>16 ) ;
    }
    return ( uint32_t )( sum^0xFFFF );
}

/*
 * 函数名：make_ip
 * 描述  ：修改、创造出一个新的IP首部
 * 输入  ：buf 接收到的以太网帧数据的存储地址
 * 输出  ：
 * 思路  ：修改源端和目的端IP，设置标志、片偏移、生存时间、以及进行IP首部校验
 * 注意  ：以IP报文需要进行软件CRC校验，校验的是IP报文首部
 */
void make_ip(unsigned char *buf)
{
    uint8_t i;
    uint32_t chacksum = 0;
    /* 校验和清0 */
    buf[IP_CHECKSUM_P] = 0;
    buf[IP_CHECKSUM_P+1] = 0;
    
    /* 设置标志和偏移 */
    buf[IP_FLAGS_P] = 0x40; //设置标志位不能对报文分片，且是最后一个报文
    buf[IP_FLAGS_P+1] = 0;  //偏移位0
    /* 设置生存时间TTL */
    buf[IP_TTL_P] = 64;
    /* 修改IP */
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
 * 函数名：make_echo_reply_from_request
 * 描述  ：ICMP的ping命令回应
 * 输入  ：buf 接收到的以太网帧数据的存储地址
 * 输出  ：
 * 思路  ：把接收到的数据进行修改然后原样返回
 */
void make_echo_reply_from_request(unsigned char *buf,unsigned int len)
{

    make_eth(buf);
    make_ip(buf);
    /* 修改ICMP的回显类型 */
    buf[ICMP_TYPE_P] = ICMP_TYPE_ECHOREPLY_V;
    
    if( buf[ICMP_CHECKSUM_P]>(0xff-0x08) )  //这里的0x08是高字节，所以要和校验和的高字节比较
    {
        buf[ICMP_CHECKSUM_P+1]++;           //实现进位
    }
    buf[ICMP_CHECKSUM_P] += 0x08;           //否则就直接把校验高位+8
    enc28j60PacketSend( len, buf);
    
}

void make_udp_reply_from_request(unsigned char *buf,char *data,unsigned int datalen,unsigned int port)
{
    
}

/*
 * 函数名：make_tcphead
 * 描述  ：创建TCP报首中的一部分(这一部分是在任何TCP报文都会改变的)
 * 输入  ：buf 接收到的以太网帧数据的存储地址
           rel_ack_num 确认号
           mss 是否使用可选字段 0：不用 1：用
           cp_seq 序号
 * 输出  ：
 * 思路  ：把接收到的数据进行修改然后原样返回
 */
void make_tcphead(unsigned char *buf,unsigned  int rel_ack_num,unsigned char mss,unsigned char cp_seq)
{
    uint8_t i;
    uint16_t tcp_seq;
    
        /* 校验和清0 */
    buf[TCP_CHECKSUM_H_P] = 0;
    buf[TCP_CHECKSUM_L_P] = 0;
    
    /* 修改目的端口 */
    buf[TCP_DST_PORT_L_P] = buf[TCP_SRC_PORT_L_P];
    buf[TCP_DST_PORT_H_P] = buf[TCP_SRC_PORT_H_P];
    /* 修改源端口 */
    buf[TCP_SRC_PORT_L_P] = wwwport&0xff;
    buf[TCP_SRC_PORT_H_P] = wwwport>>8;

    /* 
    序号：TCP会隐式的对TCP的数据区中的每个字节进行编号，那么这个序号就是我要发送的这个TCP报文片在整个报文中的字节偏移；
          举例：要给别人发一个报文3000字节，第一次发送1500个字节，序号是0，那么第二次发送序号就是1500。这个序号是用来
                表示字节报文片在整个报文中的字节偏移。
    确认号：确认号是用来回应别人是否收到数据，同时用来表明期待收到对方下一个TCP数据报数据区首字节的序号；
           举例：对方先给我发了一个100字节的报文，对方报文中的序列号为50，那么下一个我们期望收到额报文
                 序号就是50+100；
    */
    
    for( i=4;i>0;i-- )
    {
        rel_ack_num =  buf[TCP_SEQ_H_P+i-1]+rel_ack_num;/* 获取发来的确认号+接收的数据长度=确认号(对方下一次发来的TCP数据区首字节编号) */
        tcp_seq = buf[TCP_SEQACK_H_P+i-1];              /* 获取发来的确认号可能是字节的序号奥 */
        buf[TCP_SEQACK_H_P+i-1] = rel_ack_num&0xff;
        rel_ack_num >>= 8;
        if( cp_seq )    /* 别人发过来的应答号就是自己的序号所以直接拷贝就可以了 */
        {
            buf[TCP_SEQ_H_P+i-1] = tcp_seq; 
        }
        else{            /* 序号清0，使用默认序号 */
            buf[TCP_SEQ_H_P+i-1] = 0;
        }
    }
    if( cp_seq==0 )/* 如果不使用对方发来的确认号作为序号的化就使用初始序号 */
    {
        /* 设置自身默认序号 */
        buf[TCP_SEQ_H_P+0] = 0;
        buf[TCP_SEQ_H_P+1] = 0;
        buf[TCP_SEQ_H_P+2] = seqnum;
        buf[TCP_SEQ_H_P+3] = 0;
        seqnum += 2;
    }
    if( mss )/* 这里对TCP的可选项只设置了最大报文段长度=MTU-40 */
    {
        buf[TCP_OPTIONS_P+0] = 2;
        buf[TCP_OPTIONS_P+1] = 4;
        buf[TCP_OPTIONS_P+2] = 1408&0xff;
        buf[TCP_OPTIONS_P+3] = 1408>>8;
        
        /* 设置数据区偏移(TCP报头长度) 6*4=24字节 最大为15*4=60字节*/
        buf[TCP_HEADER_LEN_P] &= 0x60;
    }
    else/* 默认TCP报头大小为5*4=20字节 */
        buf[TCP_HEADER_LEN_P] &= 0x50;
}
/*
 * 函数名：make_tcp_synack_from_syn
 * 描述  ：回应TCP连接请求
 * 输入  ：buf 接收到的以太网帧数据的存储地址
 * 输出  ：
 * 思路  ：
 */
void make_tcp_synack_from_syn(unsigned char *buf)
{
    uint32_t chacknum=0;
    
    make_eth(buf);
    /* 设置IP的数据总长度 = 20(ip_h)+tp_h(20)+tp_op(4) */
    buf[IP_TOTLEN_H_P] = 0;
    buf[IP_TOTLEN_L_P] = IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4;
    
    make_ip(buf);
    
    /* 设置TCP的标志位 ACK+SYN */
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




