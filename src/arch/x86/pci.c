/*
File:		kernel/pci.c
Contains:	pci manage
Auther:		Hu Zicheng
Time:		2019/4/8
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <sys/arch.h>
#include <sys/core.h>
#include <sys/mm.h>

struct pci_device pci_device_table[PCI_MAX_DEVICE_NR];	/*device table*/
	
void pci_device_bar_init(struct pci_device_bar *bar, uint32 addr_reg_val, uint32 len_reg_val)
{
	/*if addr is 0xffffffff, we set it to 0*/
    if (addr_reg_val == 0xffffffff) {
        addr_reg_val = 0;
    }
	/*we judge type by addr register bit 0, if 1, type is io, if 0, type is memory*/
    if (addr_reg_val & 1) {
        bar->type = PCI_BAR_TYPE_IO;
		bar->base_addr = addr_reg_val  & PCI_BASE_ADDR_IO_MASK;
        bar->length    = ~(len_reg_val & PCI_BASE_ADDR_IO_MASK) + 1;
    }
    else {
        bar->type = PCI_BAR_TYPE_MEM;
        bar->base_addr = addr_reg_val  & PCI_BASE_ADDR_MEM_MASK;
        bar->length    = ~(len_reg_val & PCI_BASE_ADDR_MEM_MASK) + 1;
    }
}

void pci_device_bar_dump(struct pci_device_bar *bar)
{
	
    printk("type: %s, ", bar->type == PCI_BAR_TYPE_IO ? "io base address" : "mem base address");
    printk("base address: %x, ", bar->base_addr);
    printk("len: %x\n", bar->length);
}

void pci_device_init(struct pci_device *device, uint8 bus, uint8 dev, uint8 function,\
					uint16 vendor_id, uint16 device_id, uint32 class_code, uint8 revision_id,\
					uint8 multi_function)
{
	/*set value to device*/
    device->bus = bus;
    device->dev = dev;
    device->function = function;

    device->vendor_id = vendor_id;
    device->device_id = device_id;
    device->multi_function = multi_function;
    device->class_code = class_code;
    device->revision_id = revision_id;
	int i;
    for (i = 0; i < PCI_MAX_BAR; i++) {
         device->bar[i].type = PCI_BAR_TYPE_INVALID;
    }
    device->irq_line = -1;
}

void pci_device_dump(struct pci_device *device)
{
	//printk("status:      %d\n", device->status);
    printk("vendor id:      %x  ", device->vendor_id);
    printk("device id:      %x\n", device->device_id);
	/*printk("class code:     %x  ", device->class_code);
    printk("revision id:       %x  ", device->revision_id);
    printk("multi function: %d\n", device->multi_function);*/
    printk("irq line: %d  ", device->irq_line);
    printk("irq pin:  %d\n", device->irq_pin);
    int i;
	for (i = 0; i < PCI_MAX_BAR; i++) {
		/*if not a invalid bar*/
        if (device->bar[i].type != PCI_BAR_TYPE_INVALID) {
            printk("bar %d: ", i);
			pci_device_bar_dump(&device->bar[i]);
        }
    }
}

uint32 pci_device_get_io_addr(struct pci_device *device)
{
	int i;
    for (i = 0; i < PCI_MAX_BAR; i++) {
        if (device->bar[i].type == PCI_BAR_TYPE_IO) {
            return device->bar[i].base_addr;
        }
    }

    return 0;
}

uint32 pci_device_get_mem_addr(struct pci_device *device)
{
	int i;
    for (i = 0; i < PCI_MAX_BAR; i++) {
        if (device->bar[i].type == PCI_BAR_TYPE_MEM) {
            return device->bar[i].base_addr;
        }
    }

    return 0;
}

uint32 pci_device_get_irq_line(struct pci_device *device)
{
    return device->irq_line;
}

/*read value from pci device config space register*/
uint32 pci_device_read(struct pci_device *device, uint32 reg)
{
    return pci_read(device->bus, device->dev, device->function, reg);
}

/*write value to pci device config space register*/
void pci_device_write(struct pci_device *device, uint32 reg, uint32 value)
{
    pci_write(device->bus, device->dev, device->function, reg, value);
}

void init_pci()
{
    printk("> init pci start.\n");
	/*init pci device table*/
	int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		pci_device_table[i].status = PCI_DEVICE_STATUS_INVALID;
	}

	/*scan all pci buses*/
	pci_scan_buses();

	#ifdef _DEBUG_PCI_
	    printk("PCI: device connected number is %d.\n", pic_get_device_connected());
	#endif

    printk("< init pci done.\n");
}

struct pci_device *pci_alloc_device()
{
	int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		if (pci_device_table[i].status == PCI_DEVICE_STATUS_INVALID) {
			pci_device_table[i].status = PCI_DEVICE_STATUS_USING;
			return &pci_device_table[i];
		}
	}
	return NULL;
}

int pci_free_device(struct pci_device *device)
{
	int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		if (&pci_device_table[i] == device) {
			device->status = PCI_DEVICE_STATUS_INVALID;
			return 0;
		}
	}
	return -1;
}

void pci_scan_buses()
{
	uint16 bus;
	uint8 device, function;
	/*扫描每一条总线上的设备*/
    for (bus = 0; bus < PCI_MAX_BUS; bus++) {
        for (device = 0; device < PCI_MAX_DEV; device++) {
           for (function = 0; function < PCI_MAX_FUN; function++) {
				pci_scan_device(bus, device, function);
			}
        }
    }
}

