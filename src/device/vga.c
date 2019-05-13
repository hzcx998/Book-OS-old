/*
File:		device/ramdisk.c
Contains:	driver for vga
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_VGA_

#include <types.h>
#include <sys/dev.h>
#include <sys/arch.h>
#include <string.h>

uint16_t get_cursor()
{
	uint16_t pos_low, pos_high;		//设置光标位置的高位的低位
	//取得光标位置
	io_out8(CRTC_ADDR_REG, CURSOR_H);			//光标高位
	pos_high = io_in8(CRTC_DATA_REG);
	io_out8(CRTC_ADDR_REG, CURSOR_L);			//光标低位
	pos_low = io_in8(CRTC_DATA_REG);
	
	return (pos_high<<8 | pos_low);	//返回合成后的值
}

void set_cursor(uint16_t cursor_pos)
{
	//设置光标位置 0-2000
	io_cli();
	io_out8(CRTC_ADDR_REG, CURSOR_H);			//光标高位
	io_out8(CRTC_DATA_REG, (cursor_pos >> 8) & 0xFF);
	io_out8(CRTC_ADDR_REG, CURSOR_L);			//光标低位
	io_out8(CRTC_DATA_REG, cursor_pos & 0xFF);
	io_sti();
}

void set_video_start_addr(uint16_t addr)
{
	io_cli();
	io_out8(CRTC_ADDR_REG, START_ADDR_H);
	io_out8(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	io_out8(CRTC_ADDR_REG, START_ADDR_L);
	io_out8(CRTC_DATA_REG, addr & 0xFF);
	io_sti();
}

#endif //_CONFIG_DEVIVE_VGA_
