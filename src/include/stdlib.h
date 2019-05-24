#ifndef _STDLIB_H_
#define _STDLIB_H_
#include <stdint.h>

#include <core/timer.h>
#include <core/thread.h>

void *malloc(int size);
void free(void *ptr);
void *realloc(void *mem_address, unsigned int newsize);

void get_memory(int *size, int *free);

int execv(char *path, char *argv[]);

void exit(int status);
int _wait(int *status);
int wait_child_exit();

void clear();
void ps();

void reboot(int reboot_type);

int rand();
void srand(unsigned int seed);

int get_pid();
int get_ppid();
int get_ticks();
void sleep(u32 msec);

struct thread *thread_connect();
struct thread *thread_getinfo(struct thread *ptr);
int thread_testcancel(void);

int thread_kill(int pid);

int32_t pipe_create(uint32_t size);
int32_t pipe_close(uint32_t pipe_id);
int32_t pipe_write(uint32_t pipe_id,void* data,uint32_t size);
int32_t _pipe_write(uint32_t pipe_id,void* data,uint32_t size);
int32_t pipe_read(uint32_t pipe_id,void* buffer);
int32_t pipe_connect(uint32_t *size);
int32_t pipe_cancel(uint32_t pipe_id);
int32_t _pipe_cancel(uint32_t pipe_id);

timer_t *talloc(void);
void tfree(timer_t *timer);
int settimer(timer_t *timer, uint32 timeout);
int tcancel(timer_t *timer);
int toccur(timer_t *timer);


#endif
