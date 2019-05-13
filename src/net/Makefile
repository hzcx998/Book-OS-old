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

INCLUDE_PATH = ../include/

RTL8139_PATH = rtl8139/
AM79C970A_PATH = am79c970a/
AM79C973_PATH = am79c973/
ETHERNET_PATH = ethernet/

#flags
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

OBJS =  net.o\

#First read here
.PHONY: all

all: compile
	
#Compile files
compile: $(OBJS)
	cd $(RTL8139_PATH) && $(MAKE)
	cd $(AM79C970A_PATH) && $(MAKE)
	cd $(AM79C973_PATH) && $(MAKE)
	cd $(ETHERNET_PATH) && $(MAKE)
	
	
	
#Clean temporary files
clean:
	-rm *.o
	cd $(RTL8139_PATH) && $(MAKE) clean
	cd $(ETHERNET_PATH) && $(MAKE) clean
	cd $(AM79C970A_PATH) && $(MAKE) clean
	cd $(AM79C973_PATH) && $(MAKE) clean
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	