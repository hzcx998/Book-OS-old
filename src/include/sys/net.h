#ifndef _NET_H
#define _NET_H

#include <sys/config.h>
#include <stdint.h>

#define ETHERNET_IRQ 11

/* protocols */
#define PROTO_IP            0x0800
#define PROTO_ARP           0x0806

#ifdef _CONFIG_NET_ETHERNET_
	/*head file*/
	#include <net/ethernet/ethernet.h>
	#include <net/ethernet/net_buf.h>
	#include <net/ethernet/byte_order.h>
	#include <net/ethernet/arp.h>
	
#endif

/*if define rtl8139, we will compelete it*/
#ifdef _CONFIG_NET_RTL8139_
	
	#include <net/rtl8139/rtl8139.h>
	
#endif

/*if define am79c970a, we will compelete it*/
#ifdef _CONFIG_NET_AM79C970A_
	
	#include <net/am79c970a/am79c970a.h>
	
#endif

/*if define am79c973, we will compelete it*/
#ifdef _CONFIG_NET_AM79C973_
	
	#include <net/am79c973/am79c973.h>
	
#endif



struct net
{
	void (*transmit)(uint8 *eth_addr, uint16 protocol, uint8* data, uint32 len);
	uint8 *(*macaddr)();
	
};

extern struct net net;

void init_net();
void net_environmente_init();
void net_interface_init();

void IRQ_ethernet();


#endif