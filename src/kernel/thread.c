/*
File:		kernel/thread.c
Contains:	kernel thread 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sys/arch.h>
#include <sys/core.h>
#include <sys/mm.h>
#include <sys/debug.h>
#include <sys/gui.h>
#include <sys/dev.h>

extern int thread_opened;

struct thread thread_table[MAX_THREADS];
struct thread *thread_ready[MAX_THREADS];

uint32 thread_this, thread_runnings, thread_cur_level;

struct thread *main_thread;
struct thread *user_init_thread;
struct thread *idle_thread;

struct thread_bus thread_bus;

static void make_main_thread();
static void kernel_thread(thread_func *function, void *arg);
static void idle();

extern void switch_to(struct thread *cur, struct thread *next);

void thread_a(void *arg);
void thread_b(void *arg);
void process_a(void);
void process_b(void);
void thread_c(void *arg);
int test_va, test_vb;

#define APP_PATH "c:/bosh"

void init_thread()
{
	printk("> init thread start.\n");
	int i;
	//no 0 pid 
	for(i = 0; i < MAX_THREADS; i++){
		thread_table[i].status = THREAD_UNUSED;
		thread_table[i].pid = i+1;
	}
	thread_this = 0;
	thread_runnings = 0;
	thread_cur_level = 0;
	init_tss();
	
	test_va = test_vb = 0;
	
	thread_bus.main = 0;
	thread_bus.idle = 0;
	thread_bus.keyboard = 0;
	thread_bus.mouse = 0;
	thread_bus.timer = 0;
	thread_bus.gui = 0;

	//主线程
	make_main_thread();
	//执行init进程
	//idle进程
	idle_thread = thread_start("idle", 1, idle, NULL);
	
	/*
	初始化设备线程
	*/
	
	thread_opened = 1;
	
	printk("< init thread done.\n");
	
}

void init()
{
	
	//printf("running in init\n");
	execv(APP_PATH, NULL);
/*
	int pipe = pipe_connect();
	printf("connect pipe %d.\n", pipe);

	printf("my pid:%d\n",get_pid());
	printf("my ppid:%d\n",get_ppid());

	char buf[64];
	memset(buf, 0, 64);
	strcpy(buf, "hello, pipe!\n");

	int ret;

	int times = 0;

	while (1) {
		ret = pipe_write(pipe, buf, 64);
		times++;
		if (ret == -1) {
			printf("user pipe write failed!\n");
		} else {
			printf("pipe write done %d!\n", ret);
		}

		if (times == 10) {
			pipe_cancel(pipe);
		}

		if (times == 15) {
			pipe = pipe_connect();
		}

		if (times == 20) {
			pipe_close(pipe);
		}

		if (times > 20) {
			pipe = pipe_connect();
		}
	}
	*/

	/*
	char *argv[3];
	
	argv[0] = "C:/bv";
	argv[1] = "C:/a.jpg";
	*/

	//execv(argv[0], argv[1]);
	
	int status;
	int pid;

	while(1){
		/*等待用户退出的进程*/
		pid = _wait(&status);
		if(pid != -1){
			//printk("init: wait:%d status:%d\n",pid, status);
		}
		
	}
}

void thread_test()
{
	
	thread_start("thread b", 5, thread_b, "arg B");
	/*
	process_execute(process_a, "a");
	process_execute(process_b, "b");
	*/
}


void thread_a(void *arg)
{
	printk("run in thread a\n");
	char *para = arg;
	
	printk(para);
	int i = 0;
	while(1){
		//io_sti();
		i++;
		if(i%0x100000 == 0){
			printk("A ");
		}
	}
}

void thread_b(void *arg)
{
	printk("run in thread b\n");
	char *para = arg;
	
	printk(para);
	int i = 0;
	while(1){
		//io_sti();
		i++;
		if(i%0x100000 == 0){
			printk("B ");
		}
	}
}
void process_a(void)
{
	printf("run in process a\n");
	
	char *a = malloc(32);
	*a = 0xfa;
	
	printf("malloc a:%x data:%x\n", a, *a);
	
	free(a);
	int i = 0;
	while(1){
		i++;
		test_va++;
		if(i%0x100000 == 0){
			printf("%d ", test_va);
		}
	}
}

