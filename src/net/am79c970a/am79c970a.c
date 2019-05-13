/*
File:		net/am79c970a/am79c970a.c
Contains:	am79c970a driver
Auther:		Hu Zicheng
Time:		2019/4/15
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_NET_AM79C970A_

#include <sys/core.h>
#include <sys/dev.h>
#include <sys/net.h>
#include <sys/arch.h>

#include <string.h>

/*PCI am79c970a config space register*/
#define AM79C970A_VENDOR_ID   0x1022
#define AM79C970A_DEVICE_ID   0x2000

/*maybe 0x2001 is ok*/
//#define AM79C970A_DEVICE_ID   0x2001

/*
 * Set the number of Tx and Rx buffers, using Log_2(# buffers).
 * Reasonable default values are 4 Tx buffers, and 16 Rx buffers.
 * That translates to 2 (4 == 2^^2) and 4 (16 == 2^^4).
 */
 
#define AM79C970A_PORT_AUI      0x00
#define AM79C970A_PORT_10BT     0x01
#define AM79C970A_PORT_GPSI     0x02
#define AM79C970A_PORT_MII      0x03

#define AM79C970A_PORT_PORTSEL  0x03
#define AM79C970A_PORT_ASEL     0x04
#define AM79C970A_PORT_100      0x40
#define AM79C970A_PORT_FD       0x80

#define AM79C970A_DMA_MASK 0xffffffff
 
#ifndef AM79C970A_LOG_TX_BUFFERS
#define AM79C970A_LOG_TX_BUFFERS      4
#define AM79C970A_LOG_RX_BUFFERS      5
#define am79c970a_LOG_MAX_TX_BUFFERS  9   /* 2^9 == 512 */
#define am79c970a_LOG_MAX_RX_BUFFERS  9
#endif

#define TX_RING_SIZE        (1 << (AM79C970A_LOG_TX_BUFFERS))
#define TX_MAX_RING_SIZE    (1 << (am79c970a_LOG_MAX_TX_BUFFERS))

#define RX_RING_SIZE        (1 << (AM79C970A_LOG_RX_BUFFERS))
#define RX_MAX_RING_SIZE    (1 << (am79c970a_LOG_MAX_RX_BUFFERS))

#define PKT_BUF_SKB     1548
/* actual buffer length after being aligned */
#define PKT_BUF_SIZE        (PKT_BUF_SKB - NET_IP_ALIGN)
/* chip wants twos complement of the (aligned) buffer length */
#define NEG_BUF_SIZE        (NET_IP_ALIGN - PKT_BUF_SKB)

/*Offsets from base I/O address. */

/*16 bits mode*/
#define AM79C970A_WIO_RDP     0x10  /*Register Data Port*/
#define AM79C970A_WIO_RAP     0x12  /*Register Address Port*/
#define AM79C970A_WIO_RESET   0x14	/*Reset chip*/
#define AM79C970A_WIO_BDP     0x16	/*Bus Data Port*/

/*32 bits mode*/
#define AM79C970A_DWIO_RDP    0x10
#define AM79C970A_DWIO_RAP    0x14
#define AM79C970A_DWIO_RESET  0x18
#define AM79C970A_DWIO_BDP    0x1C

#define CSR0        0
#define CSR0_INIT   0x1
#define CSR0_START  0x2
#define CSR0_STOP   0x4
#define CSR0_TXPOLL 0x8
#define CSR0_INTEN  0x40
#define CSR0_IDON   0x0100
#define CSR0_TINT   0x0200
#define CSR0_RINT   0x0400

#define CSR0_NORMAL (CSR0_START | CSR0_INTEN)

#define AM79C970A_INIT_LOW    1
#define AM79C970A_INIT_HIGH   2

#define CSR1        1
#define CSR2        2	
#define CSR3        3	/*Interrupt Masks and Deferral Control*/
#define CSR4        4	
#define CSR5        5
#define CSR5_SUSPEND    0x0001
#define CSR12	12
#define CSR13	13
#define CSR14	14
#define CSR15   15
#define CSR24	24
#define CSR25	25
#define CSR26	26

#define CSR30	30
#define CSR31	31
#define CSR41   41	/*Current Receive Status*/
#define CSR76	76
#define CSR78	78

#define AM79C970A_MC_FILTER   8


#define BCR20       20
#define BCR20_SSIZE       0X100		/*indicate 1（32bit） 0（16bits） */


struct am79c970a_access 
{
    u32  (*read_csr)  (uint32, int);
    void (*write_csr) (uint32, int, uint32);
    u32  (*read_bcr)  (uint32, int);
    void (*write_bcr) (uint32, int, uint32);
    u32  (*read_rap)  (uint32);
    void (*write_rap) (uint32, uint32);
    void (*reset)     (uint32);
};

/* The AM79C970A Rx and Tx ring descriptors. */
struct am79c970a_rx_head 
{
    uint32  base;///存储该描述符对应的缓冲区的首地址
    uint16  buf_length; /* two`s complement of length */ ///二进制补码形式，缓冲区大小
    uint16  status;////每一位都有自己的含义，硬件手册有规定
    uint32  msg_length;
    uint32  reserved;
};

struct am79c970a_tx_head 
{
    uint32  base;
    uint16  length;     /* two`s complement of length */
    uint16  status;
    uint32  misc; ///用于错误标示和计数，详见硬件手册
    uint32  reserved;
};

