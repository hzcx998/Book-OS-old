/*
File:		device/clock.c
Contains:	driver for clock
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_CLOCK_

#include <sys/arch.h>
#include <sys/dev.h>
#include <sys/mm.h>
#include <sys/core.h>

#include <math.h>

struct clock clock;

struct time time;

/*
clock_switch will ofen use in clock interruption, we statement here
*/

void clock_change_date_time();

void init_clock(void)
{
	printk("> init clock start.\n");
	
	//0.001秒触发一次中断
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, (unsigned char) (TIMER_FREQ/HZ));
	io_out8(PIT_CNT0, (unsigned char) ((TIMER_FREQ/HZ) >> 8));

	clock.last_ticks = clock.ticks = 0;
	
	clock.can_schdule = true;
	
	//用一个循环让秒相等
	do{
		time.year = get_year();
		time.month = get_mon_hex();
		time.day = get_day_of_month();
		
		time.hour = get_hour_hex();
		time.minute =  get_min_hex8();
		time.second = get_sec_hex();
		
		//time.week_day = get_day_of_week();
		
		//trans time to 24 format
		/*if(time.hour >= 16){
			time.hour -= 16;
		}else{
			time.hour += 8;
		}*/
	}while(time.second != get_sec_hex());

	
	init_timer();
	
	put_irq_handler(CLOCK_IRQ, clock_handler);
	
	enable_irq(CLOCK_IRQ);
	printk("< init clock done.\n");
	
	//print_time();
	
}

void clock_handler(int irq)
{
	struct thread *thread = thread_current();
	clock.ticks++;
	
	//有超时的定时器
	if(timer_module->next_ticks <= clock.ticks){
		struct timer *timer;
		timer = timer_module->next_timer; /* とりあえず先頭の番地をtimerに代入 */
		while(1) {
			//没有超时
			if (timer->ticks > clock.ticks) {
				break;
			}
			//变成未激活
			timer->status = TIMER_USING;
			//发生标志
			timer->occured = 1;
			
			timer = timer->next; 
		}
		//设定下一个定时器
		timer_module->next_timer = timer;
		timer_module->next_ticks = timer->ticks;
	}
	if(thread->ticks == 0){
		schedule();
	}else{
		thread->ticks--;
		thread->run_ticks++;
	}
}

int sys_get_ticks()
{
	return clock.ticks;
}

void clock_change_date_time()
{
	time.second++;
	if(time.second >= 60){
		time.minute++;
		time.second = 0;
		if(time.minute >= 60){
			time.hour++;
			time.minute = 0;
			if(time.hour >= 24){
				time.day++;
				time.hour = 0;
				//现在开始就要判断平年和闰年，每个月的月数之类的
				if(time.day > 30){
					time.month++;
					time.day = 1;
					
					if(time.month > 12){
						time.year++;	//到年之后就不用调整了
						time.month = 1;
					}
				}
			}
		}
	}
}

void print_time()
{
	printk("=====Time&Date=====\n");
	printk("Time:%d:%d:%d Date:%d/%d/%d ", \
		time.hour, time.minute, time.second,\
		time.year, time.month, time.day);
	
	/*char *weekday[7];
	weekday[0] = "Monday";
	weekday[1] = "Tuesday";
	weekday[2] = "Wednesday";
	weekday[3] = "Thursday";
	weekday[4] = "Friday";
	weekday[5] = "Saturday";
	weekday[6] = "Sunday";
	printk("weekday:%d", time.week_day);
	*/
}


void ticks_to_sleep(u32 ticks)
{
	u32 t = clock.ticks;
	while(clock.ticks - t < ticks){
		/*sleep*/
		thread_yield();
	}
}

void msec_sleep(u32 msec)
{
	u32 t = DIV_ROUND_UP(msec, 10);
	ticks_to_sleep(t);
}

struct time *sys_gettime(struct time *tm)
{
	if(tm == NULL){
		return &time;
	}else{
		tm->second = time.second;
		tm->minute = time.minute;
		tm->hour = time.hour;
		tm->day = time.day;
		tm->month = time.month;
		tm->year = time.year;
		tm->second = time.second;
		tm->week_day = time.week_day;
		tm->year_day = time.year_day;
		tm->is_dst = time.is_dst;
		return tm;
	}
}

uint32 datetime_to_data()
{
	uint16 d = DATE_TO_DATA16(time.year, time.month, time.day);
	uint16 t =  TIME_TO_DATA16(time.hour, time.minute, time.second);
	return (d<<16) | t;
}

#endif //_CONFIG_DEVIVE_CLOCK_