void process_b(void)
{
	printf("run in process b\n");
	char *b = malloc(40960);
	*b = 0x5a;
	printf("malloc b:%x data:%x\n", b, *b);
	free(b);
	int i = 0;
	while(1){
		i++;
		test_vb++;
		if(i%0x100000 == 0){
			printf("%d ", test_vb);
		}
	}
}

struct thread *alloc_thread()
{
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		if(thread_table[i].status == THREAD_UNUSED){
			return &thread_table[i];
		}
	}
	return NULL;
}


void free_thread(struct thread *thread)
{
	int i;
	struct thread *th;

	for(i = 0; i < MAX_THREADS; i++){
		th = &thread_table[i];
		if(th == thread){
			th->status = THREAD_UNUSED;
		}
	}
}


struct thread *thread_current()
{
	return thread_ready[thread_this];
}

void thread_add(struct thread *thread)
{
	thread_ready[thread_runnings] = thread;
	thread_runnings++;
	
}

struct thread *next_thread()
{
	struct thread *thread;
	thread_this++;
	if(thread_this == thread_runnings){
		thread_this = 0;
	}
	thread = thread_ready[thread_this];
	return thread;
}

struct thread *thread_pick()
{
	struct thread *thread;
	while(1){
		thread = next_thread();
		
		if(thread->status == THREAD_READY){
			break;
		}
	}
	
	return thread;
}

bool thread_ready_empty()
{
	int i;
	struct thread *thread;

	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		if(thread->status == THREAD_READY){
			return false;
		}
	}
	return true;
}

bool thread_find(struct thread *thread)
{
	int i;
	struct thread *th;

	for(i = 0; i < thread_runnings; i++){
		th = thread_ready[i];
		if(th == thread){
			return true;
		}
	}
	return false;
}


void thread_remove(struct thread *thread)
{
	int i;
	//搜索
	for(i = 0; i < thread_runnings; i++){
		if(thread_ready[i] == thread){
			break;
		}
	}
	//调整
	thread_runnings--;
	if(i < thread_this){
		thread_this--;
	}
	//改变位置，覆盖thread
	for(; i < thread_runnings; i++){
		thread_ready[i] = thread_ready[i+1]; 
	}
}

void thread_recover()
{
	int i;
	struct thread *thread;
	
	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
	
		if(thread->status == THREAD_HANGING ){		//挂起状态是线程自己退出
			//printk("found a hanging process:%s\n", thread->name);
			/*回收资源*/
			//release_thread_resource(thread);
			
			/*退出运行*/
			thread_exit(thread);
			
			break;
		}
	}
}

/*
用户态与系统得连接
*/
struct thread *sys_thread_connect()
{
	return thread_table;
}

struct thread *sys_thread_getinfo(struct thread *ptr)
{
	int i;
	
	for(i = 0; i < MAX_THREADS; i++){
		if(thread_table[i].status != THREAD_UNUSED){
			if (&thread_table[i] > ptr) {
				return &thread_table[i];
			}
		}	
	}
	return NULL;
}



void print_threads()
{
	int i;
	struct thread *thread;
	printk("=====Thread info=====\n");
	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		printk("name:%s  pid:%d  status:%d\n", thread->name, thread->pid, thread->status);

	}
}

void thread_chain_init(struct thread_chain *thread_chain)
{
	
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		thread_chain->thread[i] = NULL;
	}
	thread_chain->in = 0;
	thread_chain->out = 0;
}

void thread_chain_put(struct thread_chain *thread_chain, struct thread *thread)
{
	
	thread_chain->thread[thread_chain->in] = thread;
	thread_chain->in++;
	if(thread_chain->in == MAX_THREADS){
		thread_chain->in = 0;
	}
	
}

struct thread *thread_chain_get(struct thread_chain *thread_chain)
{
	struct thread *thread;
	//PICK一个处于ready的任务
	
	thread = thread_chain->thread[thread_chain->out];
	thread_chain->out++;
	if(thread_chain->out == MAX_THREADS){
		thread_chain->out = 0;
	}
	return thread;
}

bool thread_chain_empty(struct thread_chain *thread_chain)
{
	if(thread_chain->in == thread_chain->out){
		return true;
	}
	return false;
}

static void kernel_thread(thread_func *function, void *arg)
{
	io_sti();
	function(arg);
}

