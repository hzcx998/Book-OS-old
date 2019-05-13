/*
File:		net/rtl8139/rtl8139.c
Contains:	rtl8139 driver
Auther:		Hu Zicheng
Time:		2019/4/10
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_NET_RTL8139_

#include <sys/core.h>
#include <sys/dev.h>
#include <sys/gui.h>
#include <sys/net.h>
#include <string.h>

/*PCI rtl8139 config space register*/
#define RTL8139_VENDOR_ID   0x10ec
#define RTL8139_DEVICE_ID   0x8139

/*rtl8139 register*/
#define RTL8139_MAC         0x00
#define RTL8139_TX_STATUS0  0x10
#define RTL8139_TX_ADDR0    0x20
#define RTL8139_RX_BUF_ADDR 0x30
#define RTL8139_COMMAND     0x37
#define RTL8139_CAPR        0x38
#define RTL8139_CBR         0x3A

#define RTL8139_INTR_MASK   0x3C
#define RTL8139_INTR_STATUS 0x3E
#define RTL8139_TCR         0x40
#define RTL8139_RCR         0x44
#define RTL8139_9346CR		0x50

#define RTL8139_CONFIG_0    0x51
#define RTL8139_CONFIG_1    0x52

/*rtl8139 intrrupt status register*/
#define RTL8139_INTR_STATUS_ROK   	  	0x0001
#define RTL8139_INTR_STATUS_RER  		0x0002
#define RTL8139_INTR_STATUS_TOK     	0x0004
#define RTL8139_INTR_STATUS_TER			0x0008
#define RTL8139_INTR_STATUS_RXOVW		0x0010
#define RTL8139_INTR_STATUS_PUN 		0x0020 
#define RTL8139_INTR_STATUS_FOVW 		0x0040 
/*reserved*/
#define RTL8139_INTR_STATUS_LENCHG		0x2000 
#define RTL8139_INTR_STATUS_TIMEOUT 	0x4000
#define RTL8139_INTR_STATUS_SERR   		0x8000 

/*rtl8139 command register*/
#define RTL8139_CMD_BUFE   	0x01	/*[R] Buffer Empty*/
#define RTL8139_CMD_TE  	0x04	/*[R/W] Transmitter Enable*/
#define RTL8139_CMD_RE  	0x08	/*[R/W] Receiver Enable*/
#define RTL8139_CMD_RST  	0x10	/*[R/W] Reset*/

/*rtl8139 receive config register*/
#define RTL8139_RCR_AAP   	0x01	/*[R/W] Accept All Packets*/
#define RTL8139_RCR_APM   	0x02	/*[R/W] Accept Physical Match packets*/
#define RTL8139_RCR_AM   	0x04	/*[R/W] Accept Multicast packets*/
#define RTL8139_RCR_AB   	0x08	/*[R/W] Accept Broadcast packets*/
#define RTL8139_RCR_AR   	0x10	/*[R/W] Accept Broadcast packets*/
#define RTL8139_RCR_AER   	0x20	/*[R/W] Accept Broadcast packets*/


#define RTL8139_RCR_RBLEN_32K   	2	/*[R/W] 2 means 32k + 16 byte*/
#define RTL8139_RCR_RBLEN_SHIFT   	11
	
#define RTL8139_RCR_MXDMA_1024K   	0x06		// 110b
#define RTL8139_RCR_MXDMA_SHIFT 	8

#define RTL8139_RCR_RXFTH_NOMAL   	0x07		// 111b
#define RTL8139_RCR_RXFTH_SHIFT 	13

/*rtl8139 transmit config register*/
#define RTL8139_TCR_MXDMA_1024K  	0x06		// 110b
#define RTL8139_TCR_MXDMA_SHIFT  	8

#define RTL8139_TCR_RXFTH_NORMAL  	0x03	// 11b
#define RTL8139_TCR_RXFTH_SHIFT 	24

/*RTL8139 TSD register*/
#define RTL8139_TSD_OWN   0x2000		/*[R/W] OWN*/
#define RTL8139_TSD_TUN   0x4000		/*[R] Transmit FIFO Underrun*/
#define RTL8139_TSD_TOK   0x8000		/*[R] Transmit OK*/
#define RTL8139_TSD_NCC   0x1000000		/*[R/W] NCC*/
#define RTL8139_TSD_CDH   0x10000000	/*[R] CD Heart Beat*/
#define RTL8139_TSD_OWC   0x20000000	/*[R] Out of Window Collision*/
#define RTL8139_TSD_TABT  0x40000000	/*[R] Transmit Abort*/
#define RTL8139_TSD_CRS   0x80000000	/*[R] Carrier Sense Lost*/

