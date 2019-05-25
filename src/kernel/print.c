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

/*用来记录线程是否已经打开*/
int thread_opened;

void init_print()
{
	thread_opened = 0;

}


int printk(const char *fmt, ...)
{
	if(thread_opened) lock_acquire(console.lock);
	
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	buffer_of_print(buf, i);
	
	if(thread_opened) lock_release(console.lock);
	
	return i;
}

int buffer_of_print(char* buf, int len)
{
	char* p = buf;
	int i = len;
	while (i) {
		display_char_func(*p++);
		i--;
	}
	return 0;
}

int sys_write_str(char *str)
{
	if(thread_opened) lock_acquire(console.lock);
	
	int len = strlen(str);
	buffer_of_print(str, len);
	if(thread_opened) lock_release(console.lock);
	return len;
}

void sys_writ_char(char ch)
{
	if(thread_opened) lock_acquire(console.lock);
	display_char_func(ch);
	if(thread_opened) lock_release(console.lock);
	
}

