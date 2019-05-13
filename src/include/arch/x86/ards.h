#ifndef _ARDS_H_
#define	_ARDS_H_

#include <types.h>
#include <stdint.h>

#define ARDS_ADDR 0x500 //ARDS结构从哪儿开始储存
#define ARDS_NR   (0x500+0x100-4) //记录的ards数量

/*
ards结构体
*/
struct ards
{
	uint32_t base_low;  //基址低32位
	uint32_t base_high;
	uint32_t length_low;  //长度低32位
	uint32_t length_high;			
	uint32_t type;  //该结构的类型(1可以被系统使用)
};

extern struct ards *ards;

void init_ards();  //初始化

#endif