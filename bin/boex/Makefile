#Makefile for boot
#auther: Hu Zicheng
#time: 2019/1/24

#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd

#The file path
INCLUDE_DIR = ../../src/include/
LIB_DIR = ../../src/lib/

BIN = bin

BIN_OFF = 0 
BIN_CNTS = 100

#img dir
IMG_DIR = ../../img/

DISK_IMG = d.img

DISK_VHD = d.vhd

HD_IMG = $(IMG_DIR)$(DISK_IMG)
HD_VHD = $(IMG_DIR)$(DISK_VHD)

#flags
ASM_FLAGS	= -I $(INCLUDE_DIR) -f elf
C_FLAGS	= -I $(INCLUDE_DIR) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

LDFLAGS		= -m elf_i386 -e _start -Ttext 0x80000000

OBJS =  _start.o \
		main.o \
		button.o \
		status_bar.o \
		path_bar.o \
		item.o \
		frame.o \
		view.o \
		scroll_bar.o \



LD_OBJS = $(LIB_DIR)printf.o\
		$(LIB_DIR)malloc.o\
		$(LIB_DIR)free.o\
		$(LIB_DIR)exit.o\
		$(LIB_DIR)getchar.o\
		$(LIB_DIR)putchar.o\
		$(LIB_DIR)write.o\
		$(LIB_DIR)vsprintf.o\
		$(LIB_DIR)string.o\
		$(LIB_DIR)wait.o\
		$(LIB_DIR)execv.o\
		$(LIB_DIR)clear.o\
		$(LIB_DIR)access.o\
		$(LIB_DIR)guia.o\
		$(LIB_DIR)fsa.o\
		$(LIB_DIR)thread.o\
		$(LIB_DIR)pipea.o\
		$(LIB_DIR)pipec.o\
		
#First read here
.PHONY: all

all: compile link img

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS) $(LD_OBJS)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS) $(LD_OBJS)

img:
	$(DD) if=$(BIN) of=$(HD_IMG) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
vhd:
	$(DD) if=$(BIN) of=$(HD_VHD) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
qemu: compile link img
	
vbox: compile link vhd

#Clean temporary files
clean:
	-rm *.o
	-rm bin

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
