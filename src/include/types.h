#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef NULL
	#define NULL ((void*)0)
#endif

#define bool int
#define true 1
#define false 0

typedef void (*irq_handler_t) (int irq);
typedef void* sys_call_t;

#endif  /*_TYPES_H_*/