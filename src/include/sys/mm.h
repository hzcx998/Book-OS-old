#ifndef _MM_H_
#define _MM_H_

#include <sys/config.h>
#include <stdint.h>


#ifdef _CONFIG_MM_BASIC_
	/*head file*/
	#include <mm/basic/mm.h>

#endif

struct mm_s
{
	void *(*malloc)(uint32_t size);
	int (*free)(void *addr);
};
extern struct mm_s mm;

void init_mm();

void mm_environmente_init();
void mm_interface_init();


#endif //_MM_H_

