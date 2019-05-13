#####
#Makefile for fs
#File: fs/Makefile
#Contains: fs makefile
#Auther: Hu Zicheng
#Time: 2019/1/24
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail: 2323168280@qq.com
#####

#The tools name
NASM 		= nasm
CC			= gcc

#The file path
INCLUDE_PATH = ../include/

BOFS_PATH = bofs/

#flags
ASM_FLAGS	= -I $(INCLUDE_PATH) -f elf
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

OBJS =  fs.o\

#First read here
.PHONY: all

all: compile

#Compile files
compile: $(OBJS)
	cd $(BOFS_PATH) && $(MAKE)
	
#Clean temporary files
clean:
	-rm *.o
	cd $(BOFS_PATH) && $(MAKE) clean
	
%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	