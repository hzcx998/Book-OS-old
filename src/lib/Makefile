#####
#Makefile for lib
#File: lib/Makefile
#Contains: lib makefile
#Auther: Hu Zicheng
#Time: 2019/1/24
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail:		2323168280@qq.com
#####

#The tools name
NASM 		= nasm
CC			= gcc

#The file path
INCLUDE_PATH = ../include/

#flags
ASM_FLAGS	= -I $(INCLUDE_PATH) -f elf
C_FLAGS	= -I $(INCLUDE_PATH) -c -fno-builtin -Wall -Wunused -m32 -fno-stack-protector

OBJS =  string.o\
		vsprintf.o\
		math.o\
		write.o\
		malloc.o\
		free.o\
		exit.o\
		getchar.o\
		putchar.o\
		printf.o\
		execv.o\
		wait.o\
		fopen.o\
		fclose.o\
		fread.o\
		fwrite.o\
		fstat.o\
		lseek.o\
		unlink.o\
		opendir.o\
		closedir.o\
		readdir.o\
		rewinddir.o\
		mkdir.o\
		rmdir.o\
		rename.o\
		copy.o\
		move.o\
		getcwd.o\
		chdir.o\
		clear.o\
		ps.o\
		reboot.o\
		random.o\
		access.o\
		gettime.o\
		graphic.o\
		mouse.o\
		igl.o\
		video.o\
		mm.o\
		thread.o\
		os.o\
		lsdir.o\
		lsdisk.o\
		guia.o\
		fsa.o\
		pipea.o\
		pipec.o\
		timer.o\
		ctype.o\
		
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
%.o: %.cpp
	$(CC) $(C_FLAGS) -o $@ $<