void thread_create(struct thread *thread, thread_func function, void *arg)
{
	thread->self_stack -= sizeof(struct intr_stack);
	
	thread->self_stack -= sizeof(struct thread_stack);
	struct thread_stack *thread_stack = (struct thread_stack *)thread->self_stack;
	thread_stack->eip = kernel_thread;
	thread_stack->function = function;
	thread_stack->arg = arg;
	thread_stack->ebp = thread_stack->ebx =\
	thread_stack->esi = thread_stack->edi = 0;
}

void thread_init(struct thread *thread, char *name, int prio)
{
	int pid = thread->pid;
	memset(thread, 0, sizeof(struct thread));
	thread->pid = pid;
	strcpy(thread->name, name);
	if(thread == main_thread){
		thread->status = THREAD_RUNNING;
	}else{
		thread->status = THREAD_READY;
	}
	
	//设置线程的flags为0
	thread->flags = 0;

	thread->kernel_stack = thread->self_stack = (uint8 *)(mm.malloc(PAGE_SIZE*2)+PAGE_SIZE*2);
	thread->priority = prio;
	
	/*改写level*/
	if(prio > thread_cur_level){
		thread_cur_level = prio;
	}
	thread->ticks = prio;
	thread->run_ticks = 0;
	thread->pdir = NULL;
	
	thread->parent_pid = 0;
	thread->bag = NULL;
	
	//default is c:/
	strcpy(thread->cwd, "c:/");
	
	thread->vidbuf = NULL;

	#ifdef _CONFIG_GUI_BOGUI_

	thread->guilib = NULL;
	#endif

	thread->pipe = NULL;

	thread->stack_magic = 0x19980325;
}

struct thread *thread_start(char *name, int prio, thread_func function, void *arg)
{
	struct thread *thread = alloc_thread();

	thread_init(thread, name, prio);

	thread_create(thread, function, arg);
	
	thread_add(thread);
	
	return thread;
} 

static void make_main_thread()
{
	main_thread = alloc_thread();
	thread_init(main_thread, "main", 2);

	thread_add(main_thread);
}

/*
强制关闭一个程序

要保证某个操作后，他不会继续运行了。
要告诉他，下一次，他会退出，下一次他就运行要退出的函数

thread_kill()->thread_block->tread_exit

struct thread *thread;	//要关闭的线程

thread->THREAD_KILL;	//修改成被击杀的状态

查看是否有父进程，有的话就取消父进程的阻塞

监测
如果有击杀的线程，就让它退出

release resource

thread exit


*/
void thread_block(int status)
{
	assert((status == THREAD_BLOCKED) ||\
	(status == THREAD_WAITING) ||\
	(status == THREAD_HANGING));
	
	int old_status = io_load_eflags();
	io_cli();
	struct thread *cur = thread_current();
	cur->status = status;
	schedule();
	io_store_eflags(old_status);
}

void thread_unblock(struct thread *thread)
{
	int old_status = io_load_eflags();
	io_cli();
	
	/*assert((thread->status == THREAD_BLOCKED) ||\
	(thread->status == THREAD_WAITING) ||\
	(thread->status == THREAD_DIED) ||\
	(thread->status == THREAD_HANGING));*/
	
	if(thread->status != THREAD_READY){
		
		thread->status = THREAD_READY;
	}
	io_store_eflags(old_status);
}

int sys_get_pid()
{
	struct thread *cur = thread_current();
	return cur->pid;
}

int sys_get_ppid()
{
	struct thread *cur = thread_current();
	return cur->parent_pid;
}

static void idle()
{
	thread_bus.idle = 1;
	while(1){
		thread_block(THREAD_BLOCKED);
		io_sti();
		io_hlt();
	}
}

void schedule()
{
	struct thread *cur = thread_current();
	
	if(cur->status == THREAD_RUNNING){
		cur->ticks = cur->priority;
		cur->status = THREAD_READY;
	}else{
		
	}
	
	if(thread_ready_empty()){
		thread_unblock(idle_thread);
	}
	struct thread *next;
	
	next = thread_pick();
	
	next->status = THREAD_RUNNING;
	if(next != cur){
		process_activate(next);
	}
		//printk("%s -> %s\n", cur->name, next->name);
	switch_to(cur, next);
	//}
	
}

void thread_yield()
{
	struct thread *cur = thread_current();
	int old_status = io_load_eflags();
	io_cli();
	cur->status = THREAD_READY;
	schedule();
	io_store_eflags(old_status);
}



