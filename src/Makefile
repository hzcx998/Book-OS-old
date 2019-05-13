######
#Makefile for BookOS
#File: Makefile
#Contains: makefile for all
#Auther: Hu Zicheng
#Time: 2019/1/20
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail: 2323168280@qq.com
#history:
#-v0.1	2019/1/20
#-v0.2	2019/2/20
######

#tool dir
BOCHS_DIR = ../bochs/

NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
QEMU 		= qemu-system-i386
BOCHS 		= bochs
BOCHSDBG 		= bochsdbg
BXRC 		= $(BOCHS_DIR)bochsrc.bxrc

#img dir
IMG_DIR = ../img/

FLAPPY_IMG 	= $(IMG_DIR)a.img
FLAPPY_IMG2 	= $(IMG_DIR)a2.img
HDA_IMG		= $(IMG_DIR)c.img
HDB_IMG		= $(IMG_DIR)d.img
HDC_IMG		= $(IMG_DIR)e.img
HDD_IMG		= $(IMG_DIR)f.img


#flags
#out a map file: -Map kernel.map
LDFLAGS		= -m elf_i386 -e _start -Ttext 0x100000

#system disk
LOADER_OFF = 2		
LOADER_CNTS = 8

KERNEL_OFF = 10
KERNEL_CNTS = 400

#src dir
BOOT_DIR = boot/
KERNEL_DIR = kernel/
INIT_DIR = init/
LIB_DIR = lib/
DEVICE_DIR = device/
GUI_DIR = gui/
FS_DIR = fs/
NET_DIR = net/
MM_DIR = mm/
ARCH_DIR = arch/

#arch
ARCH_X86_DIR = $(ARCH_DIR)x86/

#fs
BOFS_DIR = $(FS_DIR)bofs/

#gui
GUI_BASIC_DIR = $(GUI_DIR)basic/
BOGUI_DIR = $(GUI_DIR)bogui/

#net
ETHERNET_DIR = $(NET_DIR)ethernet/
RTL8139_DIR = $(NET_DIR)rtl8139/
AM79C970A_DIR = $(NET_DIR)am79c970a/
AM79C973_DIR = $(NET_DIR)am79c973/

#mm
MM_BASIC_DIR = $(MM_DIR)basic/


BOOT_BIN = $(BOOT_DIR)boot.bin
LOADER_BIN = $(BOOT_DIR)loader.bin
KERNEL_ELF = $(KERNEL_DIR)kernel.elf

ARCH_OBJS = $(ARCH_X86_DIR)x86.o \
		$(ARCH_X86_DIR)descriptor.o \
		$(ARCH_X86_DIR)8259a.o \
		$(ARCH_X86_DIR)interrupt.o \
		$(ARCH_X86_DIR)page.o \
		$(ARCH_X86_DIR)kernel.o \
		$(ARCH_X86_DIR)ards.o \
		$(ARCH_X86_DIR)cpu.o \
		$(ARCH_X86_DIR)cmos.o \
		$(ARCH_X86_DIR)tss.o \
		$(ARCH_X86_DIR)pci.o \

KERNEL_OBJS = $(INIT_DIR)main.o \
		$(KERNEL_DIR)_start.o \
		$(KERNEL_DIR)console.o \
		$(KERNEL_DIR)bitmap.o \
		$(KERNEL_DIR)ioqueue.o \
		$(KERNEL_DIR)debug.o \
		$(KERNEL_DIR)syscall.o \
		$(KERNEL_DIR)thread.o \
		$(KERNEL_DIR)sync.o \
		$(KERNEL_DIR)process.o \
		$(KERNEL_DIR)system.o \
		$(KERNEL_DIR)timer.o \
		$(KERNEL_DIR)switch_to.o \
		$(KERNEL_DIR)print.o \
		$(KERNEL_DIR)pipe.o \

MM_OBJS = $(MM_DIR)mm.o \
		$(MM_BASIC_DIR)mm.o \
		
FS_OBJS = $(FS_DIR)fs.o \
		$(BOFS_DIR)inode.o \
		$(BOFS_DIR)dir.o \
		$(BOFS_DIR)dir_entry.o \
		$(BOFS_DIR)bitmap.o \
		$(BOFS_DIR)super_block.o \
		$(BOFS_DIR)drive.o \
		$(BOFS_DIR)bofs.o \
		$(BOFS_DIR)file.o \

GUI_OBJS = $(GUI_DIR)gui.o \
		$(GUI_BASIC_DIR)basic.o \
		$(GUI_BASIC_DIR)graphic.o \
		$(GUI_BASIC_DIR)font.o \
		$(GUI_BASIC_DIR)ft_simsun.o \
		$(GUI_BASIC_DIR)ft_standard.o \
		$(GUI_BASIC_DIR)image.o \
		$(GUI_BASIC_DIR)bmp.o \
		$(GUI_BASIC_DIR)jpeg.o \
		$(BOGUI_DIR)bogui.o \
		$(BOGUI_DIR)container.o \
		$(BOGUI_DIR)image.o \
		$(BOGUI_DIR)window.o \
		$(BOGUI_DIR)message.o \
		$(BOGUI_DIR)taskbar.o \
		$(BOGUI_DIR)libary.o \
		
