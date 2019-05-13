#ifndef _PROCESS_H_
#define	_PROCESS_H_

#include <types.h>
#include <stdint.h>
#include <core/thread.h>

#define USER_STACK3_ADDR 0xffbff000
#define USER_START_ADDR 0X80000000

#define PATH_LEN 128

#define STACK_ARGC_MAX 32

void process_activate(struct thread *thread);
void page_dir_activate(struct thread *thread);
void start_process(void *filename);

uint32 *create_page_dir();
int process_execute(void *filename, char *name);

int sys_execv(char *path, char *argv[]);

void load_process(void *path);
int32_t make_arguments(char *buf, char **argv);

#endif