//* The am79c970a 32-Bit initialization block, described in databook. */
/*
 *The Mode Register (CSR15) allows alteration of the chip’s operating 
 *parameters. The Mode field of the Initialization Block is copied directly 
 *into CSR15. Normal operation is the result of configuring the Mode field 
 *with all bits zero.
 */
struct am79c970a_init_block 
{
    uint16  mode;
    uint16  tlen_rlen;
    uint8   phys_addr[6];
    uint16  reserved;
    uint32  filter[2];
    /* Receive and transmit ring base, along with extra bits. */
    uint32  rx_ring;
    uint32  tx_ring;
};

struct nic_am79c970a
{
	struct pci_device *pci_dev;
	
	uint32 io_address;
    uint32 irq;
    uint8  mac_addr[6];
	
	struct am79c970a_init_block *init_block;
	uint32 init_dma_addr;/* DMA address of beginning of the init block,
                   returned by pci_alloc_consistent */

	/* The Tx and Rx ring entries must be aligned on 16-byte boundaries in 32bit mode. */
    struct am79c970a_rx_head  *rx_ring;
    struct am79c970a_tx_head  *tx_ring;
	uint32 rx_ring_dma_addr;
    uint32 tx_ring_dma_addr;
	
    uint8 *tx_buf_addr;
    uint8 *rx_buf_addr;
	
    struct am79c970a_access   a;
    uint32 cur_rx, cur_tx; /* The next free ring entry */
    uint32 rx_ring_size;   /* current rx ring size */
    uint32 tx_ring_size;   /* current tx ring size */
    uint32 rx_mod_mask;    /* rx ring modular mask */
    uint32 tx_mod_mask;    /* tx ring modular mask */
    uint16 rx_len_bits;
    uint16 tx_len_bits;

    uint32 dirty_rx,   /* ring entries to be freed. */
           dirty_tx;
		   

	uint16 chip_version;   /* which variant this is */	   
};

struct nic_am79c970a am79c970a;

int am79c970a_driver_owns(uint8 rxtx, int idx);

/*===========================================================================*/

/*16 bits*/
/*
工作在16-bit IO mode下，要读取第index个CSR的值，首先往RAP中写入index,然后再从RDP
中读取数据(此时读到的数据就是第index个CSR低16bit的数据），
*/
static uint32 am79c970a_wio_read_csr(uint32 addr,int index)
{
    io_out16(addr + AM79C970A_WIO_RAP, index);
    return io_in16(addr + AM79C970A_WIO_RDP);
}

/*工作在16-bit IO mode下，要往第index个CSR中写入数据，首先往RAP中写入index，然后
再往RDP中写入数据(数据会自动被写入CSR index)，写入数据超过16bit的话，高位会被忽略掉*/
static void am79c970a_wio_write_csr(uint32 addr, int index, uint32 val)
{
    io_out16(addr + AM79C970A_WIO_RAP, index);
    io_out16(addr + AM79C970A_WIO_RDP, val);
}

/*
与static uint32 am79c970a_wio_read_csr(uint32 addr, int index)访问方
式类似，唯一不同的是读取数据是从BDP中读取
*/
static uint32 am79c970a_wio_read_bcr(uint32 addr, int index)
{
    io_out16(addr + AM79C970A_WIO_RAP, index);
    return io_in16(addr + AM79C970A_WIO_BDP);
}
/*
与static void am79c970a_wio_write_csr(uint32 addr, int index, uint32 
val)访问方式类似，唯一不同的是写入数据是到BDP里面
*/
static void am79c970a_wio_write_bcr(uint32 addr, int index, uint32 val)
{
    io_out16(addr + AM79C970A_WIO_RAP, index);
    io_out16(addr + AM79C970A_WIO_BDP, val);
}

static uint32 am79c970a_wio_read_rap(uint32 addr)
{
    return io_in16(addr + AM79C970A_WIO_RAP);
}

static void am79c970a_wio_write_rap(uint32 addr, uint32 val)
{
    io_out16(addr + AM79C970A_WIO_RAP, val);
}

///Reset causes the device to cease operation and clear its internal logic.
static void am79c970a_wio_reset(uint32 addr)
{
    ///read access to the RESET address (i.e., offset 0x14 for 16-bit I/O, offset 0x18 for 32-bit I/O),
    io_in16(addr + AM79C970A_WIO_RESET);  /// addr + 0x14
}

static int am79c970a_wio_check(uint32 addr)
{
    io_out16(addr + AM79C970A_WIO_RAP, 88);
    return (io_in16(addr + AM79C970A_WIO_RAP) == 88);
}


static struct am79c970a_access am79c970a_wio = {
    .read_csr = am79c970a_wio_read_csr,
    .write_csr = am79c970a_wio_write_csr,
    .read_bcr = am79c970a_wio_read_bcr,
    .write_bcr = am79c970a_wio_write_bcr,
    .read_rap = am79c970a_wio_read_rap,
    .write_rap = am79c970a_wio_write_rap,
    .reset = am79c970a_wio_reset
};

/*32 bits*/
static uint32 am79c970a_dwio_read_csr(uint32 addr, int index)
{
    io_out32(addr + AM79C970A_DWIO_RAP, index);
    return io_in32(addr + AM79C970A_DWIO_RDP);
}

static void am79c970a_dwio_write_csr(uint32 addr, int index, uint32 val)
{
    io_out32(addr + AM79C970A_DWIO_RAP, index);
    io_out32(addr + AM79C970A_DWIO_RDP, val);
}