NET_OBJS = $(NET_DIR)net.o \
		$(RTL8139_DIR)rtl8139.o \
		$(AM79C970A_DIR)am79c970a.o \
		$(AM79C973_DIR)am79c973.o \
		$(ETHERNET_DIR)ethernet.o \
		$(ETHERNET_DIR)net_buf.o \
		$(ETHERNET_DIR)byte_order.o \
		$(ETHERNET_DIR)arp.o \
		
DEVICE_OBJS = $(DEVICE_DIR)vga.o \
		$(DEVICE_DIR)clock.o \
		$(DEVICE_DIR)keyboard.o \
		$(DEVICE_DIR)harddisk.o \
		$(DEVICE_DIR)video.o \
		$(DEVICE_DIR)ramdisk.o \
		$(DEVICE_DIR)mouse.o \
		$(DEVICE_DIR)device.o \

LIB_OBJS = 	$(LIB_DIR)string.o \
		$(LIB_DIR)vsprintf.o \
		$(LIB_DIR)math.o \
		$(LIB_DIR)printf.o \
		$(LIB_DIR)malloc.o \
		$(LIB_DIR)free.o \
		$(LIB_DIR)exit.o \
		$(LIB_DIR)getchar.o \
		$(LIB_DIR)putchar.o \
		$(LIB_DIR)write.o \
		$(LIB_DIR)execv.o \
		$(LIB_DIR)wait.o \
		$(LIB_DIR)fopen.o \
		$(LIB_DIR)fclose.o \
		$(LIB_DIR)fread.o \
		$(LIB_DIR)fwrite.o \
		$(LIB_DIR)fstat.o \
		$(LIB_DIR)lseek.o \
		$(LIB_DIR)unlink.o \
		$(LIB_DIR)opendir.o \
		$(LIB_DIR)closedir.o \
		$(LIB_DIR)readdir.o \
		$(LIB_DIR)rewinddir.o \
		$(LIB_DIR)mkdir.o \
		$(LIB_DIR)rmdir.o \
		$(LIB_DIR)rename.o \
		$(LIB_DIR)copy.o \
		$(LIB_DIR)move.o \
		$(LIB_DIR)getcwd.o \
		$(LIB_DIR)chdir.o \
		$(LIB_DIR)clear.o \
		$(LIB_DIR)ps.o \
		$(LIB_DIR)reboot.o \
		$(LIB_DIR)random.o \
		$(LIB_DIR)access.o \
		$(LIB_DIR)gettime.o \
		$(LIB_DIR)graphic.o \
		$(LIB_DIR)mouse.o \
		$(LIB_DIR)igl.o \
		$(LIB_DIR)video.o \
		$(LIB_DIR)mm.o \
		$(LIB_DIR)thread.o \
		$(LIB_DIR)os.o \
		$(LIB_DIR)lsdir.o \
		$(LIB_DIR)lsdisk.o \
		$(LIB_DIR)guia.o \
		$(LIB_DIR)fsa.o \
		$(LIB_DIR)pipea.o \
		$(LIB_DIR)pipec.o \
		
#objs
OBJS =  $(KERNEL_OBJS) $(ARCH_OBJS) $(FS_OBJS) $(GUI_OBJS) $(DEVICE_OBJS) \
		$(LIB_OBJS) $(NET_OBJS) $(MM_OBJS) \
		
.PHONY: all
	
cld: compile link disk

all: compile link disk qemu


#compile file
compile:
	cd $(BOOT_DIR) && $(MAKE)
	cd $(ARCH_DIR) && $(MAKE)
	cd $(KERNEL_DIR) && $(MAKE)
	cd $(INIT_DIR) && $(MAKE)
	cd $(LIB_DIR) && $(MAKE)
	cd $(DEVICE_DIR) && $(MAKE)
	cd $(GUI_DIR) && $(MAKE)
	cd $(FS_DIR) && $(MAKE)
	cd $(NET_DIR) && $(MAKE)
	cd $(MM_DIR) && $(MAKE)
	
link: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(OBJS)
	
#write file into disk
disk:
	$(DD) if=$(BOOT_BIN) of=$(FLAPPY_IMG) bs=512 count=1 conv=notrunc
	$(DD) if=$(LOADER_BIN) of=$(FLAPPY_IMG) bs=512 seek=$(LOADER_OFF) count=$(LOADER_CNTS) conv=notrunc
	$(DD) if=$(KERNEL_ELF) of=$(FLAPPY_IMG) bs=512 seek=$(KERNEL_OFF) count=$(KERNEL_CNTS) conv=notrunc

#run kernel in vm
bochs:
	$(BOCHS) -q -f $(BXRC)
	
bochsdbg:
	$(BOCHSDBG) -q -f $(BXRC)

qemu:
	$(QEMU) -m 256 -fda $(FLAPPY_IMG)  \
	-hda $(HDA_IMG) -hdb $(HDB_IMG) \
	-boot a

#clean temporary files
clean:
	cd $(BOOT_DIR) && $(MAKE) clean
	cd $(ARCH_DIR) && $(MAKE) clean
	cd $(KERNEL_DIR) && $(MAKE) clean
	cd $(INIT_DIR) && $(MAKE) clean
	cd $(LIB_DIR) && $(MAKE) clean
	cd $(DEVICE_DIR) && $(MAKE) clean
	cd $(GUI_DIR) && $(MAKE) clean
	cd $(FS_DIR) && $(MAKE) clean
	cd $(NET_DIR) && $(MAKE) clean
	cd $(MM_DIR) && $(MAKE) clean
	