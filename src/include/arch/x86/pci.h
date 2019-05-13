#ifndef _PCI_H_
#define _PCI_H_

#include <types.h>
#include <stdint.h>

/*
【简介】
	PCI是什么？PCI是Peripheral Component Interconnect(外设部件互连标准)的缩写，
它是目前个人电脑中使用最为广泛的接口，几乎所有的主板产品上都带有这种插槽。

	PCI有三个相互独立的物理地址空间：设备存储器地址空间、I/O地址空间和配置空间。
配置空间是PCI所特有的一个物理空间。由于PCI支持设备即插即用，所以PCI设备不占
用固定的内存地址空间或I/O地址空间，而是由操作系统决定其映射的基址。

	PCI总线规范定义的配置空间总长度为256个字节，配置信息按一定的顺序和大小依次存放。
前64个字节的配置空间称为配置头，对于所有的设备都一样，配置头的主要功能是用来识别设备、
定义主机访问PCI卡的方式（I/O访问或者存储器访问，还有中断信息）。
其余的192个字节称为本地配置空间（设备有关区），主要定义卡上局部总线的特性、本地空间基地址及范围等。

【配置空间中的重要信息】

	Vendor ID：厂商ID。知名的设备厂商的ID。FFFFh是一个非法厂商ID，可它来判断PCI设备是否存在。

	Device ID：设备ID。某厂商生产的设备的ID。操作系统就是凭着 Vendor ID和Device ID 找到对应驱动程序的。

	Class Code：类代码。共三字节，分别是 类代码、子类代码、编程接口。类代码不仅用于区分设备类型，还是编程接口的规范，这就是为什么会有通用驱动程序。

	IRQ Line：IRQ编号。PC机以前是靠两片8259芯片来管理16个硬件中断。
	现在为了支持对称多处理器，有了APIC（高级可编程中断控制器），它支持管理24个中断。

	IRQ Pin：中断引脚。PCI有4个中断引脚，该寄存器表明该设备连接的是哪个引脚。


【访问配置空间】	
	如何访问配置空间呢？可通过访问0xCF8h、0xCFCh端口来实现。
	0xCF8h: CONFIG_ADDRESS。PCI配置空间地址端口。
	0xCFCh: CONFIG_DATA。PCI配置空间数据端口。
	
【CONFIG_ADDRESS寄存器格式】
	31　位： Enabled位。
	23:16 位： 总线编号。
	15:11 位： 设备编号。
	10: 8 位：功能编号。
	7: 2 位：配置空间寄存器编号。
	1: 0 位：恒为“00”。这是因为CF8h、CFCh端口是32位端口。
	
*/

/*
通过学习，可以发现，pci主要是读取pci的配置空间信息，后面读写会用到。
*/

#define PCI_CONFIG_ADDR	0xCF8	/*PCI配置空间地址端口*/
#define PCI_CONFIG_DATA	0xCFC	/*PCI配置空间数据端口*/

/*PCI配置空间数据的偏移*/
#define PCI_DEVICE_VENDER								0x00	
#define PCI_STATUS_COMMAND								0x04	
#define PCI_CLASS_CODE_REVISION_ID						0x08	
#define PCI_BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE	0x0C
#define PCI_BASS_ADDRESS0								0x10
#define PCI_BASS_ADDRESS1								0x14
#define PCI_BASS_ADDRESS2								0x18
#define PCI_BASS_ADDRESS3								0x1C
#define PCI_BASS_ADDRESS4								0x20
#define PCI_BASS_ADDRESS5								0x24
/*...还要其他没有用到的，暂时不罗列*/
#define PCI_MAX_LNT_MIN_GNT_IRQ_PIN_IRQ_LINE			0x3C

/*IO地址和MEM地址的地址mask*/
#define PCI_BASE_ADDR_MEM_MASK           (~0x0FUL)
#define PCI_BASE_ADDR_IO_MASK            (~0x03UL)

#define PCI_BAR_TYPE_INVALID 	0
#define PCI_BAR_TYPE_MEM 		1
#define PCI_BAR_TYPE_IO 			2


#define PCI_MAX_BAR 6		/*每个设备最多有6地址信息*/
#define PCI_MAX_BUS 256		/*PCI总共有256个总线*/
#define PCI_MAX_DEV 32		/*PCI每条总线上总共有32个设备*/
#define PCI_MAX_FUN 8		/*PCI设备总共有8个功能号*/

#define PCI_MAX_DEVICE_NR 256	/*系统最大支持检测多少个设备*/

/*PCI地址bar结构体，保存Bass Address （0~5）的信息*/
struct pci_device_bar
{
	uint32 type;		/*地址bar的类型（IO地址/MEM地址）*/
    uint32 base_addr;	/*地址的值*/
    uint32 length;		/*地址的长度*/
};

#define PCI_DEVICE_STATUS_INVALID 		0
#define PCI_DEVICE_STATUS_USING		 	1

/*
PCI设备结构体，用于保存我们所需要的pci信息，并不是和硬件的一样
*/
struct pci_device
{
	char status; 		/*device status*/
	
	uint8 bus;				/*bus总线号*/
	uint8 dev;				/*device号*/
	uint8 function;		/*功能号*/
	
	uint16 vendor_id;		/*配置空间:Vendor ID*/
    uint16 device_id;		/*配置空间:Device ID*/
    uint32 class_code;		/*配置空间:Class Code*/
	uint8 revision_id;		/*配置空间:Revision ID*/
    uint8 multi_function;	/*多功能标志*/
    uint8 irq_line;			/*配置空间:IRQ line*/
    uint8 irq_pin;			/*配置空间:IRQ pin*/
    
    struct pci_device_bar bar[PCI_MAX_BAR];	/*有6个地址信息*/
};

void pci_device_bar_init(struct pci_device_bar *bar, uint32 addr_reg_val, uint32 len_reg_val);
void pci_device_bar_dump(struct pci_device_bar *bar);
void pci_device_init(struct pci_device *device, uint8 bus, uint8 dev, uint8 function,\
					uint16 vendor_id, uint16 device_id, uint32 class_code, uint8 revision_id,\
					uint8 multi_function);
void pci_device_dump(struct pci_device *device);
uint32 pci_device_get_io_addr(struct pci_device *device);
uint32 pci_device_get_mem_addr(struct pci_device *device);
uint32 pci_device_get_irq_line(struct pci_device *device);
void init_pci();
struct pci_device *pci_alloc_device();
int pci_free_device(struct pci_device *device);
void pci_scan_buses();
void pci_scan_device(uint8 bus, uint8 device, uint8 function);
uint32 pci_read(uint32 bus, uint32 device, uint32 function, uint32 addr);
void pci_write(uint32 bus, uint32 device, uint32 function, uint32 addr, uint32 val);
struct pci_device* pci_get_device(uint16 vendor_id, uint16 device_id);
void pci_enable_bus_mastering(struct pci_device *device);

uint32 pci_device_read(struct pci_device *device, uint32 reg);
void pci_device_write(struct pci_device *device, uint32 reg, uint32 value);
uint32 pic_get_device_connected();

#endif
