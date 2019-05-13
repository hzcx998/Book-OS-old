/*
File:		net/ethernet/ethernet.c
Contains:	ethernet
Auther:		Hu Zicheng
Time:		2019/4/14
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_NET_ETHERNET_


#include <string.h>

#include <sys/core.h>
#include <sys/gui.h>
#include <sys/arch.h>
#include <sys/net.h>

uint8 local_ethernet_addr[ETH_ADDR_LEN];

uint32 local_ipaddr;

int ethernet_init()
{
	if(net_buf_init() == -1){
		return -1;
	}
	return 0;
}

void ethernet_hdr_init(struct ethernet_hdr *eth_hdr, uint8 dst[ETH_ADDR_LEN], uint8 source[ETH_ADDR_LEN], uint16 proto)
{
	memcpy(eth_hdr->dest, dst, ETH_ADDR_LEN);
	memcpy(eth_hdr->source, source, ETH_ADDR_LEN);
	eth_hdr->proto = proto;
}

void ethernet_set_eth_addr(uint8 eth_addr[ETH_ADDR_LEN])
{
    memcpy(local_ethernet_addr, eth_addr, ETH_ADDR_LEN);
}

void ethernet_set_ip_addr(uint8 a0, uint8 a1, uint8 a2,uint8 a3)
{
	local_ipaddr = ethernet_make_ipaddr(a0, a1, a2, a3);
	
	uint8* ip = (uint8 *) (&local_ipaddr);
    printk("IP: %d.%d.%d.%d\n", ip[3], ip[2], ip[1], ip[0]);
}

uint32 ethernet_make_ipaddr(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3)
{
    return (uint32) ((uint32) ip0 << 24) | ((uint32) ip1 << 16) | ((uint32) ip2 << 8) | ip3;
}

void ethernet_transmit(uint8 eth_addr[ETH_ADDR_LEN], uint16 protocol, uint8* data, uint32 len)
{
	//printk("old tx len:%d  ",len);
	
    uint32 length = len;
    if (length < 46) {
        length = 46;
    }
	
	//printk("ethernet tx len:%d\n",length);
	
	/*whether send to self*/
    if (memcmp(eth_addr, local_ethernet_addr, ETH_ADDR_LEN) == 0) {
		/*if send to self, we just return now*/
		//printk("sent to self! ");
        return;
    }

    uint32 total = length + sizeof(struct ethernet_hdr);
    struct net_buf *buf = net_buf_alloc(total);
    if (buf != NULL) {
        struct ethernet_hdr eth_hdr;
		/*init ethernet header info*/
        ethernet_hdr_init(&eth_hdr, eth_addr, local_ethernet_addr, ntohs(protocol));
		
		/*init net_buf*/
        buf->data_len = total;
        buf->data = (uint8 *) buf + sizeof(struct net_buf);
        buf->ext_data = NULL;
 
		/*put eth hdr at the beginning of the net buf data*/
        uint8* p = buf->data;
        memcpy(p, &eth_hdr, sizeof(struct ethernet_hdr));
 
        p += sizeof(struct ethernet_hdr);
        memcpy(p, data, len);
		
		/*if len < 46, we fill 0 between len and 46*/
        if (length > len) {
            memset(p + len, 0, length - len);
        }
		//printk("data:%c %c ",p[0], p[1]);
		
		printk("===== transmit packet =====\n");
		
		printk("src[%x:%x:%x:%x:%x:%x]\ndest[%x:%x:%x:%x:%x:%x]\n",\
            eth_hdr.source[0], eth_hdr.source[1], eth_hdr.source[2],\
			eth_hdr.source[3], eth_hdr.source[4], eth_hdr.source[5],\
            eth_hdr.dest[0], eth_hdr.dest[1], eth_hdr.dest[2], \
			eth_hdr.dest[3], eth_hdr.dest[4], eth_hdr.dest[5]);
		printk("proto:%x\n", eth_hdr.proto);
		
		#ifdef _CONFIG_NET_RTL8139_
			/* transmit buf to net card*/
			rtl8139_transmit(buf->data, buf->data_len);
		#endif
		
		#ifdef _CONFIG_NET_AM79C970A_
			/* transmit buf to net card*/
			am79c970a_transmit(buf->data, buf->data_len);
		#endif
		
        /* free the buffer */
        net_buf_free(buf);
    } else {
		printk("ethernet: alloc net buf failed!\n");
	}
}

void ethernet_receive(uint8* data, uint32 len)
{
    struct net_buf *buf = net_buf_alloc(len);
    if (buf != NULL) {
        buf->data_len = len;
        buf->data = (uint8 *) buf + sizeof(struct net_buf);
        buf->ext_data = NULL;
        memcpy(buf->data, data, len);
 
        struct ethernet_hdr *hdr = (struct ethernet_hdr *) buf->data;
		
        printk("net receive from [%2x:%2x:%2x:%2x:%2x:%2x] to [%2x:%2x:%2x:%2x:%2x:%2x]\n",
                hdr->source[0], hdr->source[1], hdr->source[2], hdr->source[3], hdr->source[4], hdr->source[5],
                hdr->dest[0], hdr->dest[1], hdr->dest[2], hdr->dest[3], hdr->dest[4], hdr->dest[5]);
		
		/*receive from self*/
        if (memcmp(local_ethernet_addr, hdr->dest, ETH_ADDR_LEN) == 0) {
            printk("data: %s\n", (char *) buf->data + sizeof(struct ethernet_hdr));
        }
    } else {
		printk("ethernet: alloc net buf failed!\n");
	}
}

uint32 ethernet_crc(uint8* data, int32 length)
{
    static const uint32 crc_table[] = {
        0x4DBDF21C, 0x500AE278, 0x76D3D2D4, 0x6B64C2B0,
        0x3B61B38C, 0x26D6A3E8, 0x000F9344, 0x1DB88320,
        0xA005713C, 0xBDB26158, 0x9B6B51F4, 0x86DC4190,
        0xD6D930AC, 0xCB6E20C8, 0xEDB71064, 0xF0000000
    };
 
    uint32 crc = 0;
	int n;
    for (n = 0; n < length; n++) {
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[n] >> 0)) & 0x0F];  /* lower nibble */
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[n] >> 4)) & 0x0F];  /* upper nibble */
    }
 
    return crc;
}


uint8* ethernet_get_addr()
{
    return local_ethernet_addr;
}

uint32 ethernet_get_ipaddr()
{
    return local_ipaddr;
}

int ethernet_is_valid_mac_addr(uint8 mac_addr[ETH_ADDR_LEN])
{
	int i, j = 0;
	for(i = 0; i < ETH_ADDR_LEN; i++){
		if(mac_addr[i] == 0){
			j++;
		}
	}
	return (j == ETH_ADDR_LEN) ? 0:1;
}

int ethernet_is_same_mac_addr(uint8 mac_addr1[ETH_ADDR_LEN], uint8 mac_addr2[ETH_ADDR_LEN])
{
	
	int i, j = 0;
	for(i = 0; i < ETH_ADDR_LEN; i++){
		if(mac_addr1[i] == mac_addr2[i]){
			j++;
		}
	}
	return (j == ETH_ADDR_LEN) ? 1:0;
}

#endif