/*RTL8139 9346CMD register*/
#define RTL8139_9346CMD_EEM_11  0XC0	

/*RTL8139 Tranmit configuration register*/
#define RTL8139_TCR_MXDMA_UNLIMITED  	0X07	
#define RTL8139_TCR_IFG_NORMAL  		0X07	

#define RTL8139_ETH_ZLEN  		60	

struct nic_rtl8139 rtl8139;

void rtl8139_tx_interrupt();
void rtl8139_rx_interrupt();

void rtl8139_init()
{
	printk(">init rtl8139\n");
	
	if(rtl8139_get_info_from_pci() != 0){
		printk("rtl8139: get info from pci failed!\n");
		return;
	}
	
	/*Unlock config register*/
	io_out8(rtl8139.io_address + RTL8139_9346CR, RTL8139_9346CMD_EEM_11);
	
	/*
	send 0x00 to CONFIG_1 register to set the LWAKE + LWPTN to active high, 
    * this should essentially power on the device 
	(we should make rtl8139 work by this way)
	*/
	/*we need reset driver before we transmit or receive a packet*/
	
    //io_out8(rtl8139.io_address + RTL8139_CONFIG_1, 0x00);
	
	/*software reset, to clear the RX and TX buffers and set everything back to defaults */
    
	/*get cmd bufe, if it is 1, it means rx buffer is empty*/
	uint8 cmd_bufe = io_in8(rtl8139.io_address + RTL8139_COMMAND);
	cmd_bufe &= RTL8139_CMD_BUFE;
	io_out8(rtl8139.io_address + RTL8139_COMMAND, cmd_bufe | RTL8139_CMD_RST);
	/*wait reset done*/
    while ((io_in8(rtl8139.io_address + RTL8139_COMMAND) & RTL8139_CMD_RST) != 0) {}
	
	/* mac address */
	int i;
    for (i = 0; i < RTL8139_MAX_MAC_NR; i++) {
        rtl8139.mac_addr[i] = io_in8(rtl8139.io_address + RTL8139_MAC + i);
    }
	
    printk("mac addr: %x:%x:%x:%x:%x:%x\n", rtl8139.mac_addr[0], rtl8139.mac_addr[1],\
            rtl8139.mac_addr[2], rtl8139.mac_addr[3], rtl8139.mac_addr[4], rtl8139.mac_addr[5]);
	
	#ifdef _CONFIG_NET_ETHERNET_
		ethernet_set_eth_addr(rtl8139.mac_addr);
		
	#endif
	
	/* 
    * set IMR(Interrupt Mask Register)
    * To set the RTL8139 to accept only the Transmit OK (TOK) and Receive OK (ROK) interrupts, 
    * we would have the TOK and ROK bits of the IMR high and leave the rest low. 
    * That way when a TOK or ROK IRQ happens, it actually will go through and fire up an IRQ.
    */
	/*we just receive RX and TX interrupts*/
    //io_out16(rtl8139.io_address + RTL8139_INTR_MASK, RTL8139_INTR_STATUS_ROK | RTL8139_INTR_STATUS_TOK);
	io_out16(rtl8139.io_address + RTL8139_INTR_MASK, 0XFF);
	
	
	/*init transmit buffer */
	for (i = 0; i < RTL8139_NUM_TX_BUFFER; i++) {
		/*alloc 8kb for every transmit buffer*/
        rtl8139.tx_buffers[i] = (uint8 *) kernel_alloc_page(2);	
    }
    rtl8139.current_tx = 0;
	
	/*init receive buffer */
    rtl8139.rx_buffer = (uint8 *) kernel_alloc_page(9);	/*alloc 32+4(reserved) kb for receive buffer*/
    rtl8139.current_rx = 0;		/*set CRPR to 0*/
    rtl8139.rx_buf_len = RTL8139_RX_BUF_SIZE;	/*we have 32 kb data*/
	
	/*transmit configuration*/
	for (i = 0; i < RTL8139_NUM_TX_BUFFER; i++) {
		io_out32(rtl8139.io_address + RTL8139_TX_ADDR0 + i * 4, addr_v2p((uint32)rtl8139.tx_buffers[i]));
		//printk("tx buf phy:%x ",io_in32(rtl8139.io_address + RTL8139_TX_ADDR0 + i * 4));
	}
	
	/* TxConfig = IFG: normal, MXDMA: 1024 */
	io_out32(rtl8139.io_address + RTL8139_TCR, \
		(RTL8139_TCR_RXFTH_NORMAL<<RTL8139_TCR_RXFTH_SHIFT)|\
		(RTL8139_TCR_MXDMA_1024K<<RTL8139_TCR_MXDMA_SHIFT));
	
	/*receive configuration*/
	/*we need set phy addr to buffer address start*/
    io_out32(rtl8139.io_address + RTL8139_RX_BUF_ADDR, addr_v2p((uint32 )rtl8139.rx_buffer));
	/*set CAPR to register*/
    io_out16(rtl8139.io_address + RTL8139_CAPR, rtl8139.current_rx);
	/* 
    * configuring receive buffer(RCR)
    * Before hoping to see a packet coming to you, you should tell the RTL8139 to accept packets 
    * based on various rules. The configuration register is RCR.
    *  AB - Accept Broadcast: Accept broadcast packets sent to mac ff:ff:ff:ff:ff:ff
    *  AM - Accept Multicast: Accept multicast packets.
    *  APM - Accept Physical Match: Accept packets send to NIC's MAC address.
    *  AAP - Accept All Packets. Accept all packets (run in promiscuous mode).
    *  Another bit, the WRAP bit, controls the handling of receive buffer wrap around.
    */
	io_out32(rtl8139.io_address + RTL8139_RCR,\
		(RTL8139_RCR_RBLEN_32K << RTL8139_RCR_RBLEN_SHIFT)|\
		(RTL8139_RCR_RXFTH_NOMAL<<RTL8139_RCR_RXFTH_SHIFT)|\
		(RTL8139_RCR_MXDMA_1024K<<RTL8139_RCR_MXDMA_SHIFT)|\
		(RTL8139_RCR_AR | RTL8139_RCR_AER) |\
		(RTL8139_RCR_AB|RTL8139_RCR_AM|RTL8139_RCR_APM|RTL8139_RCR_AAP)); 
		
    
	/* enable receive and transmitter */
	cmd_bufe = io_in8(rtl8139.io_address + RTL8139_COMMAND);
	cmd_bufe &= RTL8139_CMD_BUFE;
	io_out8(rtl8139.io_address + RTL8139_COMMAND, cmd_bufe | RTL8139_CMD_TE|RTL8139_CMD_RE);
    
	/* enable interrupt */
	put_irq_handler(rtl8139.irq, rtl8139_handler);
	enable_irq(rtl8139.irq);
	
	/*Lock config register*/
	//io_out8(rtl8139.io_address + RTL8139_9346CR, 0);
	/*
	char buf[100];
	
	while(1){
		rtl8139_transmit(buf, 100);

	}
	
	printk(">init rtl8139 done!\n");
	
	while(1);
	*/
}

