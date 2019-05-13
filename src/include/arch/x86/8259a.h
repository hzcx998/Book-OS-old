#ifndef _8259A_H
#define _8259A_H
#include <types.h>

/*
各个端口
*/
#define PIC0_ICW1		0x0020 //主
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0 //从
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

void init_8259a();  //初始化
void enable_irq(int irq);

#endif