static uint32 am79c970a_dwio_read_bcr(uint32 addr, int index)
{
    io_out32(addr + AM79C970A_DWIO_RAP, index);
    return io_in32(addr + AM79C970A_DWIO_BDP);
}

static void am79c970a_dwio_write_bcr(uint32 addr, int index, uint32 val)
{
    io_out32(addr + AM79C970A_DWIO_RAP, index);
    io_out32(addr + AM79C970A_DWIO_BDP, val);
}

static uint32 am79c970a_dwio_read_rap(uint32 addr)
{
    return io_in32(addr + AM79C970A_DWIO_RAP);
}

static void am79c970a_dwio_write_rap(uint32 addr, uint32 val)
{
    io_out32(addr + AM79C970A_DWIO_RAP, val);
}

static void am79c970a_dwio_reset(uint32 addr)
{
    io_in32(addr + AM79C970A_DWIO_RESET);
}

static int am79c970a_dwio_check(uint32 addr)
{
    io_out32(addr + AM79C970A_DWIO_RAP, 88);
    return (io_in32(addr + AM79C970A_DWIO_RAP) == 88);
}

static struct am79c970a_access am79c970a_wdio = {
    .read_csr = am79c970a_dwio_read_csr,
    .write_csr = am79c970a_dwio_write_csr,
    .read_bcr = am79c970a_dwio_read_bcr,
    .write_bcr = am79c970a_dwio_write_bcr,
    .read_rap = am79c970a_dwio_read_rap,
    .write_rap = am79c970a_dwio_write_rap,
    .reset = am79c970a_dwio_reset
};


/*===========================================================================*/

/*
check whether os have change to read or write buffer.
if rxtx == 0: check driver access buffer 
if rxtx == 1: check card access buffer 
*/
int am79c970a_driver_owns(uint8 rxtx, int idx)
{
	if(rxtx == 0){
		/*
		0x8000 means card control the buffer
		OWNS bits
		*/
		return ((am79c970a.rx_ring[idx].status&0x8000) == 0);
	}else if(rxtx == 1){
		return ((am79c970a.tx_ring[idx].status&0x8000) == 0);
	}
}