int32 rtl8139_get_info_from_pci()
{
    /* get pci device */
    struct pci_device* device = pci_get_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (device == NULL) {
        printk("RTL8139 init failed: not find pci device.\n");
        return -1;
    }
	rtl8139.dev = device;
	
    printk("find rtl8139 device, vendor id: 0x%x, device id: 0x%x\n",\
            device->vendor_id, device->device_id);

    /* enable bus mastering */
	pci_enable_bus_mastering(device);

    /* get io address */
    rtl8139.io_address = pci_device_get_io_addr(device);
    if (rtl8139.io_address == 0) {
        printk("RTL8139 init failed: INVALID pci device io address.\n");
        return -1;
    }
    printk("rlt8139 io address: 0x%x\n", rtl8139.io_address);

    /* get irq */
    rtl8139.irq = pci_device_get_irq_line(device);
    if (rtl8139.irq == 0xff) {
        printk("RTL8139 init failed: INVALID irq.\n");
        return -1;
    }
	
    printk("rlt8139 irq: %d\n", rtl8139.irq );

    return 0;
}

int32 rtl8139_transmit(uint8* buf, uint32 len)
{
	/*judge data len*/
	if( len < RTL8139_ETH_ZLEN ){//if len < 60  
		memset(buf + len, 0x20, (RTL8139_ETH_ZLEN - len) );  
		len = RTL8139_ETH_ZLEN;
	}
	/*get status*/
	//uint32 status = io_in16(rtl8139.io_address + RTL8139_INTR_STATUS);
	
    /*printk("transmit TX_STATUS0: 0x%8x, INTR_STATUS: %x\n",\
            io_in32(rtl8139.io_address + RTL8139_TX_STATUS0), status);
	*/
	
	uint32 eflags = io_load_eflags();
	io_cli();
	/*copy data to buffer*/
    memcpy(rtl8139.tx_buffers[rtl8139.current_tx], buf, len);
    
	/*set transmit status, after set this, driver will work*/
	io_out32(rtl8139.io_address + RTL8139_TX_STATUS0 + rtl8139.current_tx * 4,\
		RTL8139_TSD_NCC | 0 | (len&0X1FFF));
    
	rtl8139.current_tx = (rtl8139.current_tx + 1) % RTL8139_NUM_TX_BUFFER;
	//printk("after transmit TX_STATUS0: 0x%8x\n", io_in32(rtl8139.io_address + RTL8139_TX_STATUS0));
	
	io_store_eflags(eflags);
    return 0;
}

