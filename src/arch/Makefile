#####
#Makefile for gui
#File: gui/Makefile
#Contains: gui makefile
#Auther: Hu Zicheng
#Time: 2019/1/24
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail:		2323168280@qq.com
#####


#The tools name
CC			= gcc

INCLUDE_PATH = ../include/

X86_PATH = x86/
ARM_PATH = arm/

#flags
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

#OBJS =  gui.o\
	
#First read here
.PHONY: all

all: compile
	
#Compile files
compile: 
	cd $(X86_PATH) && $(MAKE)
	
#Clean temporary files
clean:
	-rm *.o
	cd $(X86_PATH) && $(MAKE) clean
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	