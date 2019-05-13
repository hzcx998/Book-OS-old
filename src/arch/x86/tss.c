/*
File:		kernel/tss.c
Contains:	init tss
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <string.h>

#include <sys/arch.h>
#include <sys/core.h>

struct tss tss;

void init_tss()
{
	printk("> init tss start.\n");
	
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = KERNEL_DATA_SEL;
	tss.iobase = sizeof(tss);
	load_tr(KERNEL_TSS_SEL);
	printk("< init tss done.\n");
}

/*
	把任务的页目录表放在tss中，以后便于写入
*/
void update_tss_esp(struct thread *thread)
{
	tss.esp0 = (uint32 *)thread->kernel_stack;
	
}

#endif //_CONFIG_ARCH_X86_