int32 am79c970a_config(uint32 ioaddr, int shared)
{
	int i, media;
    int fdx, mii, fset, dxsuflo;
    int chip_version; ///版本号
    char *chipname;  ///芯片名称
    struct net_device *dev;
    struct am79c970a_access *a = NULL;
    uint8 mac_addr[6];
	uint32 val;
	
	printk("set and enable interrupt irq:%d\n", am79c970a.irq);
	
	/*79c970A默认的IO mode是16bit 所以这里reset要使用16bit mode的读操作方式*/
    ///Reset causes the device to cease operation and clear its internal logic.
	/*1.reset*/
	am79c970a_wio_reset(ioaddr);
	printk("AM79C970A: reset\n");
	
	/*往RDP写入一个32位数值，就可以切换到32位模式*/
	io_out32(ioaddr + AM79C970A_DWIO_RDP, 0);

	a = &am79c970a_wdio;
	
	int16 io_mode = a->read_bcr(ioaddr, 18);
	if(io_mode & 0x80){
		printk("AM79C970A: default 32 bits mode\n");
	}else{
		printk("AM79C970A: default 16 bits mode\n");
	}
	
	a->write_csr(ioaddr, CSR0, CSR0_STOP);
	
	io_mode = a->read_bcr(ioaddr, 18);
	if(io_mode & 0x80){
		printk("AM79C970A: default 32 bits mode\n");
	}else{
		printk("AM79C970A: default 16 bits mode\n");
	}
	
	a->write_bcr(ioaddr, 20, 2);//BCR20控制从16位到32位的切换，切换的是software style
	
	/*
	uint32 csr58 = am79c970a_wio_read_csr(ioaddr, 58);
	csr58 &= 0xfff0;
	csr58 |= 2;
	am79c970a_wio_write_csr(ioaddr, 58, csr58);
	*/
	
	//io_out32(ioaddr + AM79C970A_DWIO_RDP, 0);
	//
	
	
	/*4.get mac address*/
	
	/* In most chips, after a chip reset, the ethernet address is read from the
     * station address PROM at the base address and programmed into the
     * "Physical Address Registers" CSR12-14.
     * As a precautionary measure, we read the PROM values and complain if
     * they disagree with the CSRs.  If they miscompare, and the PROM addr
     * is valid, then the PROM addr is used.
     */
    /*填充dev中的MAC地址。MAC地址存放于CSR12(对应MAC[15:0])，CSR13(MAC[31:16])
     *  ，CSR14(MAC[47:32]),每个CSR均使用低16位
     * CSR3 bit2可设置网卡使用大端模式或者小端模式，CSR3 bit2为0是为小端，为1时为大端，
     * 默认值是0，即默认为小端，因为此时网卡已经重置，所以此时网卡使用的是小端，当前CPU
     * 为x86架构，也为小端，所以不存在大小端转换问题
     */
    for (i = 0; i < 3; i++) {
        val = a->read_csr(ioaddr, i + 12) & 0x0ffff;
        /* There may be endianness issues here. */
        mac_addr[2 * i] = val & 0x0ff;
        mac_addr[2 * i + 1] = (val >> 8) & 0x0ff;
    }
    /* 
     *对从PROM中读取的MAC地址（存放于promaddr）和从CSR12~CSR14中读取的MAC地址（存放于dev->dev_addr)
	* 进行比较，如果两个值不相等或者dev_addr不是一个有效的MAC地址，则把prom的值拷给dev->dev_addr;
     *为什么要比较，因为网卡可能重新打开，那么可能CSR12~CSR14里面的值是之前已经改动过
     *的MAC地址，非原始MAC地址，所以要把他回复成原始的MAC地址
     */
	 
    if (memcmp(mac_addr, am79c970a.mac_addr, 6)
        || !ethernet_is_valid_mac_addr(mac_addr)) {
        if (ethernet_is_valid_mac_addr(am79c970a.mac_addr)) {
            printk(" warning: CSR address invalid,\n");
            printk("    using instead PROM address of");
            
            memcpy(mac_addr, am79c970a.mac_addr, 6);
        }
    }
	memcpy(am79c970a.mac_addr, mac_addr, ETH_ADDR_LEN);
	
	printk("AM79C970A: set mac addr: %x:%x:%x:%x:%x:%x\n", am79c970a.mac_addr[0], am79c970a.mac_addr[1],\
            am79c970a.mac_addr[2], am79c970a.mac_addr[3], am79c970a.mac_addr[4], am79c970a.mac_addr[5]);
	
	#ifdef _CONFIG_NET_ETHERNET_
		ethernet_set_eth_addr(am79c970a.mac_addr);
	#endif
	
	/*5.init rx and tx buffer*/
	
	/////am79c970a.tx_ring_size 表示发送ring的大小（即一个ring上有多少个tramsimit descriptor)，
    ///  #define TX_RING_SIZE  (1 << (AM79C970A_LOG_TX_BUFFERS)) 
    /// default tx ring size *AM79C970A_LOG_TX_BUFFERS是存储在init block中 TLEN域的值 
    am79c970a.tx_ring_size = TX_RING_SIZE;    
    ////和tx_ring_size类似，不同的是rx对应的域是RELN
    am79c970a.rx_ring_size = RX_RING_SIZE;    /* default rx ring size */
    
    /*
     * 这里的tx_len_bits和rx_len_bits是后面用来填充init block的 tlen_rlen成员的
    （tlen_rlen为uint16类型的 变量）。 tlen_rlen成员的bit12~big15用来存放tx_len_bits，
    * bit4~bit7用来存放rx_len_bigs，tlen_rlen = am79c970a.tx_len_bits | am79c970a.rx_len_bits,
    * 左移是为了把tx_len_bits、rx_len_bits准确地填入tlen_rlen对应位置中
    */
    am79c970a.tx_len_bits = (AM79C970A_LOG_TX_BUFFERS << 12);  ////(4 << 12)
    am79c970a.rx_len_bits = (AM79C970A_LOG_RX_BUFFERS << 4);   ////(5 << 4)
	
	am79c970a.cur_tx = 0;
	am79c970a.cur_rx = 0;
	
	/*alloc ring struct buffer*/
	am79c970a.rx_ring = mm.malloc(sizeof(struct am79c970a_rx_head) *am79c970a.rx_ring_size);
    if (am79c970a.rx_ring == NULL) {
        printk("AM79C970A: memory allocation failed.\n");
		goto err_release_region;
    }
	am79c970a.rx_ring_dma_addr = addr_v2p((uint32 )am79c970a.rx_ring);

	 /// 分配一致映射DMA缓冲区，缓冲区的大小和ring的大小（即 tx_ring_size 个  tx_head是相同的)
    am79c970a.tx_ring = mm.malloc(sizeof(struct am79c970a_tx_head) *am79c970a.tx_ring_size);
    if (am79c970a.tx_ring == NULL) {
        printk("AM79C970A: memory allocation failed.\n");
        goto err_release_region;
    }
	am79c970a.tx_ring_dma_addr = addr_v2p((uint32 )am79c970a.tx_ring);

	/*alloc data buffer*/
	/*为每一个块分配连续的地址，为了保证物理地址连续，就直接分配一个页*/
	am79c970a.rx_buf_addr = kernel_alloc_page(RX_RING_SIZE);
	
	for (i = 0; i < RX_RING_SIZE; i++) {
		am79c970a.rx_ring[i].base = addr_v2p((uint32 )(am79c970a.rx_buf_addr + PAGE_SIZE*i));
		am79c970a.rx_ring[i].buf_length = -PKT_BUF_SKB;
		am79c970a.rx_ring[i].status = 0x8000; 
	}
	
	/*为每一个块分配连续的地址，为了保证物理地址连续，就直接分配一个页*/
	am79c970a.tx_buf_addr = kernel_alloc_page(TX_RING_SIZE);
	
	for (i = 0; i < TX_RING_SIZE; i++) {
		am79c970a.tx_ring[i].base = addr_v2p((uint32 )(am79c970a.tx_buf_addr + PAGE_SIZE*i));
		am79c970a.tx_ring[i].length  = 0;
		am79c970a.rx_ring[i].status = 0x0000; 
	}
	printk("AM79C970A: tx and tx buf init\n");
	
	/*初始化block*/
	
	/*6.access way set*/
	if (!a) {
        printk("AM79C970A: No access methods\n");
        goto err_release_region;
    }
    am79c970a.a = *a;
	
	/*7.init initialization block*/
	am79c970a.init_block = (struct am79c970a_init_block *)kernel_alloc_page(1);
	if(am79c970a.init_block == NULL){
		printk("AM79C970A: alloc mem for init block failed!\n");
		goto err_release_region;
	}
	am79c970a.init_dma_addr = addr_v2p((uint32 )am79c970a.init_block);
	
	/*填写initialization block开始*/
    ///对应MODE 0x0003 设置了 两位  1. loopback enable  2.Disable Transmit
    am79c970a.init_block->mode = 0x0003; /* Disable Rx and Tx. Mode初始化的是CSR15*/
    ///对应TLEN 和 RLEN
    am79c970a.init_block->tlen_rlen =
        am79c970a.tx_len_bits | am79c970a.rx_len_bits;
    ///对应PADR 48bit
    for (i = 0; i < 6; i++)
        am79c970a.init_block->phys_addr[i] = am79c970a.mac_addr[i];
    ///对应LADRF
    am79c970a.init_block->filter[0] = 0x00000000;
    am79c970a.init_block->filter[1] = 0x00000000;
    ///对应 RDRA和 TDRA， RDRA 是由传输任务描述符组成的数组(tx_ring)的首地址
    am79c970a.init_block->rx_ring = am79c970a.rx_ring_dma_addr;   //// rx_ring_dma_addr的值在am79c970a_alloc_ring中被赋值
    am79c970a.init_block->tx_ring = am79c970a.tx_ring_dma_addr;
    
	printk("===== init block =====\n");
	printk("addr:%x dma:%x\n", am79c970a.init_block, am79c970a.init_dma_addr);
	printk("mode:%x tlen_rlen:%x\n", am79c970a.init_block->mode, am79c970a.init_block->tlen_rlen);
	printk("mac:%x:%x:%x:%x:%x:%x\n", am79c970a.init_block->phys_addr[0], \
		am79c970a.init_block->phys_addr[1], am79c970a.init_block->phys_addr[2], \
		am79c970a.init_block->phys_addr[3], am79c970a.init_block->phys_addr[4], \
		am79c970a.init_block->phys_addr[5]);
		
	printk("filter:%x %x\n", am79c970a.init_block->filter[1] , am79c970a.init_block->filter[0]);
	printk("rx_ring:%x tx_ring:%x\n", am79c970a.init_block->rx_ring, am79c970a.init_block->tx_ring);
	
	/*把结构体传输给芯片*/
	/*
	a->write_csr(ioaddr, 1, (am79c970a.init_dma_addr & 0xffff));//CSR1和CSR2中存放intial_block的起始地址。
    a->write_csr(ioaddr, 2, (am79c970a.init_dma_addr >> 16));*/
	
	/*写入mode*/
	a->write_csr(ioaddr, CSR15, am79c970a.init_block->mode);
	/*写入TLEN AND RLEN*/
	/*可能需要设置成补码*/
	a->write_csr(ioaddr, CSR76, am79c970a.rx_ring_size);
	a->write_csr(ioaddr, CSR78, am79c970a.tx_ring_size);
	/*写入MAC地址*/
	a->write_csr(ioaddr, CSR12, (am79c970a.init_block->phys_addr[1]<<8)|am79c970a.init_block->phys_addr[0]);
	a->write_csr(ioaddr, CSR13, (am79c970a.init_block->phys_addr[3]<<8)|am79c970a.init_block->phys_addr[2]);
	a->write_csr(ioaddr, CSR14, (am79c970a.init_block->phys_addr[5]<<8)|am79c970a.init_block->phys_addr[4]);
	
	/*写入ring dma addr*/
	a->write_csr(ioaddr, CSR24, am79c970a.init_block->rx_ring&0xffff);
	a->write_csr(ioaddr, CSR25, (am79c970a.init_block->rx_ring>>16)&0xffff);
	
	a->write_csr(ioaddr, CSR30, am79c970a.init_block->tx_ring&0xffff);
	a->write_csr(ioaddr, CSR31, (am79c970a.init_block->tx_ring>>16)&0xffff);
	
	
	/*写入控制信息*/
	a->write_csr(ioaddr, 18, addr_v2p((uint32)am79c970a.rx_buf_addr)&0xffff);
	a->write_csr(ioaddr, 19, (addr_v2p((uint32)am79c970a.rx_buf_addr)>>16)&0xffff);
	
	a->write_csr(ioaddr, 20, addr_v2p((uint32)am79c970a.tx_buf_addr)&0xffff);
	a->write_csr(ioaddr, 21, (addr_v2p((uint32)am79c970a.tx_buf_addr)>>16)&0xffff);
	
	a->write_csr(ioaddr, 28, addr_v2p((uint32)am79c970a.rx_ring)&0xffff);
	a->write_csr(ioaddr, 29, (addr_v2p((uint32)am79c970a.rx_ring)>>16)&0xffff);
	
	a->write_csr(ioaddr, 34, addr_v2p((uint32)am79c970a.tx_ring)&0xffff);
	a->write_csr(ioaddr, 35, (addr_v2p((uint32)am79c970a.tx_ring)>>16)&0xffff);
	
	
	printk("put block\n");
	/*设置一些控制相关的信息*/
	
	/* enable interrupt */
	put_irq_handler(am79c970a.irq, am79c970a_handler);
	enable_irq(am79c970a.irq);
	
	
	printk("AM79C970A config done!\n");
	
	return 0;
	/*信息配置到此告一段落*/
	while(1);
	
	am79c970a_wio_write_csr(ioaddr, 0, 0);
	printk("AM79C970A: clear csr0\n");
	
	/*设置SWSTYLE，并且切换到32位模式*/
	/*uint32 csr58 = am79c970a_wio_read_csr(ioaddr,58);
	csr58 &= 0xfff0;
	csr58 |= 2;
	am79c970a_wio_write_csr(ioaddr,58, csr58);
	a = &am79c970a_wdio;
	printk("AM79C970A: set 32 bits mode\n");
	*/
	/* set/reset autoselect bit */

	/*把csr0写入INIT, 初始化*/
	a->write_csr(ioaddr, CSR0, CSR0_INIT);
	
	/*循环等待，当init block写入网卡寄存器完成，contorller会自动将CSR0_IDON位置1，
	该位由0变为1时就说明init block传输完成了*/
	i = 0;
    while (i++ < 100){
		if (a->read_csr(ioaddr, CSR0) & CSR0_IDON){
			printk("card init done!\n");
			break;
		}
	}

	a->write_csr(ioaddr, CSR0, CSR0_NORMAL);
	
	printk("set csr0 to normal mode\n");
	return 0;
	while(1);
	
	
	/*3.get chip's version*/
	/// read the chip's version number from CSR88 and CSR89
    chip_version =
        a->read_csr(ioaddr, 88) | (a->read_csr(ioaddr, 89) << 16);
    ///经测试芯片型号是am79c970A，其version number is (0x0242 1003 | 0x0262H << 16) = 0x0262 1003

	
	/// the last 12 bits of the version number should all be 0x003, I think
    ///  chipversion = 0262 1003H
    if ((chip_version & 0xfff) != 0x003) {
        printk("Unsupported chip version.\n");
        goto err_release_region;
    }
	
	fdx = mii = fset = dxsuflo = 0;

	/// get the 2 bytes befor 0x003;
    chip_version = (chip_version >> 12) & 0xffff;
    /// VMware提供的网卡是79c97cA,chip_version = 2621H
    switch (chip_version) {
		case 0x2420:
		//// because the chip supplied by vmware is 70c970, so I will concentrate on  this case
			chipname = "PCnet/PCI 79C970";  /* PCI */
			break;
		case 0x2430:
			if (shared)
				chipname = "PCnet/PCI 79C970";  /* 970 gives the wrong chip id back */
			else
				chipname = "PCnet/32 79C965";   /* 486/VL bus */
			break;
		case 0x2621:
			chipname = "PCnet/PCI II 79C970A";  /* PCI */
			fdx = 1;
			break;
		case 0x2623:
			chipname = "PCnet/FAST 79C971"; /* PCI */
			fdx = 1;
			mii = 1;
			fset = 1;
			break;
		case 0x2624:
			chipname = "PCnet/FAST+ 79C972";    /* PCI */
			fdx = 1;
			mii = 1;
			fset = 1;
			break;
		case 0x2625:
			chipname = "PCnet/FAST III 79C973"; /* PCI */
			fdx = 1;
			mii = 1;
			break;
		case 0x2626:
			/// well,the 79c978 chip looks like it's a little special, I don't have the hardware manual of it ,and  just ignore it
			chipname = "PCnet/Home 79C978"; /* PCI */
			fdx = 1;
			
			break;
		case 0x2627:
			chipname = "PCnet/FAST III 79C975"; /* PCI */
			fdx = 1;
			mii = 1;
			break;
		case 0x2628:
			chipname = "PCnet/PRO 79C976";
			fdx = 1;
			mii = 1;
			break;
		default:
			printk("PCnet version %x, no am79c970a chip.\n", chip_version);
			goto err_release_region;
    }
	/*
     *  On selected chips turn on the BCR18:NOUFLO bit. This stops transmit
     *  starting until the packet is loaded. Strike one for reliability, lose
     *  one for latency - although on PCI this isnt a big loss. Ohlder chips
     *  have FIFO's smaller tan a packet, so you can't do this.
     *  Turn on BCR18:BurstRdEn and BCR18:BurstWrEn.
     */
    //// case  79c970 ，fset == 0, false， so was ingored by me
    if (fset) {
        a->write_bcr(ioaddr, 18, (a->read_bcr(ioaddr, 18) | 0x0860));//打开BCR18的第11位，即NOUFLO位，以及第6位（BREADE）和第5位（BWRITE）
        a->write_csr(ioaddr, 80,
                 (a->read_csr(ioaddr, 80) & 0x0C00) | 0x0c00);
        dxsuflo = 1;
    }
	am79c970a.chip_version = chip_version;

	printk("AM79C970A: get chip %s.\n", chipname);
	
	/***
     * While in auto-selection mode, the interface in use is determined by an auto-sensing mechanism which checks the
     * link status on the 10BASE-T port. If there is no active link status, then the device assumes an AUI connection
     *bcr2 bit1置0，
     */
	
	///\ 单个网卡芯片，不设置GPSI
	/*
	///\ bit4 GPSIEN:General Purpose Serial Interface Enable.，GPSI is used as an interface between Ethernet MAC and PHY blocks.
    val = am79c970a.a.read_csr(ioaddr, 124) & ~0x10;

    am79c970a.a.write_csr(ioaddr, 124, val);
	*/

	
	
err_release_region:
	return 0;
}

