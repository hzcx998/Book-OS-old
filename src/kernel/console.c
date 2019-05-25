/*
File:		kernel/console.c
Contains:	console 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/core.h>
#include <sys/dev.h>
#include <sys/debug.h>
#include <sys/core.h>

struct console console;


struct lock lock_console;

void init_console()
{
	console.vram_addr = 0;
	console.vram_limit = V_MEM_SIZE>>1;
	console.current_start_addr = 0;
	
	console.cursor = 80*4;
	
	console.color = COLOR_DEFAULT;
	
	set_cursor(console.cursor);
	
	//由于在初始化console的时候还没有初始化内存管理，所以不能使用create_lock
	console.lock = &lock_console;
	lock_init(console.lock);

	/*指向文本模式的文字显示*/
	display_char_func = out_char;
	sys_clean_screen = console_clean;
	
	printk(">console\n");
	
}

void out_char(char ch)
{
	uint8_t *vram = (uint8_t *)(V_MEM_BASE + console.cursor *2) ;
	switch(ch){
		case '\n':
			if(console.cursor < console.vram_addr + console.vram_limit - SCREEN_WIDTH){
				console.cursor = console.vram_addr + SCREEN_WIDTH*((console.cursor - console.vram_addr)/SCREEN_WIDTH+1);
			}
			break;
		case '\b':
			if(console.cursor > console.vram_addr){
				console.cursor--;
				*(vram-2) = ' ';
				*(vram-1) = console.color;
			}
			break;
		default: 
			if(console.cursor < console.vram_addr + console.vram_limit - 1){
				*vram++ = ch;
				*vram++ = console.color;
				console.cursor++;
			}
			break;
	}
	while(console.cursor >= console.current_start_addr + SCREEN_SIZE){
		scroll_screen(SCREEN_DOWN);
	}
	flush();
}

void flush()
{
	set_cursor(console.cursor);
	set_video_start_addr(console.current_start_addr);
}

void scroll_screen(int direction)
{
	if(direction == SCREEN_UP){
		if(console.current_start_addr > console.vram_addr){
			console.current_start_addr -= SCREEN_WIDTH;
		}
	}else if(direction == SCREEN_DOWN){
		if(console.current_start_addr + SCREEN_SIZE < console.vram_addr + console.vram_limit){
			console.current_start_addr += SCREEN_WIDTH;
		}
	}
	flush();
}

void console_clean()
{
	console.cursor = console.vram_addr;
	console.current_start_addr = console.vram_addr;
	flush();
	uint8_t *vram = (uint8_t *)(V_MEM_BASE + console.cursor *2) ;
	int i;
	for(i = 0; i < console.vram_limit*2; i+=2){
		*vram = 0;
		vram += 2;
	}
}

void console_gotoxy(int8_t x, int8_t y)
{
	if(x < 0){
		x = 0;
	}
	if(x > SCREEN_WIDTH - 1){
		x = SCREEN_WIDTH - 1;
	}
	
	if(y < 0){
		y = 0;
	}
	if(y > SCREEN_HEIGHT - 1){
		y = SCREEN_HEIGHT - 1;
	}
	console.cursor = console.vram_addr + y*SCREEN_WIDTH+x;
	flush();
}

void console_set_color(uint8_t color)
{
	console.color = color;
}

void console_reset_color()
{
	console.color = COLOR_DEFAULT;
}
