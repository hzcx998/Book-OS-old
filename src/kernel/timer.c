/*
Contains:	system timer
Auther:		Jason Hu
Time:		2019/3/5
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/dev.h>
#include <sys/mm.h>
#include <sys/arch.h>
#include <sys/core.h>
#include <sys/gui.h>
#include <stdio.h>

/*
@
我添加了这个问答的地方，来帮助开发者快速理解并使用这里面的函数。
希望大家以后可以借鉴这类方法，多写点注释，好让其他人能搞快速了解
你完成的内容。^-^
@
Q:如何使用定时器？
A:首先，先分配一个定时器，并用一个指针变量保存它。
	struct timer *timer = timer_alloc();
	现在只是分配了，但是系统还不能识别它，所以还要设定它的超时时间
	timer_settime(timer, 超时时间(100=1秒));
	这样，系统就会在到达超时的时候告诉我们，这个定时器超时了
Q:怎么知道定时器超时？
A:用timer_occur(timer);函数来检测，如果返回值为1就说明这个定时器超时了，
	如果为0，说明定时器还没有发生。
Q:如果不想使用某个定时器了，怎么办？
A:不想用，就把定时器取消就是了，timer_cancel(timer);就可以取消定时器
	但，这个只是说系统不检测它，它依然处于分配状态，如果要彻底取消它，
	就调用timer_free(timer);把它释放掉，他就可以被再次分配
*/

/*定义成指针，在内存中分配空间*/
struct timer_module *timer_module;

/*
@函数名：	init_timer
@参数：	无
@功能：	初始化定时器模块
@作者：	香橙子
@日期：	2019/3/5
@返回值：	无
*/
void init_timer()
{
	//为定时器模块分配内存
	timer_module = mm.malloc(sizeof(struct timer_module));
	
	int i;
	for(i = 0; i < MAX_TIMER_NR; i++){
		timer_module->timer_table[i].status = TIMER_UNUSED;
	}
	
	//分配一个定时器作为最后的定时器
	struct timer *timer = timer_alloc();
	//设置成最大的定时器
	timer->ticks = MAX_TIMER_TICKS;	
	timer->status = TIMER_ACTIVE;	//设置成激活
	timer->next = NULL;

	timer_module->next_timer = timer;
	timer_module->next_ticks = timer->ticks;
	
	//初始化定时器的缓冲区
	timer_module->ioqueue = create_ioqueue();
	ioqueue_init(timer_module->ioqueue);
}
/*
@函数名：	thread_timer
@参数：	传入参数
@功能：	定时器线程，用于判断系统中需要使用的定时器
@作者：	香橙子
@日期：	2019/3/5
@返回值：	线程不允许返回
*/
void thread_timer(void *arg)
{
	//printk("running in clock\n");
	
	int time_x, time_y;
	int time_width = 5*8;

	time_x = bogui_taskbar.width - time_width - 16;
	time_y = 4;
	
	char s[40];

	thread_bus.timer = 1;
	//分配一个用于时钟的定时器
	struct timer *timer_clock = timer_alloc();
	//设定时间为1秒=100ticks
	timer_settime(timer_clock, 100);
	while(1){
		//如果定时器发生超时
		if(timer_occur(timer_clock)){
			//在设置一次，下次超时使用，达到无限使用
			timer_settime(timer_clock, 100);
			//执行具体函数
			clock_change_date_time();

			//显示在任务栏上面
			sprintf(s, "%d:%d", time.hour, time.minute);
			//绘制背景
			bogui_container_draw_rect(bogui_taskbar.container, time_x, time_y, time_width, 16, bogui_taskbar.bcolor);
			bogui_container_draw_string(bogui_taskbar.container, time_x, time_y, s, bogui_taskbar.ftcolor);
			
			bogui_container_refresh(bogui_taskbar.container, time_x, time_y, time_x+time_width, time_y + 16);
		}
	}
}

/*
@函数名：	timer_alloc
@参数：	无
@功能：	分配一个定时器
@作者：	香橙子
@日期：	2019/3/5
@返回值：	timer=获取一个定时器指针，NULL=分配失败
*/
struct timer *timer_alloc(void)
{
	int i;
	
