/*
File:		net/am79c973/am79c973.c
Contains:	am79c973 driver
Auther:		Hu Zicheng
Time:		2019/4/15
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_NET_AM79C973_

#include <sys/core.h>
#include <sys/dev.h>
#include <sys/net.h>
#include <sys/arch.h>

#include <string.h>

/*PCI am79c973 config space register*/
#define AM79C973_VENDOR_ID   0x1022
#define AM79C973_DEVICE_ID   0x2000


/*16 bits mode*/
#define AM79C973_WIO_RDP     0x10  /*Register Data Port*/
#define AM79C973_WIO_RAP     0x12  /*Register Address Port*/
#define AM79C973_WIO_RESET   0x14	/*Reset chip*/
#define AM79C973_WIO_BDP     0x16	/*Bus Data Port*/

/*32 bits mode*/
#define AM79C973_DWIO_RDP    0x10
#define AM79C973_DWIO_RAP    0x14
#define AM79C973_DWIO_RESET  0x18
#define AM79C973_DWIO_BDP    0x1C

struct am79c973_private_s
{
	struct pci_device *pci_dev;
	
	uint32 io_address;
    uint32 irq;
    uint8  mac_addr[6];

};

struct InitializationBlock_s
{
    uint16 mode;
    unsigned reserved1 : 4;
    unsigned numSendBuffers : 4;
    unsigned reserved2 : 4;
    unsigned numRecvBuffers : 4;
    uint8   physicalAddress[6];
    uint16 reserved3;
    uint32  logicalAddress[2];

    uint32  recvBufferDescrAddress;
    uint32  sendBufferDescrAddress;
}__attribute__((packed));

struct BufferDescriptor_s
{
    uint32  address;
    uint32  flags;
    uint32  flags2;
    uint32  avali;
}__attribute__((packed));

int32 am79c973_get_info_from_pci();
struct am79c973_private_s *am79c973_get_private();
int32 am79c973_config();

void am79c973_send(uint8_t *buffer, uint32_t size);
void am79c973_receive();



struct am79c973_private_s am79c973_private;
struct InitializationBlock_s initBlock;
struct BufferDescriptor_s *sendBufferDescr, *recvBufferDescr;

uint8 currentSendBuffer, currentRecvBuffer;
uint8 isInitDone;

static uint32 am79c973CsrRead(int index)
{
    io_out32(am79c973_private.io_address + AM79C973_DWIO_RAP, index);
    return io_in32(am79c973_private.io_address + AM79C973_DWIO_RDP);
}

static void am79c973CsrWrite(int index, uint32 val)
{
    io_out32(am79c973_private.io_address + AM79C973_DWIO_RAP, index);
    io_out32(am79c973_private.io_address + AM79C973_DWIO_RDP, val);
}

static uint32 am79c973BcrRead(int index)
{
    io_out32(am79c973_private.io_address + AM79C973_DWIO_RAP, index);
    return io_in32(am79c973_private.io_address + AM79C973_DWIO_BDP);
}

static void am79c973BcrWrite(int index, uint32 val)
{
    io_out32(am79c973_private.io_address + AM79C973_DWIO_RAP, index);
    io_out32(am79c973_private.io_address + AM79C973_DWIO_BDP, val);
}

static uint32 am79c973RapRead()
{
    return io_in32(am79c973_private.io_address + AM79C973_DWIO_RAP);
}

static void am79c973RapWrite(uint32 val)
{
    io_out32(am79c973_private.io_address + AM79C973_DWIO_RAP, val);
}


