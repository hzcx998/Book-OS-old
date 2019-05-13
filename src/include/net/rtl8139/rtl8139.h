#ifndef _NET_RTL8139_H
#define _NET_RTL8139_H

#include <sys/config.h>

#ifdef _CONFIG_NET_RTL8139_

#include <stdint.h>
#include <sys/pci.h>

#define RTL8139_MAX_MAC_NR    6

#define RTL8139_NUM_TX_BUFFER   4

#define RTL8139_RX_BUF_SIZE  	32*1024	

struct nic_rtl8139
{
	uint32  io_address;
    uint32  irq;
    uint8   mac_addr[6];
	
	struct pci_device *dev;
	
	uint8*  rx_buffer;
    uint32  current_rx;   /* CAPR, Current Address of Packet Read */
    uint32  rx_buf_len;

    uint8*  tx_buffers[RTL8139_NUM_TX_BUFFER];
    uint8   current_tx;
};

void rtl8139_init();
int32 rtl8139_get_info_from_pci();
int32 rtl8139_transmit(uint8* buf, uint32 len);
void rtl8139_handler();


int8 rtl8139_check_TSD_status();

#endif

#endif 