	for (i = 0; i < MAX_TIMER_NR; i++) {
		//如果定时器状态时没有使用
		if (timer_module->timer_table[i].status == TIMER_UNUSED) {
			//把它设置为使用中
			timer_module->timer_table[i].status = TIMER_USING;
			//返回它的指针
			return &timer_module->timer_table[i];
		}
	}
	//没有空闲的定时器
	return NULL;
}
/*
@函数名：	timer_free
@参数：	timer=要操作的定时器
@功能：	释放一个定时器，让它可以重新被分配
@作者：	香橙子
@日期：	2019/3/5
@返回值：	无
*/
void timer_free(struct timer *timer)
{
	timer->status = TIMER_UNUSED; //状态设置成未使用，就可以重新分配
}

/*
@函数名：	timer_settime
@参数：	timer=要操作的定时器，timeout=要设置多少ticks
@功能：	取消一个定时器
@作者：	香橙子
@日期：	2019/3/5
@返回值：	成功返回0，失败返回-1
*/
int timer_settime(struct timer *timer, uint32 timeout)
{
	int old_status;
	struct timer *next_timer, *last_timer;
	//设置定时器的ticks
	timer->ticks = clock.ticks + timeout*TIMER_QUICKEN;
	timer->status = TIMER_ACTIVE;	//变成激活状态
	timer->occured = 0;	//没有超时
	//先保留原来的eflags状态，并且关闭中断
	old_status = io_load_eflags();
	io_cli();
	//指向即将到来的定时器
	next_timer = timer_module->next_timer;
	if (timer->ticks <= next_timer->ticks) {
		//插入最前面
		timer_module->next_timer = timer;	//更改即将到来的定时器
		timer->next = next_timer; //把设定的定时器的下一个指向上一个那个定时器
		timer_module->next_ticks = timer->ticks;	//设定修改后的最新next_ticks
		io_store_eflags(old_status);	//恢复之前的状态
		return 0;
	}
	//插入在中间
	for (;;) {
		last_timer = next_timer;	//保存上一个定时器
		next_timer = next_timer->next;	//切换到下一个定时器
		if (timer->ticks <= next_timer->ticks) {
			//找到后面的定时器
			last_timer->next = timer;	//上一个定时器的下一个指向要设定的定时器
			timer->next = next_timer;	//要设定的定时器的下一个指向原有的下一个定时器
			io_store_eflags(old_status);	//恢复之前的状态
			return 0;
		}
	}
	return -1;
} 

/*
@函数名：	timer_cancel
@参数：	timer=要取消的定时器
@功能：	取消一个定时器
@作者：	香橙子
@日期：	2019/3/5
@返回值：	成功返回0，失败返回-1
*/
int timer_cancel(struct timer *timer)
{

	struct timer *next_timer;
	int old_status = io_load_eflags();
	io_cli();
	//只能取消已经激活的定时器
	if (timer->status == TIMER_ACTIVE) {
		//如果要取消的定时器在定时器队列的最前面
		if (timer == timer_module->next_timer) {
			//改变指针指向
			next_timer = timer->next;
			timer_module->next_timer = next_timer;
			timer_module->next_ticks = next_timer->ticks;
		} else {
			
			next_timer = timer_module->next_timer;
			for (;;) {
				if (next_timer->next == timer) {
					break;
				}
				next_timer = next_timer->next;
			}
			//让下一个定时器指向要取消的定时器的下一个，这样要取消的定时器就从中去掉了
			next_timer->next = timer->next;
		}
		//设置状态
		timer->status = TIMER_USING;
		io_store_eflags(old_status);
		return 0;
	}
	io_store_eflags(old_status);
	return -1;
}

/*
@函数名：	timer_occur
@参数：	timer=要操作的定时器
@功能：	检测一个定时器是否超时
@作者：	香橙子
@日期：	2019/3/5
@返回值：	0=没有发生，1=发生
*/
int timer_occur(struct timer *timer)
{
	//如果超时，occured会变成1
	if(timer->occured == 1){
		timer->occured = 0;
		return 1;
	}
	return 0;
}

