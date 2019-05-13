#ifndef _TIMER_H_
#define _TIMER_H_
#include <types.h>
#include <core/ioqueue.h>

#define MAX_TIMER_NR 32

enum timer_status
{
	TIMER_UNUSED,
	TIMER_USING,
	TIMER_ACTIVE,
};

/*
#define TIMER_UNUSED 	0
#define TIMER_ALLOCED	1
#define TIMER_ACTIVE	2
*/

#define MAX_TIMER_TICKS	 0xffffff00

struct timer
{
	uint32 ticks;	//到达该定时器的ticks
	enum timer_status status;	//定时器状态
	uint32 occured;
	struct timer *next;	//下一个定时器的结构体指针
};

struct timer_module
{
	uint32 next_ticks;	//下一个即将到达的定时器的ticks
	struct timer *next_timer;	//下一个定时器的指针
	struct timer timer_table[MAX_TIMER_NR];
	struct ioqueue *ioqueue;	//定时器模块的环形缓冲
};
extern struct timer_module *timer_module;

void init_timer();
struct timer *timer_alloc(void);
void timer_free(struct timer *timer);
void timer_setup(struct timer *timer, void (*function)(int), int data);
int timer_settime(struct timer *timer, uint32 timeout);
int timer_cancel(struct timer *timer);

void thread_timer(void *arg);
int timer_occur(struct timer *timer);
void timer_adjust();

#endif

