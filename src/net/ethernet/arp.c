/*
File:		net/ethernet/net_buf.c
Contains:	ethernet buf
Auther:		Hu Zicheng
Time:		2019/4/14
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_NET_ETHERNET_

#include <sys/core.h>
#include <string.h>

#include <sys/net.h>

void arp_hdr_init(struct arp_hdr *arp_hdr, uint16 hw_type, uint16 proto_type, uint8 hw_len, uint8 proto_len,
        uint16 opcode, uint8 src_hw_addr[ETH_ADDR_LEN], uint32 src_proto_addr,
        uint8 tgt_hw_addr[ETH_ADDR_LEN], uint32 tgt_proto_addr)
{
    arp_hdr->hardware_type = hw_type;
    arp_hdr->protocol_type = proto_type;
    arp_hdr->hardware_len  = hw_len;
    arp_hdr->protocol_len  = proto_len;
    arp_hdr->opcode        = opcode;
    arp_hdr->source_protocol_addr = src_proto_addr;
    arp_hdr->target_protocol_addr = tgt_proto_addr;
 
    memcpy(arp_hdr->source_hw_addr, src_hw_addr, ETH_ADDR_LEN);
    memcpy(arp_hdr->target_hw_addr, tgt_hw_addr, ETH_ADDR_LEN);
}

void arp_request(uint32 ip)
{
    static uint8 empty_mac_addr[ETH_ADDR_LEN] = { 0x0 };
    static uint8 broadcast_mac_addr[ETH_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
 
    struct arp_hdr hdr;
    arp_hdr_init(&hdr, 
			 ntohs(0x0001),                                 /* ethernet */
             ntohs(PROTO_IP),                               	/* IPv4 */
             0x06,                                          /* ethernet addr len */
             0x04,                                          /* IPv4 len */
             ntohs(ARP_OP_REQUEST),                         /* op */
             ethernet_get_addr(),   						/* my mac addr */
             ethernet_get_ipaddr(),         				/* my ip addr */
             empty_mac_addr,                                /* 0 */
             ntohl(ip)                                      /* which IP I want to know it's mac addr */
    );
 
    uint32 len = sizeof(struct arp_hdr );
    struct net_buf* buf = net_buf_alloc(len);
    if (buf != NULL) {
        buf->data_len = len;
        buf->data = (uint8 *) buf + sizeof(struct net_buf);
        buf->ext_data = NULL;
        memcpy(buf->data, &hdr, len);
 
        net.transmit(broadcast_mac_addr, PROTO_ARP, 
                buf->data, buf->data_len);
        net_buf_free(buf);
    }
}



#endif