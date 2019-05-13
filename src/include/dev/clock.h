#ifndef _DEVICE_CLOCK_H_
#define _DEVICE_CLOCK_H_
#include <types.h>
#include <stdint.h>
#include <time.h>

#define PIT_CTRL	0x0043
//控制端口
#define PIT_CNT0	0x0040
//数据端口
#define TIMER_FREQ     1193180	
#define TIMER_QUICKEN     5
#define HZ             (100*TIMER_QUICKEN)	//1000 快速 100 普通0.001
#define CLOCK_IRQ 0
//时钟中断的irq号

/**
日期和时间的转换
*/
#define TIME_TO_DATA16(hou, min, sec) ((unsigned short)(((hou&0x1f)<<11)|((min&0x3f)<<5)|((sec/2)&0x1f)))

#define DATA16_TO_TIME_HOU(data) ((unsigned int)((data>>11)&0x1f))
#define DATA16_TO_TIME_MIN(data) ((unsigned int)((data>>5)&0x3f))
#define DATA16_TO_TIME_SEC(data) ((unsigned int)((data&0x1f) *2))

#define DATE_TO_DATA16(yea, mon, day) ((unsigned short)((((yea-1980)&0x7f)<<9)|((mon&0xf)<<5)|(day&0x1f)))

#define DATA16_TO_DATE_YEA(data) ((unsigned int)(((data>>9)&0x7f)+1980))
#define DATA16_TO_DATE_MON(data) ((unsigned int)((data>>5)&0xf))
#define DATA16_TO_DATE_DAY(data) ((unsigned int)(data&0x1f))


void IRQ_clock();
//汇编处理
void clock_handler(int irq);

struct clock
{
	
	uint32_t ticks;
	uint32_t last_ticks;
	bool can_schdule;
	
};

extern struct clock clock;
extern struct time time;

void print_time();

void init_clock(void);
void ticks_to_sleep(u32 ticks);
void msec_sleep(u32 msec);

struct time *sys_gettime(struct time *tm);

int sys_get_ticks();

void clock_change_date_time();
uint32 datetime_to_data();

#endif

