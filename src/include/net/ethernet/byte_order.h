#ifndef _NET_BYTE_ORDER_H
#define _NET_BYTE_ORDER_H

#include <sys/config.h>

#ifdef _CONFIG_NET_ETHERNET_

#include <stdint.h>
#include <types.h>

uint32 htonl(uint32 hostlong);	/*host to net long*/
uint16 htons(uint16 hostshort);	/*host to net short*/

uint32 ntohl(uint32 netlong);	/*net to host long*/
uint16 ntohs(uint16 netshort);	/*net to host short*/

#endif 

#endif 