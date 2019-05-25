/*
Contains:	System Pipe
Auther:		Dorbmon
Time:		2019/2/24
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		1263592223@qq.com
Modified: Jason Hu (2019.5.11)
*/
#include <stdint.h>
#include <sys/mm.h>
#include <sys/core.h>
#include <stdio.h>
#include <string.h>
/*
管道链表管理结构的指针，在初始化的时候会分配空间给他
*/
struct pipe_manager_s pipe_manager;

void init_pipe()
{
    printk("> init pipe start.\n");
    //初始化基础信息
    pipe_manager.pipe_header = NULL;
    pipe_manager.pipe_number = 0;
    /*int pipe = sys_pipe_create(64);

    char buf[64];

    memset(buf, 0, 64);

    strcpy(buf, "hello, pipe");

    sys_pipe_write(pipe, buf, 64);
    
    memset(buf, 0, 64);

    sys_pipe_read(pipe, buf);
    
    printk("str:%s\n", buf);*/

/*
    int pipe = sys_pipe_create(256);

    pipe_dump(pipe);
    pipe = sys_pipe_create(512);
    
    pipe_dump(pipe);
    pipe = sys_pipe_create(64);
    
    pipe_dump(pipe);

    if (sys_pipe_close(0) == 0) {
        printk("close pipe %d ok!\n", 0);
    }

    if (sys_pipe_close(1) == 0) {
        printk("close pipe %d ok!\n", 1);
    }

    if (sys_pipe_close(2) == 0) {
        printk("close pipe %d ok!\n", 2);
    }

    if (sys_pipe_close(3) == -1) {
        printk("close pipe %d bad!\n", 3);
    }

    pipe = sys_pipe_create(1024);
    pipe_dump(pipe);

    pipe_manager_dump();
*/
    printk("< init pipe done.\n");
}


void pipe_manager_dump()
{
    printk("=====pipe manager=====\n");
    pipe_t *p = pipe_manager.pipe_header;

    printk("pipe number:%d\n", pipe_manager.pipe_number);
    while (p != NULL) {
        printk("id %d, status %d size %d\n", p->pipe_id, p->pipe_status, p->data_size); 
        p = p->next;
    }

}

void pipe_dump(int pipe_id)
{
    pipe_t *p = pipe_manager.pipe_header;

    while (p != NULL) {
        if (p->pipe_id == pipe_id) {
            printk("id %d, status %d size %d\n", p->pipe_id, p->pipe_status, p->data_size);
            break;
        }
        p = p->next;
    }
}
/*
创建一个管道，成功返回管道id，失败返回-1
*/
int32_t sys_pipe_create(uint32_t size)
{   //即时分配
    pipe_t *target;
    //首先判断表中是否有了管道
    if (pipe_manager.pipe_header == NULL) {
        //创建一个管道，并挂载到管道管理的开始
        pipe_manager.pipe_header = (pipe_t *)mm.malloc(sizeof(pipe_t));
        if (pipe_manager.pipe_header == NULL) {
            printk("create pipe failed!\n");
            return -1;
        }
        target = pipe_manager.pipe_header;
        //把下一个设置成空
        pipe_manager.pipe_header->next = NULL;
    }else{
        pipe_t *p = pipe_manager.pipe_header;

        //把p指向最后一个
        while (p->next != NULL) {
            p = p->next;
        }

        p->next = (pipe_t *)mm.malloc(sizeof(pipe_t));

        //分配失败
        if (p->next == NULL) {
            printk("create pipe failed!\n");
            return -1;
        }
        target = p->next;

        p = p->next;
        p->next = NULL;
    }

    //设定pipe的id
    target->pipe_id = pipe_manager.pipe_number;

    //设置成创建状态
    target->pipe_status = PIPE_STATUS_CREATE;

    //分配缓存区
    target->data_size = size;
    target->data_area = mm.malloc(size);
    
    if (target->data_area == NULL) {
        printk("alloc memory for pipe failed!\n");
        //把管道从链表上去除
        pipe_t *p =  pipe_manager.pipe_header;

        //是在链表头
        if (p == target) {
            //释放pipe空间
            mm.free(p);
            //从链表去除
            pipe_manager.pipe_header = NULL;
        } else {

            while (p->next != NULL) {
                //在第二个及其后面找到
                if (p->next == target) {
                    //从链表去除
                    p->next = target->next;    

                    //释放pipe空间
                    mm.free(target);

                    break;
                }

                p = p->next;
            }
        }

        return -1;
    }

    //初始化新建pipe的锁
    target->lock = kernel_malloc(sizeof(struct lock));
    lock_init(target->lock);

    pipe_manager.pipe_number++;

    struct thread *cur = thread_current();
    cur->pipe = target;
    
    /*printk("pipe create id %d, status %d size %d\n", \
        target->pipe_id, target->pipe_status, target->data_size);*/

    //返回创建好的管道的id
    return target->pipe_id;
}

