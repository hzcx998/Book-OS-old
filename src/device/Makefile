######
#Makefile for device
#File: device/Makefile
#Contains: device makefile
#Auther: Hu Zicheng
#Time: 2019/1/29
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail: 2323168280@qq.com
######

#The tools name
NASM 		= nasm
CC			= gcc

#The file path
INCLUDE_PATH = ../include/

#flags
ASM_FLAGS	= -I $(INCLUDE_PATH) -f elf
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -std=c99 -m32 -fno-stack-protector

OBJS =  clock.o\
		keyboard.o\
		harddisk.o\
		video.o\
		ramdisk.o\
		mouse.o\
		vga.o\
		device.o\
		
		
		
		
		
#First read here
.PHONY: all

all: compile

#Compile files
compile: $(OBJS)

#Clean temporary files
clean:
	-rm *.o

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	