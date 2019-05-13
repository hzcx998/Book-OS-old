#ifndef _IOQUEQUE_H_
#define	_IOQUEQUE_H_

#include <types.h>
#include <core/sync.h>
#include <core/thread.h>

#define IO_QUEUE_BUF_LEN 128

#define IQ_MODE_IDLE 0
#define IQ_MODE_MOVE 1

struct ioqueue {
    struct lock *lock;
    int32_t *buf;			    // 缓冲区大小
    int32_t in;			    // 队首,数据往队首处写入
    int32_t out;			    // 队尾,数据从队尾处读出
	int32_t size;
	struct thread *producer;
	struct thread *consumer;
};

#define IOQUEUE_SIZE sizeof(struct ioqueue)
struct ioqueue* create_ioqueue();
void ioqueue_init(struct ioqueue* ioqueue);

int ioqueue_get(struct ioqueue* ioqueue);
void ioqueue_put(struct ioqueue* ioqueue, int data); 

bool ioqueue_empty(struct ioqueue* ioqueue);
void ioqueue_wakeup(struct thread **waiter);
bool ioqueue_full(struct ioqueue* ioqueue);
void ioqueue_wait(struct thread **waiter);

#endif