#ifndef _SYNC_H_
#define	_SYNC_H_

#include <stdint.h>
#include <core/thread.h>

#define RESERVE_SIZE (32+2)

struct semaphore
{
	uint8 value;
	struct thread_chain *thread_chain;
	int reserved[RESERVE_SIZE];
};

struct lock 
{
	struct thread *holder;
	struct semaphore semaphore;
	uint32 holder_repeat_nr;
};
struct lock *create_lock();

void sema_init(struct semaphore *sema, uint8 value);
void lock_init(struct lock *lock);
void sema_down(struct semaphore *sema);
void sema_up(struct semaphore *sema);
void lock_acquire(struct lock *lock);
void lock_release(struct lock *lock);

#endif