/*
File:		kernel/ards.c
Contains:	ards struct read
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <sys/arch.h>
#include <sys/mm.h>
#include <sys/core.h>

struct ards *ards;
void init_ards()
{
	uint16_t ards_nr =  *((uint16_t *)ARDS_NR);	//ards 结构数
	ards = (struct ards *) ARDS_ADDR;	//ards 地址
	int i;
	for(i = 0; i < ards_nr; i++){
		//寻找可用最大内存
		if(ards->type == 1){
			//冒泡排序获得最大内存
			if(ards->base_low+ards->length_low > memory_total_size){
				memory_total_size = ards->base_low+ards->length_low;
			}
		}
		ards++;
	}
	
	printk(">memory size:%x bytes %d mb\n", memory_total_size, memory_total_size/MB);
	
}


#endif //_CONFIG_ARCH_X86_


