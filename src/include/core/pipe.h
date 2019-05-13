#ifndef _KERNEL_PIPE_H
#define _KERNEL_PIPE_H

#include <stdint.h>

#include <sys/core.h>

enum pipe_status_e
{
    PIPE_STATUS_CREATE,     //创建后
    PIPE_STATUS_CONNECT,    //连接后
    PIPE_STATUS_DATA,       //有数据
    PIPE_STATUS_CANCEL,     //有数据
};

typedef struct pipe_s
{
    uint32_t pipe_id;   //管道的id
    enum pipe_status_e pipe_status;   //管道的状态
    uint32_t data_size; //数据的大小
    void* data_area;    //数据区域
    struct pipe_s *next;  //下一个pipe指针
    struct lock lock;   //管道的锁
}pipe_t;

/*
管道链表管理
*/
struct pipe_manager_s
{
    struct pipe_s *pipe_header;        //管道链表头         
    uint32_t pipe_number;   //管道数量
}pipe_manager_t;

void init_pipe();
void pipe_manager_dump();
void pipe_dump(int pipe_id);
int32_t sys_pipe_create(uint32_t size);
int32_t sys_pipe_close(uint32_t pipe_id);
int32_t sys_pipe_write(uint32_t pipe_id,void* data,uint32_t size);
int32_t sys_pipe_read(uint32_t pipe_id,void* buffer);
int32_t sys_pipe_connect(uint32_t *size);
int32_t sys_pipe_cancel(uint32_t pipe_id);


pipe_t *sys_id_to_pipe(uint32_t pipe_id);

#endif