/*
关闭一个pipe，成功返回0，失败返回-1
*/
bool sys_pipe_close(uint32_t pipe_id)
{
    //检测pipe id 是否正确
    if(pipe_manager.pipe_number <= pipe_id){
        return false;
    }

    /*
    判断可否关闭
    */

    //开始遍历，获取管道引用
    pipe_t *p = pipe_manager.pipe_header;
    
    //如果头是空就返回
    if (p == NULL) {
        return false;
    }

    struct thread *cur = thread_current();

    //如果在最前面找到了
    if (p->pipe_id == pipe_id) {
        //头往后移动
        pipe_manager.pipe_header = pipe_manager.pipe_header->next;

        //释放数据区
        mm.free(p->data_area);

        //释放pipe
        mm.free(p);

        cur->pipe = NULL;

        return true;
    }
    pipe_t *target;
    //从第二个开始往后面找
    while (p->next != NULL) {
        if (p->next->pipe_id == pipe_id) {
            target = p->next;

            p->next = target->next;

            //释放数据区
            mm.free(target->data_area);

            //释放pipe
            mm.free(target);

            cur->pipe = NULL;

            return true;
        }
        p = p->next;
    }
    return false;
}
/*
往pipe写入数据
成功返回数据大小
失败返回-1
*/
int32_t sys_pipe_write(uint32_t pipe_id,void* data,uint32_t size)
{
    //检测pipe id 是否正确
    if(pipe_manager.pipe_number <= pipe_id){
        return -1;
    }

    /*
    判断可否写入
    */

    //开始遍历，获取管道引用
    pipe_t *target_pipe = pipe_manager.pipe_header;
    
    while (target_pipe != NULL) {
        if (target_pipe->pipe_id == pipe_id) {
            break;
        }
        target_pipe = target_pipe->next;
    }

    //没有找到对应的pipe
    if (target_pipe == NULL) {
        //printk("pipe write none\n");
        return -1;
    }

    //开始拷贝数据
    if(size > target_pipe->data_size){
        return -1;
    }
    //连接的时候才可以写入数据
    if (target_pipe->pipe_status == PIPE_STATUS_CONNECT) {
        /*struct thread *cur = thread_current();
        printk("pipe write id %d, status %d pid %d ppid %d\n", \
            target_pipe->pipe_id, target_pipe->pipe_status, cur->pid, cur->parent_pid);*/
        
        //上锁
        lock_acquire(&target_pipe->lock);
        memcpy(target_pipe->data_area,data,size);
        //解锁
        lock_release(&target_pipe->lock);

        //设置成有数据状态
        target_pipe->pipe_status = PIPE_STATUS_DATA;

        return size;
    }
    return -1;
}

/*
从pipe读取数据
成功返回数据大小
失败返回-1
*/
int32_t sys_pipe_read(uint32_t pipe_id,void* buffer)
{
    //检测pipe id 是否正确
    if (pipe_manager.pipe_number <= pipe_id) {
        return -1;
    }

    /*
    判断可否读取
    */
     
    //开始遍历，获取管道引用
    pipe_t *target_pipe = pipe_manager.pipe_header;
    
    while (target_pipe != NULL) {
        if (target_pipe->pipe_id == pipe_id) {
            break;
        }
        target_pipe = target_pipe->next;
    }

    //没有找到对应的pipe
    if (target_pipe == NULL) {
        //printk("pipe read none\n");
        return -1;
    }

    //读取的时候要有数据才行
    if (target_pipe->pipe_status == PIPE_STATUS_DATA) {
        /*struct thread *cur = thread_current();

        printk("pipe read id %d, status %d pid %d ppid %d\n", \
            target_pipe->pipe_id, target_pipe->pipe_status, cur->pid, cur->parent_pid);*/
        

        //上锁
        lock_acquire(&target_pipe->lock);
        memcpy(buffer,target_pipe->data_area,target_pipe->data_size);
        
        //memset(target_pipe->data_area, 0, target_pipe->data_size);
        //解锁
        lock_release(&target_pipe->lock);

        //数据读取完后变成连接状态
        target_pipe->pipe_status = PIPE_STATUS_CONNECT;

        return target_pipe->data_size;
    }

    return -1;
}

/*
连接父进程的pipe
连接成功返回pipe id
失败返回-1
*/
int32_t sys_pipe_connect(uint32_t *size)
{
    struct thread *cur = thread_current();

    if (cur->parent_pid != -1) {
        //获取父进程
        struct thread *parent = pid2thread(cur->parent_pid);
        //printk("parent thread %s.\n", parent->name);
        if (parent->pipe != NULL) {
            //printk("thread pipe id %d status %d.\n", parent->pipe->pipe_id, parent->pipe->pipe_status);

            //创建后可以连接，取消后可以重连
            if (parent->pipe->pipe_status == PIPE_STATUS_CREATE || \
                parent->pipe->pipe_status == PIPE_STATUS_CANCEL) {
                //连接成功
                parent->pipe->pipe_status = PIPE_STATUS_CONNECT;

                /*printk("pipe connect id %d, status %d pid %d ppid %d\n", \
                    parent->pipe->pipe_id, parent->pipe->pipe_status, cur->pid, cur->parent_pid);
                */
                *size = parent->pipe->data_size;
                //获取pipe id
                return parent->pipe->pipe_id;
            }
        } else {
            //printk("thread pipe none.\n");
        }
    }
    //连接失败
    return -1;
}

int32_t sys_pipe_cancel(uint32_t pipe_id)
{
    struct thread *cur = thread_current();

    if (cur->parent_pid != -1) {
        //获取父进程
        struct thread *parent = pid2thread(cur->parent_pid);
        //printk("parent thread %s.\n", parent->name);
        if (parent->pipe != NULL) {
            //printk("thread pipe id %d.\n", parent->pipe->pipe_id);

            //要变成连接状态时才能取消，如果有数据，那就得等待一会儿
            if (parent->pipe->pipe_status == PIPE_STATUS_CONNECT) {
                //连接成功
                parent->pipe->pipe_status = PIPE_STATUS_CANCEL;

                //获取pipe id
                return parent->pipe->pipe_id;
            }
        } else {
            //printk("thread pipe none.\n");
        }
    }
    //连接失败
    return -1;
}

pipe_t *sys_id_to_pipe(uint32_t pipe_id)
{
    pipe_t *target_pipe = pipe_manager.pipe_header;
    while (target_pipe != NULL) {
        if (target_pipe->pipe_id == pipe_id) {
            return target_pipe;
        }
        target_pipe = target_pipe->next;
    }
    return NULL;
}