/*
File:		net/rtl8139/rtl8139.c
Contains:	net module
Auther:		Hu Zicheng
Time:		2019/4/10
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>


#include <sys/core.h>
#include <sys/mm.h>

#include <string.h>

struct mm_s mm;

void init_mm()
{
	/*ethernet environment init*/
	mm_environmente_init();
	
	/*external interface init*/
	mm_interface_init();
	
}

void mm_environmente_init()
{

	#ifdef _CONFIG_MM_BASIC_
		memory_init();
	#endif
}

void mm_interface_init()
{
	/*interface set*/
	#ifdef _CONFIG_MM_BASIC_
		mm.malloc = kernel_malloc;
		mm.free = kernel_free;
	#endif
	
}

