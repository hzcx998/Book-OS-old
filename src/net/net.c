/*
File:		net/rtl8139/rtl8139.c
Contains:	net module
Auther:		Hu Zicheng
Time:		2019/4/10
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#include <sys/core.h>
#include <sys/dev.h>
#include <sys/net.h>

#include <string.h>

struct net net;

void init_net()
{
	/*ethernet environment init*/
	net_environmente_init();
	
	/*external interface init*/
	net_interface_init();
	/*
	uint8 dst[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbe};
	char buf[64];
	while(1){
		net.transmit(dst, buf,PROTO_ARP, 64);
	}
	*/
	
	//uint8 dst[6] = {0X7c, 0X2a, 0X31, 0Xba, 0X60, 0X62};
	/*uint8 dst[6] = {0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff};
	
	printk("dst mac:%x:%x:%x:%x:%x:%x\n", dst[0], \
		dst[1], dst[2], \
		dst[3], dst[4], \
		dst[5]);
		
	char buf[64];
	memset(buf, 0, 64);
	
	strcpy(buf, "hello");
	
	char tx;
	
	uint8 *src = net.macaddr();
	ethernet_set_ip_addr(192, 168, 140,3);
	
	while(1){
		net.transmit(dst, PROTO_IP, buf, 64);
	}*/
	/*
	if(src[5] == 0X38){
		tx = 0;
		printk("won't tx buf!\n");
		ethernet_set_ip_addr(192, 168, 140,3);
		
	}else{
		tx = 1;
		printk("will tx buf!\n");
		ethernet_set_ip_addr(192, 168, 140,4);
	}*/
	/*
	net.transmit(dst, PROTO_ARP, buf, 32);
	msec_sleep(5000);
	
	net.transmit(dst, PROTO_ARP, buf, 32);
	msec_sleep(5000);
	
	net.transmit(dst, PROTO_ARP, buf, 32);
	msec_sleep(5000);*/
	
	/*
	while(1){
		if(tx){
			net.transmit(dst, PROTO_ARP, buf, 32);
		}
		//msec_sleep(5000);
		msec_sleep(5000);
	}*/
	/*
	while(1){
		
		//arp_request(ethernet_make_ipaddr(10,253,167,228));
		arp_request(ethernet_make_ipaddr(10,253,100,103));
		msec_sleep(5000);
	}*/
}

void net_environmente_init()
{
	/*ethernet layer init*/
	#ifdef _CONFIG_NET_ETHERNET_
		ethernet_init();
	#endif
	
	/*nic driver init*/
	#ifdef _CONFIG_NET_RTL8139_
		rtl8139_init();
	#endif
	
	/*nic driver init*/
	#ifdef _CONFIG_NET_AM79C970A_
		am79c970a_init();
	#endif
	#ifdef _CONFIG_NET_AM79C973_
		am79c973_init();
	#endif
	
}

void net_interface_init()
{
	/*interface set*/
	#ifdef _CONFIG_NET_ETHERNET_
		net.transmit = ethernet_transmit;
		net.macaddr = ethernet_get_addr;
	#endif
	
}

