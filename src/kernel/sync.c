/*
File:		kernel/sync.c
Contains:	semaphore and lock
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/core.h>
#include <sys/debug.h>
#include <sys/arch.h>
#include <sys/mm.h>

void sema_init(struct semaphore *sema, uint8 value)
{
	sema->value = value;
	//init wait list
	thread_chain_init(sema->thread_chain);
}

struct lock *create_lock()
{
	struct lock *lock = (struct lock *)mm.malloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}
	return lock;
}

void lock_init(struct lock *lock)
{
	lock->holder = NULL;
	lock->holder_repeat_nr = 0;
	sema_init(&lock->semaphore, 1);
}

void sema_down(struct semaphore *sema)
{
	int old_status = io_load_eflags();
	io_cli();
	while(sema->value == 0){
		//add 
		thread_chain_put(sema->thread_chain, thread_current());
		thread_block(THREAD_BLOCKED);
	}
	sema->value--;
	assert(sema->value == 0);
	io_store_eflags(old_status);
}

void sema_up(struct semaphore *sema)
{
	int old_status = io_load_eflags();
	io_cli();
	if(!thread_chain_empty(sema->thread_chain)){
		struct thread *thread = thread_chain_get(sema->thread_chain);
		thread_unblock(thread);
	}
	sema->value++;
	assert(sema->value == 1);
	io_store_eflags(old_status);
}

void lock_acquire(struct lock *lock)
{
	if(lock->holder != thread_current()){
		sema_down(&lock->semaphore);
		lock->holder = thread_current();
		assert(lock->holder_repeat_nr == 0);
		lock->holder_repeat_nr = 1;
	}else{
		lock->holder_repeat_nr++;
	}
}


void lock_release(struct lock *lock)
{
	assert(lock->holder == thread_current());
	if(lock->holder_repeat_nr > 1){
		lock->holder_repeat_nr--;
		return;
	}
	lock->holder = NULL;
	lock->holder_repeat_nr = 0;
	sema_up(&lock->semaphore);
}
