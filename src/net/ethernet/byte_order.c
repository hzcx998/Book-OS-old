/*
File:		net/rtl8139/rtl8139.c
Contains:	net module
Auther:		Hu Zicheng
Time:		2019/4/10
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_NET_ETHERNET_

#include <sys/net.h>

#define BIG_LITTLE_SWAP16(A) ((((uint16)(A) & 0xff00) >> 8) | \
							(((uint16)(A) & 0x00ff) << 8))
#define BIG_LITTLE_SWAP32(A)  ((((uint32)(A) & 0xff000000) >> 24) | \
							(((uint32)(A) & 0x00ff0000) >> 8) | \
							(((uint32)(A) & 0x0000ff00) << 8) | \
							(((uint32)(A) & 0x000000ff) << 24))		
							
uint32 htonl(uint32 hostlong)
{
	#ifdef _CONFIG_BYTE_ORDER_BE_
		/*
		if host is big endian, it is same with net work byte order,
		we just return it
		*/
		return hostlong;
	#endif
	
	#ifdef _CONFIG_BYTE_ORDER_LE_
		/*if host is little endian, we need swap it*/
		return BIG_LITTLE_SWAP32(hostlong);
	#endif
}

uint16 htons(uint16 hostshort)
{
	#ifdef _CONFIG_BYTE_ORDER_BE_
		/*
		if host is big endian, it is same with net work byte order,
		we just return it
		*/
		return hostshort;
	#endif
	
	#ifdef _CONFIG_BYTE_ORDER_LE_
		/*if host is little endian, we need swap it*/
		return BIG_LITTLE_SWAP16(hostshort);
	#endif
}

uint32 ntohl(uint32 netlong)
{
	#ifdef _CONFIG_BYTE_ORDER_BE_
		/*
		if host is big endian, it is same with net work byte order,
		we just return it
		*/
		return netlong;
	#endif
	
	#ifdef _CONFIG_BYTE_ORDER_LE_
		/*if host is little endian, we need swap it*/
		return BIG_LITTLE_SWAP32(netlong);
	#endif
}

uint16 ntohs(uint16 netshort)
{
	#ifdef _CONFIG_BYTE_ORDER_BE_
		/*
		if host is big endian, it is same with net work byte order,
		we just return it
		*/
		return netshort;
	#endif
	
	#ifdef _CONFIG_BYTE_ORDER_LE_
		/*if host is little endian, we need swap it*/
		return BIG_LITTLE_SWAP16(netshort);
	#endif
}
#endif