void thread_init_bitmap(struct thread *thread)
{
	thread->vir_mem_bitmap.bits = (uint8_t*)mm.malloc(VIR_MEM_BITMAP_SIZE);
	thread->vir_mem_bitmap.btmp_bytes_len = VIR_MEM_BITMAP_SIZE;
	bitmap_init(&thread->vir_mem_bitmap);
}

void init_thread_memory_manage(struct thread *thread)
{
	int i;
	/*为结构体分配空间*/
	//thread->memory_manager = (struct memory_manage *)mm.malloc(sizeof(struct memory_manage));

	uint32_t pages = DIV_ROUND_UP(sizeof(struct memory_manage), PAGE_SIZE );
	
	/*
	可能时因为在这个地方释放和分配太大的内存会出错
	所以直接换成page的分配与释放
	*/
	thread->memory_manager = (struct memory_manage *)kernel_alloc_page(pages);
	if(thread->memory_manager == NULL){
		panic("thread memory manage failed!");
	}
	
	thread->memory_manager->pages = pages;

	//printk("mm:%x\n",(int)thread->memory_manager);

	memset(thread->memory_manager, 0, sizeof(struct memory_manage));
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		thread->memory_manager->free_blocks[i].size = 0;	//大小是页的数量
		thread->memory_manager->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void thread_exit(struct thread *thread)
{
	
	int old_status = io_load_eflags();
	io_cli();
	//thread->status = THREAD_DIED;
	
	//从队列中移除
	if(thread_find(thread)){
		thread_remove(thread);
	}
	
	if(thread->pdir != NULL){
		mm.free(thread->pdir);
	}
	
	if(thread != main_thread){
		//printk("release %s\n", thread->name);
		//释放内核栈
		mm.free(thread->kernel_stack - PAGE_SIZE);
		
		//释放结构体
		free_thread(thread);
		
	}
	
	io_store_eflags(old_status);
}

struct thread *pid2thread(int pid)
{
	int i;
	struct thread *thread;
	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		if(thread->pid == pid){
			return thread;
		}
	}
	return NULL;
}

static void release_thread_resource(struct thread *thread)
{
	//释放程序自身占用的储存空间，代码，数据，栈
	uint32 *pdir_vaddr = thread->pdir;
	uint16 user_pde_nr = 1023, pde_idx = 512;
	uint32 pde = 0;
	uint32 *var_pde_ptr = NULL;
	
	uint16 user_pte_nr = 1024, pte_idx = 0;
	uint32 pte = 0;
	uint32 *var_pte_ptr = NULL;

	uint32 *first_pte_vaddr_in_pde = NULL;
	
	uint32 pg_phy_addr = 0;
	//printk("start release space.\n");
	while(pde_idx < user_pde_nr){
		//获取页目录项地址
		var_pde_ptr = pdir_vaddr + pde_idx;
		//获取内容
		pde = *var_pde_ptr;
		if(pde & 0x01){
			//页目录P为1
			first_pte_vaddr_in_pde = pte_ptr(pde_idx*0x400000);
			//获取页表地址
			pte_idx = 0;
			while(pte_idx < user_pte_nr){
				//获取页目表项地址
				var_pte_ptr = first_pte_vaddr_in_pde + pte_idx;
				//获取页表内容
				pte = *var_pte_ptr;
				if(pte & 0x01){
					//页表P为1
					//获取物理页的地址（去掉属性就是地址）
					pg_phy_addr = pte & 0xfffff000;
					free_mem_page(pg_phy_addr);
					//printk("phy:%x\n", pg_phy_addr);
				}
				pte_idx++;
			}
			pg_phy_addr = pde & 0xfffff000;
			free_mem_page(pg_phy_addr);
		}
		pde_idx++;
	}
	
	//释放位图和内存管理

	/*
	可能时因为在这个地方释放和分配太大的内存会出错
	所以直接换成page的分配与释放
	*/
	kernel_free_page((uint32_t)thread->memory_manager, thread->memory_manager->pages);

	//printk("end\n");
	//printk("release bits start ");
	mm.free(thread->vir_mem_bitmap.bits);
	//printk("end\n");
	mm.free(thread->bag);
	
}

