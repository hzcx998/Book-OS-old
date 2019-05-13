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
NASM 		= nasm
CC			= gcc

INCLUDE_PATH = ../../include/

#flags
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

OBJS =  bogui.o \
		container.o \
		image.o \
		window.o \
		message.o \
		taskbar.o \
		libary.o \

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
	