int am79c970a_open()
{
	printk("am79c970a open start.\n");
	
	int ret = -1;
	uint32 ioaddr = am79c970a.io_address;
	struct am79c970a_access *a;
	int val;
	/*检测mac是否有效*/
	if(!ethernet_is_valid_mac_addr(am79c970a.mac_addr)){
		ret = -1;
		printk("AM79C970A mac addr is invalid!\n");
		goto err_release_region;
	}
	
	a = &am79c970a.a;
	
	/*再次重置设备*/
	//a->reset(ioaddr);
	/*往RDP写入一个32位数值，就可以切换到32位模式*/
	//io_out32(ioaddr + AM79C970A_DWIO_RDP, 0);
	
	/*设置成自动选择网络接口，第二位设置成1*/
    val = a->read_bcr(ioaddr, 2) & ~2;
    val |= 0x02;
	a->write_bcr(ioaddr, 2, val);
	
	/*设置初始化完后不产生中断，我们循环判断标志位*/
	uint32 csr3 = a->read_csr(ioaddr, CSR3);
	csr3 |= 0X100;
	a->write_csr(ioaddr, CSR3, csr3);
	printk("no init intr after init done\n");
	
	/*更新初始化相关模式*/
	//am79c970a.init_block->mode = (AM79C970A_PORT_ASEL & AM79C970A_PORT_PORTSEL) << 7;
	
	//am79c970a_wio_write_csr(ioaddr, 20, 2);//BCR20控制从16位到32位的切换，切换的是software style
	/*
	uint32 csr58 = am79c970a_wio_read_csr(ioaddr, 58);
	csr58 &= 0xfff0;
	csr58 |= 2;
	am79c970a_wio_write_csr(ioaddr, 58, csr58);
	*/
	
	/*写初始化地址到初始化块地址寄存器*/
	/*a->write_csr(ioaddr, 1, (am79c970a.init_dma_addr & 0xffff));//CSR1和CSR2中存放intial_block的起始地址。
    a->write_csr(ioaddr, 2, (am79c970a.init_dma_addr >> 16));
	*/
	printk("set new addr\n");
	
	/*设置一些中断遮罩以及特征*/
	/*如果数据长度小于64，则填充后面的数据
	CSR4: Test and Features Control，屏蔽Jabber Error，Transmit Start，
	Receive Collision Counter Overflow产生的中断
    */
	a->write_csr(ioaddr, CSR4, 0x0915);  /* auto tx pad */
	
    /*INIT  启动init block从内存写入网卡*/
    //a->write_csr(ioaddr, CSR0, CSR0_INIT);
	printk("init start\n");
	
	/*循环等待，当init block写入网卡寄存器完成，contorller会自动将CSR0_IDON位置1，
	该位由0变为1时就说明init block传输完成了
    */
	/*int i = 0;
    while (i++ < 100)
        if (a->read_csr(ioaddr, CSR0) & CSR0_IDON)
            break;
	*/
	
	/*设置开始并且可以接收中断*/
	a->write_csr(ioaddr, CSR0, CSR0_NORMAL);	
	printk("init normal\n");
	
	printk("===== info report =====\n");
	uint32 init_block_addr = a->read_csr(ioaddr, CSR1) | (a->read_csr(ioaddr, CSR2)<<16);
	printk("init block addr:%x\n",init_block_addr);
	
	uint16 mac_addr = a->read_csr(ioaddr, CSR12);
	printk("mac: %x:%x:",mac_addr&0xff, mac_addr>>8);
	mac_addr = a->read_csr(ioaddr, CSR13);
	printk("%x:%x:",mac_addr&0xff, mac_addr>>8);
	mac_addr = a->read_csr(ioaddr, CSR14);
	printk("%x:%x\n",mac_addr&0xff, mac_addr>>8);
	
	uint32 val32 = a->read_csr(ioaddr, CSR24) | (a->read_csr(ioaddr, CSR25)<<16);
	printk("rx addr:%x ",val32);
	
	val32 = a->read_csr(ioaddr, CSR30) | (a->read_csr(ioaddr, CSR31)<<16);
	printk("tx addr:%x\n",val32);
	
	val32 = a->read_csr(ioaddr, CSR76);
	printk("Receive Descriptor Ring Length:%x\n",val32);
	
	val32 = a->read_csr(ioaddr, CSR78);
	printk("Transmit  Descriptor Ring Length:%x\n",val32);
	
	printk("am79c970a open done.\n");
	
	
err_release_region:
	return 0;
}


