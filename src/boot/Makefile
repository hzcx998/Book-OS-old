######
#Makefile for boot
#File: boot/Makefile
#Contains: boot makefile
#Auther: Hu Zicheng
#Time: 2019/1/29
#Copyright:	(C) 2018-2019 by Retal developers. All rights reserved.
#E-mail: 2323168280@qq.com
######

#The tools name
NASM 		= nasm

#The file path
BOOT_BIN = boot.bin
LOAD_BIN = loader.bin

#First read here
.PHONY: all

all: clean compile

#Compile files
compile: $(BOOT_BIN) $(LOAD_BIN) 

#Clean temporary files
clean:
	-rm *.bin

$(BOOT_BIN): boot.asm
	$(NASM) -o $@ $<
	
$(LOAD_BIN): loader.asm
	$(NASM) -o $@ $<
