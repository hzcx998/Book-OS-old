#ifndef _DEVICE_MOUSE_H_
#define _DEVICE_MOUSE_H_

#include <types.h>
#include <stdint.h>

#include <sys/core.h>

#define MOUSE_IRQ 12
//鼠标irq

#define	MOUSE_FIFO_BUF_LEN	256
//鼠标缓冲区

/*mouse key left,right,middle*/
#define	MOUSE_LEFT	1
#define	MOUSE_RIGHT	2
#define	MOUSE_MIDDLE 3
/*mouse state idle(nothing),down,up*/
#define	MOUSE_IDLE	0
#define	MOUSE_DOWN	1
#define	MOUSE_UP	2

#define	MOUSE_CLICK_TIME	5*1000	//1000 = 1秒

#define	MOUSE_NORMAL	0
#define	MOUSE_HOLD		1

/*
鼠标结构体
*/
struct mouse {
	unsigned char read_buf[3], phase;
	int x_increase, y_increase, button;
	int x, y;
	int old_x, old_y;
	int new_x, new_y;
	int key_left, key_right, key_middle;//鼠标按键状态
	
	int key_left_continue, key_right_continue, key_middle_continue;//鼠标按键状态
	
	int key_occur;
	
	int rf_x0,rf_y0,rf_x1,rf_y1;	//刷新区域
	bool active;	//是否有鼠标行动
	
	int status;
	struct ioqueue *ioqueue;
};

extern struct mouse mouse;
//全局变量

void IRQ_mouse();
//汇编处理
void mouse_handler(int32_t irq);
void init_mouse();
void enable_mouse(struct mouse *mouse);
int mouse_read(struct mouse *mouse, unsigned char data);

void thread_mouse(void *arg);
void mouse_analysis();

int32_t get_mouse_x();
int32_t get_mouse_y();


void mouse_main();

int sys_get_mouse_position(int *x, int *y);
void sys_get_mouse_button(char buf[]);

#endif

