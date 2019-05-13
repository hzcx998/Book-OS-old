#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include <stdint.h>
#include <types.h>

/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_G			0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TIG		0
#define	SA_TIL		4

//EFLAGS
#define	EFLAGS_MBS (1<<1)
#define	EFLAGS_IF_1 (1<<9)
#define	EFLAGS_IF_0 0
#define	EFLAGS_IOPL_3 (3<<12)
#define	EFLAGS_IOPL_1 (1<<12)
#define	EFLAGS_IOPL_0 (0<<12)

//index of descriptor
#define	INDEX_DUMMY 0
#define	INDEX_KERNEL_C 1
#define	INDEX_KERNEL_RW 2
#define	INDEX_TSS 3
#define	INDEX_USER_C 4
#define	INDEX_USER_RW 5

#define	INDEX_DRIVER_C 6
#define	INDEX_DRIVER_RW 7

//选择子...
//内核代码，数据，栈，视频

#define KERNEL_CODE_SEL ((INDEX_KERNEL_C << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_DATA_SEL ((INDEX_KERNEL_RW << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_STACK_SEL KERNEL_DATA_SEL 

//用户代码，数据，栈
#define USER_CODE_SEL ((INDEX_USER_C << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_DATA_SEL ((INDEX_USER_RW << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_STACK_SEL USER_DATA_SEL 

//用户代码，数据，栈
#define DRIVER_CODE_SEL ((INDEX_DRIVER_C << 3) + (SA_TIG << 2) + SA_RPL1)
#define DRIVER_DATA_SEL ((INDEX_DRIVER_RW << 3) + (SA_TIG << 2) + SA_RPL1)
#define DRIVER_STACK_SEL DRIVER_DATA_SEL 

//TSS
#define KERNEL_TSS_SEL ((INDEX_TSS << 3) + (SA_TIG << 2) + SA_RPL0)

#define ADR_GDT			0x200000
#define LIMIT_GDT		0x000007ff

//0x90800
#define ADR_IDT			0x200800
#define LIMIT_IDT		0x000007ff

//0x91000

#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a

#define NR_IRQ 16 //irq数量
#define	CASCADE_IRQ	2

#define	IRQ_PORT9	9



//从盘联系irq

/*We will alloc descriptor for tss start at this idx*/
#define DESCRIPTOR_ALLOC_START 10

/*
描述符机构
*/
struct descriptor {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
/*
门结构
*/
struct gate
{
	uint16_t offset_low, selector;
	uint8_t datacount;
	uint8_t attr;		/* P(1) DPL(2) DT(1) TYPE(4) */
	uint16_t offset_high;
};

extern struct gdtr_s gdtr;

extern struct descriptor *gdt;
extern struct gate       *idt;
	

void init_descriptor();
void set_gate_descriptor(struct gate *gd, uint32_t offset, uint32_t selector, uint32_t ar, uint8_t privilege);//设置门描述符
void set_segment_descriptor(struct descriptor *sd, unsigned int limit, int base, int ar);//设置段描述符

//异常
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();
void exception_handler(int esp, int vec_no, int err_code, int eip, int cs, int eflags);

//把中断服处理过程地址放进irq表
void put_irq_handler(int irq, irq_handler_t handler);

/*Alloc a none descriptor.*/
int descriptor_alloc();

void descriptor_free(int idx);

void soft_intr();

#endif