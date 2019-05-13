######
#Makefile for kernel
#File: kernel/Makefile
#Contains: kernel makefile
#Auther: Hu Zicheng
#Time: 2019/1/24
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
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

OBJS =  _start.o\
		console.o\
		bitmap.o\
		ioqueue.o\
		debug.o\
		syscall.o\
		thread.o\
		sync.o\
		process.o\
		system.o\
		timer.o\
		switch_to.o\
		print.o\
		pipe.o\
		
		
		
		
		
		
#First read here
.PHONY: all

all: compile

#Compile files
compile: $(OBJS)

#Clean temporary files
clean:
	-rm *.o
	-rm kernel.elf

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	