void am79c970a_handler(int32 irq)
{
	printk("am79c970a rx start>>>\n");
	//return;
	u16 csr0;
	int entry;
	uint8 *rx_buf;
	uint32 ioaddr = am79c970a.io_address;
	int i;
	uint16 status;
	/* Mask to ring buffer boundary. */
	printk("check owns start!\n");
	entry = am79c970a.cur_rx;
	
	while(am79c970a_driver_owns(0, entry)){
		entry = am79c970a.cur_rx;
		status = am79c970a.rx_ring[entry].status>>8;
		//printk("1 ");
		printk("rx[%d].status == %x\n", entry, status);
		if(status == 0x3){
			printk("error! --");
			 // 下面的错误种类和位的对应关系让人有点迷茫，暂时不管
			if (status & 0x01)  /* Only count a general error at the */
				printk("Only count a general error at the end of a packet.\n");
				//dev->stats.rx_errors++; /* end of a packet. */
				
			if (status & 0x20)  //DM//Start of Packet
				printk("Start of Packet.\n");
				
				//dev->stats.rx_frame_errors++;
			if (status & 0x10)
				printk("rx_over_errors.\n");
				
				//dev->stats.rx_over_errors++;
			if (status & 0x08)
				printk("rx_crc_errors.\n");
				
				//dev->stats.rx_crc_errors++;
			if (status & 0x04)
				printk("rx_fifo_errors.\n");
				
				//dev->stats.rx_fifo_errors++;
			return;
		}
		//printk("2 ");
		rx_buf = am79c970a.rx_buf_addr + entry*PAGE_SIZE;
		//memcpy(tx_buf, data_buf, data_len);
		//printk("3 ");
		for (i = 0; i < 16; i++) {
			printk("%x ", rx_buf[i]);
		}
		//printk("4 ");
		/*hand the buffer back to the card*/
		am79c970a.rx_ring[entry].status = 0x8000;
		//printk("5 ");
		am79c970a.cur_rx++;
		if(am79c970a.cur_rx >= RX_RING_SIZE){
			am79c970a.cur_rx = 0;
		}
	}
	//am79c970a.a.write_csr(ioaddr, CSR0, am79c970a.a.read_csr(ioaddr, 0) | CSR0_RINT);
	printk("am79c970a rx end>>>\n");
}

