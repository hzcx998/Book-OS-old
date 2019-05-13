#####
#Makefile for bofs
#File: fs/bofs/Makefile
#Contains: bofs makefile
#Auther: Hu Zicheng
#Time: 2019/3/24
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail: 2323168280@qq.com
#####

#The tools name
NASM 		= nasm
CC			= gcc

#The file path
INCLUDE_PATH = ../../include/

#flags
ASM_FLAGS	= -I $(INCLUDE_PATH) -f elf
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

OBJS =  inode.o\
		dir.o\
		dir_entry.o\
		super_block.o\
		bitmap.o\
		drive.o\
		bofs.o\
		file.o\

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
	