void rtl8139_handler()
{
    printk("interrupt, ");

    uint32 status = io_in16(rtl8139.io_address + RTL8139_INTR_STATUS);
	/*we must write it back to tell driver that we had handled interrupts.*/
    io_out16(rtl8139.io_address + RTL8139_INTR_STATUS, status);
	
	if ((status & (RTL8139_INTR_STATUS_ROK | RTL8139_INTR_STATUS_RER | \
		RTL8139_INTR_STATUS_TOK | RTL8139_INTR_STATUS_TER | \
		RTL8139_INTR_STATUS_RXOVW | RTL8139_INTR_STATUS_PUN | \
		RTL8139_INTR_STATUS_TIMEOUT | RTL8139_INTR_STATUS_SERR)) == 0) 
	{
		return;
		
	} else if (status & (RTL8139_INTR_STATUS_ROK | RTL8139_INTR_STATUS_RXOVW | \
		RTL8139_INTR_STATUS_PUN | RTL8139_INTR_STATUS_FOVW)) 
	{
		rtl8139_rx_interrupt ();
		
    } else if (status & (RTL8139_INTR_STATUS_TOK | RTL8139_INTR_STATUS_TER)) {
		/*lock*/
		rtl8139_tx_interrupt ();
		/*unlock*/
    } else {
        printk("0x%8x\n", status);
    }
	
    printk("\n");
}
void rtl8139_rx_interrupt()
{
    printk("RXOK, ");
    uint8* rx = rtl8139.rx_buffer;
    uint32 cur = rtl8139.current_rx;

	/*read rx buf data if fifo not empty*/
    while (!(io_in8(rtl8139.io_address + RTL8139_COMMAND) & RTL8139_CMD_BUFE)) {
        uint32 offset = cur % rtl8139.rx_buf_len;
        uint8* buf = rx + offset;
		
		/*first 2 bytes is status(The Packet Header)*/
        uint32 status = *(uint32 *) (buf);
		/*second 2 bytes is size*/
        uint32 size   = status >> 16;

        //printk("[0x%x, 0x%x] | ", offset, size);
		/*int i;
		for (i = 0; i < 16; i++) {
			printk("%2x ", rx[4 + i + offset]);
		}*/

		/*save data*/
		//os()->get_net()->receive(buf + 4, size - 4);
		#ifdef _CONFIG_NET_ETHERNET_
		
			ethernet_receive(buf + 4, size - 4);
			
		#endif
		/*align to 4*/
        cur = (cur + size + 7) & ~3;
        io_out16(rtl8139.io_address + RTL8139_CAPR, cur - 16);
    }

    rtl8139.current_rx = cur;
}

void rtl8139_tx_interrupt()
{
	printk("TXOK, ");
    
	/*get last transmit buffer*/
	uint8 cur_tx = ((rtl8139.current_tx - 1) < 0)? (RTL8139_NUM_TX_BUFFER-1): (rtl8139.current_tx - 1);
	
	/*get tx status*/
	uint32 tx_status = io_in32(rtl8139.io_address + RTL8139_TX_STATUS0 + cur_tx*4);
	printk("TX_STATUS[%d]: 0x%x\n", cur_tx, tx_status);
	
	/*set tx size to 0*/
	tx_status &= ~0x1fffUL;
	/*write to status*/
	io_out32(rtl8139.io_address + RTL8139_TX_STATUS0 + cur_tx*4, tx_status);
	//printk(" TX_STATUS: 0x%x\n", tx_status);
}

int8 rtl8139_check_TSD_status()
{
	uint16 tmpTSD = io_in32(rtl8139.io_address + RTL8139_TX_STATUS0);
	
	if(tmpTSD&RTL8139_TSD_OWN){
		printk("OWN ");
	}
	if(tmpTSD&RTL8139_TSD_TUN){
		printk("TUN ");
	}
	if(tmpTSD&RTL8139_TSD_TOK){
		printk("TOK ");
	}
	if(tmpTSD&RTL8139_TSD_CDH){
		printk("CDH ");
	}
	if(tmpTSD&RTL8139_TSD_OWC){
		printk("OWC ");
	}
	if(tmpTSD&RTL8139_TSD_TABT){
		printk("TABT ");
	}
	if(tmpTSD&RTL8139_TSD_CRS){
		printk("CRS ");
	}
	return -1;
}

#endif