int am79c970a_transmit(uint8 *data_buf, uint32 data_len)  //发包时会调用该函数
{
	printk("am79c970a tx start>>>\n");
    uint32 ioaddr = am79c970a.io_address;
    u16 status;
    int entry;
    uint32 flags;
    uint32 dma_addr;
    uint8 *tx_buf;
    /* Mask to ring buffer boundary. */
    entry = am79c970a.cur_tx;
	int i;
	if(!am79c970a_driver_owns(1, entry)){
		printk("##owns not in driver!\n");
		return 0;
	}
	printk("@@owns in driver!\n");
	
	printk("tx[%d] \n", entry);
	/*
	while(!am79c970a_driver_owns(0, entry)){
		printk("tx driver own\n");
		return 0;
	}
	*/
    /* Default status -- will not enable Successful-TxDone
     * interrupt when that option is available to us.
     */
	/*card owns, is start entry, is end entry*/
    status = 0x8300;

    /* Fill in a Tx ring entry */

    
    /* Caution: the write order is important here, set the status
     * with the "ownership" bits last. */

    am79c970a.tx_ring[entry].length = (uint16)(-data_len);//填写length的补码 ////额。。这里有点奇怪啊。。

    am79c970a.tx_ring[entry].misc = 0x00000000;  ////对应TMD2，具体含义暂时不看
	
	/*copy data*/
	tx_buf = am79c970a.tx_buf_addr + entry*PAGE_SIZE;
	memcpy(tx_buf, data_buf, data_len);
	
	printk("tx data:%x\n");
	
	for(i = 0; i < 16; i++){
		printk("%x ", tx_buf[i]);
	}
	
	/* Make sure owner changes after all others are visible */  //memory barrier
    ///其他位暂时不管，反正这么填就对了
    am79c970a.tx_ring[entry].status = status;//将own位置1，即将ring buffer控制权交给网卡。

    am79c970a.cur_tx++;
	if(am79c970a.cur_tx >= TX_RING_SIZE){
		am79c970a.cur_tx = 0;
	}
    /* Trigger an immediate send poll. */
    //CSR0_TXPOLL=0x8，CSR0_INTEN=0x40 ，即将第3位和第6位置1。即触发传输包，并使网卡中断生效
   // am79c970a.a.write_csr(ioaddr, CSR0, CSR0_INTEN | CSR0_TXPOLL);
	printk("<<<am79c970a tx end\n");
    return 0;
}

