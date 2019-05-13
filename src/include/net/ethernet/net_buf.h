#ifndef _NET_ETHERNET_NET_BUF_H
#define _NET_ETHERNET_NET_BUF_H

#include <sys/config.h>

#ifdef _CONFIG_NET_ETHERNET_

#include <stdint.h>
#include <types.h>

#define NET_BUF_SIZE        256
#define NET_BUF_DATA_SIZE   232	//256-24

#define NET_MAX_BUF_NR       1024


#define NET_BUF_PAGES       ((NET_MAX_BUF_NR*NET_BUF_SIZE)/4096)


struct net_buf
{
    uint32  data_len;
    uint8*  data;
    uint8*  ext_data;
	uint32  flags;
};

int net_buf_init();

void net_buf_free(struct net_buf *net_buf);
struct net_buf *net_buf_alloc(uint32 len);
#endif

#endif 