int32 am79c973_get_info_from_pci()
{
	/* get pci device */
    struct pci_device* device = pci_get_device(AM79C973_VENDOR_ID, AM79C973_DEVICE_ID);
    if (device == NULL) {
        printk("am79c973 init failed: not find pci device.\n");
        return -1;
    }

    struct am79c973_private_s *pl = am79c973_get_private();

	pl->pci_dev = device;
	
    
    /* enable bus mastering */
	pci_enable_bus_mastering(device);

    /* get io address */
    pl->io_address = pci_device_get_io_addr(device);
    if (pl->io_address == 0) {
        printk("am79c973 init failed: INVALID pci device io address.\n");
        return -1;
    }
    
    /* get irq */
    pl->irq = pci_device_get_irq_line(device);
    if (pl->irq == 0xff) {
        printk("am79c973 init failed: INVALID irq.\n");
        return -1;
    }
	
    #ifdef _DEBUG_AM79C973_
        printk("find am79c973 device, vendor id: 0x%x, device id: 0x%x\n",\
                device->vendor_id, device->device_id);
        printk("am79c973 io address: 0x%x\n", pl->io_address);
        printk("am79c973 irq: %d\n", pl->irq );
    #endif
	return 0;
}

struct am79c973_private_s *am79c973_get_private()
{
    return &am79c973_private;
}


int32 am79c973_config()
{

    /*alloc ring struct buffer*/
	sendBufferDescr = (struct BufferDescriptor_s *)mm.malloc(sizeof(struct BufferDescriptor_s) *8);
    if (sendBufferDescr == NULL) {
        printk("AM79C973: memory allocation failed.\n");
		goto err_release_region;
    }
	recvBufferDescr = (struct BufferDescriptor_s *)mm.malloc(sizeof(struct BufferDescriptor_s) *8);
    if (recvBufferDescr == NULL) {
        printk("AM79C973: memory allocation failed.\n");
		goto err_release_region;
    }

    currentSendBuffer = 0;
    currentRecvBuffer = 0;
    isInitDone = 0;
    /* enable interrupt */
	put_irq_handler(am79c973_private.irq, am79c973_handler);
	enable_irq(am79c973_private.irq);
	
    /* 32 BIT MODE*/
    am79c973BcrWrite(20, 0X102);

    /* STOP set*/
    am79c973CsrWrite(0, 0x04);

    /* Init block*/
    initBlock.mode = 0x0000;
    initBlock.reserved1 = 0;
    initBlock.numSendBuffers = 3;
    initBlock.reserved2 = 0;
    initBlock.numRecvBuffers = 3;
    int i;
    for (i = 0; i < 6; i++) {
        initBlock.physicalAddress[i] = am79c973_private.mac_addr[i];
    }
    initBlock.reserved3 = 0;
    initBlock.logicalAddress[0] = 0;
    initBlock.logicalAddress[1] = 0;

    initBlock.recvBufferDescrAddress = addr_v2p((uint32 ) recvBufferDescr);
    initBlock.sendBufferDescrAddress = addr_v2p((uint32 ) sendBufferDescr);

    for (i = 0; i < 8; i++) {
        recvBufferDescr[i].address = addr_v2p((uint32 ) kernel_alloc_page(1));

        recvBufferDescr[i].flags = 0x7ff | 0xf000;

        recvBufferDescr[i].flags2 = 0;
        recvBufferDescr[i].avali = 0;
        
        sendBufferDescr[i].address = addr_v2p((uint32 ) kernel_alloc_page(1));

        sendBufferDescr[i].flags = 0xf7ff | 0x80000000;

        sendBufferDescr[i].flags2 = 0;
        sendBufferDescr[i].avali = 0;
    }

    am79c973CsrWrite(1, (uint32)(&initBlock) & 0xffff);
    am79c973CsrWrite(2, ((uint32)(&initBlock) >> 16) & 0xffff);

err_release_region:

    return 0;
}