void am79c970a_init()
{
	printk(">init AM79C970A\n");
	
	if(am79c970a_get_info_from_pci() != 0){
		printk("AM79C970A: get info from pci failed!\n");
		return;
	}
	
	/* mac address */
	int i;
    for (i = 0; i < 6; i++) {
        am79c970a.mac_addr[i] = io_in8(am79c970a.io_address + 0 + i);
    }
    
	am79c970a_config(am79c970a.io_address, 0);
	am79c970a_open();
	/*
	uint8 buf[64];
	memset(buf, 0, 64);
	
	am79c970a_transmit(buf, 64);
	*/
	printk(">init AM79C970A done!\n");
	//while(1);
}

int32 am79c970a_get_info_from_pci()
{
	/* get pci device */
    struct pci_device* device = pci_get_device(AM79C970A_VENDOR_ID, AM79C970A_DEVICE_ID);
    if (device == NULL) {
        printk("AM79C970A init failed: not find pci device.\n");
        return -1;
    }
	am79c970a.pci_dev = device;
	
    printk("find AM79C970A device, vendor id: 0x%x, device id: 0x%x\n",\
            device->vendor_id, device->device_id);

    /* enable bus mastering */
	pci_enable_bus_mastering(device);

    /* get io address */
    am79c970a.io_address = pci_device_get_io_addr(device);
    if (am79c970a.io_address == 0) {
        printk("AM79C970A init failed: INVALID pci device io address.\n");
        return -1;
    }
    printk("AM79C970A io address: 0x%x\n", am79c970a.io_address);

    /* get irq */
    am79c970a.irq = pci_device_get_irq_line(device);
    if (am79c970a.irq == 0xff) {
        printk("AM79C970A init failed: INVALID irq.\n");
        return -1;
    }
	
    printk("AM79C970A irq: %d\n", am79c970a.irq );

	return 0;
}

#endif

