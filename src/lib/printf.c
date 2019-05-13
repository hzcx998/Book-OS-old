#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int printf(const char *fmt, ...)
{
	//int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
	return write(buf);
}

int speak(const char *fmt, ...)
{
	uint32_t size;
	//连接到管道
	int id = pipe_connect(&size);
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	//写入数据
	pipe_write(id, buf, size);
	//取消管道
	pipe_cancel(id);
	return i;
}


