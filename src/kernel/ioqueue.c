/*
File:		kernel/ioqueue.c
Contains:	ioqueue
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/core.h>
#include <string.h>
#include <sys/mm.h>
#include <sys/debug.h>

struct ioqueue* create_ioqueue()
{
	return (struct ioqueue* )mm.malloc(IOQUEUE_SIZE);
}

void ioqueue_init(struct ioqueue* ioqueue)
{
	//初始化队列锁
	ioqueue->lock = create_lock();
	lock_init(ioqueue->lock);
	//初始化缓冲区
	ioqueue->buf = (int32_t *)mm.malloc(IO_QUEUE_BUF_LEN*4);
	
	memset(ioqueue->buf, 0, IO_QUEUE_BUF_LEN*4);
	ioqueue->in = ioqueue->out = 0;
	ioqueue->size = 0;
	
	ioqueue->producer = ioqueue->consumer = NULL;
}

int ioqueue_get(struct ioqueue* ioqueue)
{
	/*if empty, wait*/
	while(ioqueue_empty(ioqueue)){
		lock_acquire(ioqueue->lock);
		ioqueue_wait(&ioqueue->consumer);
		lock_release(ioqueue->lock);
	}
	int data = ioqueue->buf[ioqueue->out];
	
	ioqueue->out++;	//改变指针位置
	ioqueue->size--;	//数据数量减少
	//修复越界
	if(ioqueue->out >= IO_QUEUE_BUF_LEN){
		ioqueue->out = 0;
	}
	/*wakeup producer, we let him produce*/
	if(ioqueue->producer != NULL){
		ioqueue_wakeup(&ioqueue->producer);
	}
	return data;
}

void ioqueue_put(struct ioqueue* ioqueue, int data)
{
	/*if full, wait, we can't put*/
	while(ioqueue_full(ioqueue)){
		lock_acquire(ioqueue->lock);
		ioqueue_wait(&ioqueue->producer);
		lock_release(ioqueue->lock);
	}
	
	//如果最大了就直接覆盖原有信息
	ioqueue->buf[ioqueue->in] = data;
	
	ioqueue->in++;	//改变指针位置
	ioqueue->size++;	//数据数量增加
	//修复越界
	if(ioqueue->in >= IO_QUEUE_BUF_LEN){
		ioqueue->in = 0;
	}
	/*wakeup consumer, we have data to use*/
	if(ioqueue->consumer != NULL){
		ioqueue_wakeup(&ioqueue->consumer);
	}
}

bool ioqueue_empty(struct ioqueue* ioqueue)
{
	return (ioqueue->size == 0) ? 1:0;	//如果大小为0就是空的
}

bool ioqueue_full(struct ioqueue* ioqueue)
{
	return (ioqueue->size == IO_QUEUE_BUF_LEN) ? 1:0;	//如果大小为0就是空的
}

void ioqueue_wait(struct thread **waiter)
{
	assert(*waiter == NULL && waiter != NULL);
	*waiter = thread_current();
	thread_block(THREAD_BLOCKED);
}

void ioqueue_wakeup(struct thread **waiter)
{
	assert(*waiter != NULL);
	thread_unblock(*waiter);
	*waiter = NULL;
}
