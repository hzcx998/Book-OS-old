#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef NULL
#ifdef __cplusplus
        #define NULL 0
#else
        #define NULL ((void *)0)
#endif
#endif


#ifndef __cplusplus
#define bool _Bool      //C语言下实现Bool
#define true 1
#define false 0
#endif

typedef void (*irq_handler_t) (int irq);
typedef void* sys_call_t;

#endif  /*_TYPES_H_*/
