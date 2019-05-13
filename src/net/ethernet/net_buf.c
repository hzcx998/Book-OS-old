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
#include <sys/gui.h>
#include <sys/net.h>
#include <sys/arch.h>
#include <sys/dev.h>

#include <string.h>

struct net_buf *net_buf_talbe[NET_MAX_BUF_NR];

int net_buf_init()
{
	
	/*get total buf number*/
	uint8 *bufs = (uint8 *)kernel_alloc_page(NET_BUF_PAGES);
	
	if(bufs == NULL){
		return -1;
	}
	int i;
	
	for(i = 0; i < NET_MAX_BUF_NR; i++){
		/*put addr*/
		net_buf_talbe[i] = (struct net_buf *)(bufs + i * NET_BUF_SIZE);
		net_buf_talbe[i]->flags = 0;
		net_buf_talbe[i]->data_len = 0;
		net_buf_talbe[i]->data = NULL;
		net_buf_talbe[i]->ext_data = NULL;
	}
	return 0;
}

struct net_buf *net_buf_alloc(uint32 len)
{
    if (len > NET_BUF_DATA_SIZE) {
        return NULL;
    }
 
    struct net_buf *buf = NULL, *tmp;

    uint32 eflags = io_load_eflags();
	io_cli();
	
	int i;
	for(i = 0; i < NET_MAX_BUF_NR; i++){
		tmp = net_buf_talbe[i];
		if(tmp->flags == 0){
			tmp->flags = 1;
			buf = tmp;
			break;
		}
	}
   
    io_store_eflags(eflags);
	
    return buf;
}

void net_buf_free(struct net_buf *net_buf)
{
    
    uint32 eflags = io_load_eflags();
	io_cli();
	
	int i;
	for(i = 0; i < NET_MAX_BUF_NR; i++){
		if(net_buf_talbe[i] == net_buf){
			net_buf->flags = 0;
			net_buf->data_len = 0;
			net_buf->data = NULL;
			net_buf->ext_data = NULL;
			break;
		}
	}
   
    io_store_eflags(eflags);
}
#endif