void sys_exit(int status)
{
	struct thread *child_thread = thread_current();
	child_thread->exit_status = status;
	//printk("%s exit status %d.\n", child_thread->name, status);

	release_thread_resource(child_thread);

	/*如果有子进程，就把让mian cover来处理*/
	struct thread *orphan_thread;	//孤儿进程，没有父进程

	
	/*
	检测是否有子进程，如果有，就把子进程设置成孤儿
	这样的话，如果子进程就不用等待父进程了，当子进程退出后
	就不会检测父进程，再把父进程激活
	*/
	int i;
	for(i = 0; i < thread_runnings; i++){
		orphan_thread = thread_ready[i];
		if(orphan_thread->parent_pid == child_thread->pid){
			printk("parent %s has child %s\n", child_thread->name, orphan_thread->name);
			//设置为-1，变成孤儿
			orphan_thread->parent_pid = -1;
		}
	}
	/*
	检测是否有父进程，如果有父进程，并且父进程处于等待状态，我们就唤醒父进程
	不然就把父进程设置成-1
	*/
	if(child_thread->parent_pid != -1){
		struct thread *parent_thread = pid2thread(child_thread->parent_pid);
		//printk("parent is %s\n", parent_thread->name);
		
		if(parent_thread->status == THREAD_WAITING){
			//printk("parent wait\n");
			
			//父进程在等他
			thread_unblock(parent_thread);
		}else{
			//printk("parent no wait\n");
			
			//父进程没有等他，让maind的recover来处理
			child_thread->parent_pid = -1;
		}
	}
	//阻塞自己，等待 主进程来释放
	thread_block(THREAD_HANGING);
}

//发出取消的命令，使得线程可以被检测到退出
void thread_cancel(struct thread *thread)
{
	//把线程设置成取消状态
	thread->flags |= THREAD_FLAG_CANCLE;

	//返回继续运行，知道有一个系统调用或者是testcancle运行，就可以检测是否退出
}

//检测是否已经被取消了
int thread_do_testcancel(void)
{
	//获取当前的线程
	struct thread *cur = thread_current();

	//如果已经取消了，就执行里面的退出操作
	if (cur->flags&THREAD_FLAG_CANCLE) {
		return 1;
	}
	//返回，什么也不做
	return 0;
}

/*
击杀一个线程
*/
int sys_thread_kill(int pid)
{
	struct thread *thread;
	int i;
	for(i = 0; i < thread_runnings; i++){
		thread = thread_ready[i];
		//找到
		if(thread->pid == pid){
			//取消线程连接
			thread_cancel(thread);

			//成功
			return 0;
		}
	}
	//没有找到
	return -1;
}

int sys_wait(int32 *status)
{
	struct thread *parent_thread = thread_current();
	struct thread *child_thread;
	int have_child;
	
	int i;
	//printk("parent %s waitting...\n", parent_thread->name);
	while(1){
		have_child = 0;
		//是否有子进程
		for(i = 0; i < thread_runnings; i++){
			child_thread = thread_ready[i];
			if(child_thread->parent_pid == parent_thread->pid){
				have_child = 1;
				//printk("parent %s waitting...\n", parent_thread->name);
				break;
			}
		}
		if(have_child){
			//如果有子进程退出
			if(child_thread->status == THREAD_HANGING){
				printk("child %s exit!\n", child_thread->name);
	
				*status = child_thread->exit_status;
				
				int32 child_pid = child_thread->pid;
				/*
				这里的线程退出，可能不需要
				*/
				thread_exit(child_thread);
				
				return child_pid;
			}else{
				thread_block(THREAD_WAITING);
			}
		}else{
			return -1;
		}
	}
}

/*
如果有子进程就等待，知道子进程退出。
成功返回0，失败返回-1
*/
int sys_wait_child_exit()
{
	struct thread *parent_thread = thread_current();
	struct thread *child_thread;
	int have_child;
	
	int i;

	have_child = 0;
	//是否有子进程
	for(i = 0; i < thread_runnings; i++){
		child_thread = thread_ready[i];
		if(child_thread->parent_pid == parent_thread->pid){
			have_child = 1;
			//printk("parent %s waitting...\n", parent_thread->name);
			break;
		}
	}
	//当没有子进程时才可以争取返回
	if(have_child){
		//如果有子进程退出
		return -1;
	}
	return 0;
}

void thread_graphic_exit(struct thread *thread)
{
	//将当前进程的vidbuf的清空的buffer
	graph_draw_rect(thread->vidbuf->buffer, 0, 0, video_info.width, video_info.height, COLOR_BLACK);
	//释放
	free_vidbuf(thread->vidbuf);
	//默认切换到console
	switch_video_buffer(vidbuf_console);
}