void pci_scan_device(uint8 bus, uint8 device, uint8 function)
{
	/*pci_read vendor id and device id*/
    uint32 val = pci_read(bus, device, function, PCI_DEVICE_VENDER);
    uint16 vendor_id = val & 0xffff;
    uint16 device_id = val >> 16;
	/*if vendor id is 0xffff, it means that this bus , device not exist!*/
    if (vendor_id == 0xffff) {
        return;
    }
	/*pci_read header type*/
    val = pci_read(bus, device, function, PCI_BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE);
    uint8 header_type = ((val >> 16));
	/*pci_read command*/
    val = pci_read(bus, device, function, PCI_STATUS_COMMAND);
   // uint16 command = val & 0xffff;
	/*pci_read class code and revision id*/
    val = pci_read(bus, device, function, PCI_CLASS_CODE_REVISION_ID);
    uint32 classcode = val >> 8;
    uint8 revision_id = val & 0xff;
	
	/*alloc a pci device to store info*/
	struct pci_device *pci_dev = pci_alloc_device();
	if(pci_dev == NULL){
		return;
	}
	/*init pci device*/
    pci_device_init(pci_dev, bus, device, function, vendor_id, device_id, classcode, revision_id, (header_type & 0x80));
	
	/*init pci device bar*/
	int bar, reg;
    for (bar = 0; bar < PCI_MAX_BAR; bar++) {
        reg = PCI_BASS_ADDRESS0 + (bar*4);
		/*pci_read bass address[0~5] to get address value*/
        val = pci_read(bus, device, function, reg);
		/*set 0xffffffff to bass address[0~5], so that if we pci_read again, it's addr len*/
        pci_write(bus, device, function, reg, 0xffffffff);
       
	   /*pci_read bass address[0~5] to get addr len*/
		uint32 len = pci_read(bus, device, function, reg);
        /*pci_write the io/mem address back to confige space*/
		pci_write(bus, device, function, reg, val);
		/*init pci device bar*/
        if (len != 0 && len != 0xffffffff) {
            pci_device_bar_init(&pci_dev->bar[bar], val, len);
        }
    }

	/*get irq line and pin*/
    val = pci_read(bus, device, function, PCI_MAX_LNT_MIN_GNT_IRQ_PIN_IRQ_LINE);
    if ((val & 0xff) > 0 && (val & 0xff) < 32) {
        uint32 irq = val & 0xff;
        pci_dev->irq_line = irq;
        pci_dev->irq_pin = (val >> 8)& 0xff;
    }
	
    #ifdef _DEBUG_PCI_
        printk("pci device at bus: %d, device: %d\n", bus, device);
        pci_device_dump(pci_dev);
	#endif
}

uint32 pci_read(uint32 bus, uint32 device, uint32 function, uint32 addr)
{
	uint32 reg = 0x80000000;
	/*make config add register*/
	reg |= (bus & 0xFF) << 16;
    reg |= (device & 0x1F) << 11;
    reg |= (function & 0x7) << 8;
    reg |= (addr & 0xFF) & 0xFC;	/*bit 0 and 1 always 0*/
	/*pci_write to config addr*/
    io_out32(PCI_CONFIG_ADDR, reg);
    return io_in32(PCI_CONFIG_DATA);	/*return confige addr's data*/
}

void pci_write(uint32 bus, uint32 device, uint32 function, uint32 addr, uint32 val)
{
	uint32 reg = 0x80000000;
	/*make config add register*/
	reg |= (bus & 0xFF) << 16;
    reg |= (device & 0x1F) << 11;
    reg |= (function & 0x7) << 8;
    reg |= (addr & 0xFF) & 0xFC;	/*bit 0 and 1 always 0*/
	
	/*pci_write to config addr*/
    io_out32(PCI_CONFIG_ADDR, reg);
	/*pci_write data to confige addr*/
    io_out32(PCI_CONFIG_DATA, val);
}
struct pci_device* pci_get_device(uint16 vendor_id, uint16 device_id)
{
	int i;
	struct pci_device* device;
	
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		device = &pci_device_table[i];
		if (device->vendor_id == vendor_id && device->device_id == device_id) {
			return device;
		}
	}
    return NULL;
}

void pci_enable_bus_mastering(struct pci_device *device)
{
    uint32 val = pci_read(device->bus, device->dev, device->function, PCI_STATUS_COMMAND);
    #ifdef _DEBUG_PCI_
        printk("before enable bus mastering, command: %x\n", val);
    
    #endif
	val |= 4;
    pci_write(device->bus, device->dev, device->function, PCI_STATUS_COMMAND, val);

    val = pci_read(device->bus, device->dev, device->function, PCI_STATUS_COMMAND);
    #ifdef _DEBUG_PCI_
        printk("after enable bus mastering, command: %x\n", val);

    #endif
}

uint32 pic_get_device_connected()
{
	int i;
	struct pci_device *device;
	for (i = 0; i < PCI_MAX_BAR; i++) {
		device = &pci_device_table[i];
        if (device->status != PCI_DEVICE_STATUS_USING) {
            break;
        }
    }
	return i;
}

#endif //_CONFIG_ARCH_X86_

