#ifndef _NET_ETHERNET_H
#define _NET_ETHERNET_H

#include <sys/config.h>

#ifdef _CONFIG_NET_ETHERNET_

#include <stdint.h>
#include <types.h>

#define ETH_ADDR_LEN        6

struct ethernet_hdr
{
    uint8   dest[ETH_ADDR_LEN];
    uint8   source[ETH_ADDR_LEN];
    uint16  proto;
};

int ethernet_init();
void ethernet_hdr_init(struct ethernet_hdr *eth_hdr, uint8 dst[ETH_ADDR_LEN], uint8 source[ETH_ADDR_LEN], uint16 proto);
void ethernet_set_eth_addr(uint8 eth_addr[ETH_ADDR_LEN]);
uint32 ethernet_crc(uint8* data, int32 length);
void ethernet_transmit(uint8 eth_addr[ETH_ADDR_LEN], uint16 protocol, uint8* data, uint32 len);
void ethernet_receive(uint8* data, uint32 len);
uint8* ethernet_get_addr();
uint32 ethernet_make_ipaddr(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3);
uint32 ethernet_get_ipaddr();

int ethernet_is_valid_mac_addr(uint8 mac_addr[ETH_ADDR_LEN]);
int ethernet_is_same_mac_addr(uint8 mac_addr1[ETH_ADDR_LEN], uint8 mac_addr2[ETH_ADDR_LEN]);
void ethernet_set_ip_addr(uint8 a0, uint8 a1, uint8 a2,uint8 a3);
#endif

#endif 