int32 am79c973_active()
{
    /*开始初始化*/
    am79c973CsrWrite(0, 0X41);
    
    while(isInitDone == 0);
    
    /*设定数据传输信息*/
    uint32 temp = am79c973CsrRead(4);
    am79c973CsrWrite(4, temp | 0Xc00);

    /*
    #ifdef _DEBUG_AM79C973_
        printk("===== info report =====\n");
        uint32 init_block_addr = am79c973CsrRead(1) | (am79c973CsrRead(2)<<16);
        printk("init block addr:%x\n",init_block_addr);
        
        uint16 mac_addr = am79c973CsrRead( 12);
        printk("mac: %x:%x:",mac_addr&0xff, mac_addr>>8);
        mac_addr = am79c973CsrRead(13);
        printk("%x:%x:",mac_addr&0xff, mac_addr>>8);
        mac_addr = am79c973CsrRead( 14);
        printk("%x:%x\n",mac_addr&0xff, mac_addr>>8);
        
        uint32 val32 = am79c973CsrRead( 24) | (am79c973CsrRead( 25)<<16);
        printk("rx addr:%x ",val32);
        
        val32 = am79c973CsrRead( 30) | (am79c973CsrRead( 31)<<16);
        printk("tx addr:%x\n",val32);
        
        val32 = am79c973CsrRead( 76);
        printk("Receive Descriptor Ring Length:%x\n",val32);
        
        val32 = am79c973CsrRead( 78);
        printk("Transmit  Descriptor Ring Length:%x\n",val32);
    #endif
    */

    /*开始运作，打开中断*/
    am79c973CsrWrite(0, 0X42);
   
  

    return 0;
}

int32 am79c973_reset()
{
    io_in16(am79c973_private.io_address + AM79C973_WIO_RESET);
    io_out16(am79c973_private.io_address + AM79C973_WIO_RESET, 0);

	uint32 io_mode = am79c973BcrRead(18);
	if(io_mode & 0x80){
		printk("32 BITS MODE\n");
	}else{
		printk("16 BITS MODE\n");
	}
    return 0;
}


void am79c973_handler(int32 irq)
{
    #ifdef _DEBUG_AM79C973_
        printk("INTERRUPT FROM AMD am79c973\n");
    #endif

    /* GET STATUS*/
    uint32 temp = am79c973CsrRead(0);
    #ifdef _DEBUG_AM79C973_
        if ((temp&0x8000) == 0x8000) printk("AMD am79c973 ERROR\n");
        if ((temp&0x2000) == 0x2000) printk("AMD am79c973 COLLISION ERROR\n");
        if ((temp&0x1000) == 0x1000) printk("AMD am79c973 MISSED ERROR\n");
        if ((temp&0x0800) == 0x0800) printk("AMD am79c973 MEMORY ERROR\n");
        if ((temp&0x0400) == 0x0400) printk("AMD am79c973 DATA RECEIVED\n");
        if ((temp&0x0200) == 0x0200) printk("AMD am79c973 SENT\n");
    #endif

    /* ACKNOLEDGE*/
    am79c973CsrWrite(0, temp);

    if ((temp&0x0100) == 0x0100){
        #ifdef _DEBUG_AM79C973_
            printk("AMD am79c973 INIT DONE\n");
        #endif
        isInitDone = 1;
    }

}

void am79c973_send(uint8_t *buffer, uint32_t size)
{
    int sendDescriptor = currentSendBuffer;
    currentSendBuffer = (currentSendBuffer + 1)%8;

    if (size > 1518) {
        size = 1518;
    }

    uint8_t *src, *dst;
    for (src = buffer + size - 1,
        dst = (uint8_t *)(sendBufferDescr[sendDescriptor].address + size - 1);
        src >= buffer; src--, dst--)
        *dst = *src;
    
    sendBufferDescr[sendDescriptor].avali = 0;
    sendBufferDescr[sendDescriptor].flags2 = 0;
    sendBufferDescr[sendDescriptor].flags = 0x8300f000
                                            |((uint16_t )((-size)&0xfff));

    //am79c973CsrWrite(0, 0x48);

}

void am79c973_receive()
{


}


static void am79c973_test()
{

    am79c973_send((uint8_t *)"hello", 5);

}

void am79c973_init()
{
    printk("> init am79c973 start.\n");
    if(am79c973_get_info_from_pci() != 0){
        printk("am79c973 get info from pci failed!\n");
        return;
    }
    am79c973_reset();
    if(am79c973_config() != 0){
        printk("am79c973 config failed!\n");
        return;
    }
    am79c973_active();
    printk("< init am79c973 done.\n");

   am79c973_test();
}

#endif

