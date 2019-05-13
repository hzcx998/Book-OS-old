#####
#Makefile for net
#File: net/Makefile
#Contains: net makefile
#Auther: Hu Zicheng
#Time: 2019/4/10
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail:		2323168280@qq.com
#####

#The tools name
NASM 		= nasm
CC			= gcc

INCLUDE_PATH = ../../include/

#flags
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

OBJS =  ethernet.o\
		net_buf.o\
		byte_order.o\
		arp.o\
		
#First read here
.PHONY: all

all: compile
	
#Compile files
compile: $(OBJS)
	
#Clean temporary files
clean:
	-rm *.o
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	