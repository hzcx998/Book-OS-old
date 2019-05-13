/*
File:		kernel/descriptor.c
Contains:	gdt and idt descriptor
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <sys/arch.h>
#include <sys/dev.h>
#include <sys/core.h>
#include <sys/net.h>


#include <string.h>

irq_handler_t irq_table[NR_IRQ];
void default_irq_handler(int irq);

struct descriptor *gdt;
struct gate       *idt;
	
extern void intrrupt_sys_call();

void IRQ_port9();


void init_descriptor()
{
	/*获取gdt得信息*/
	//gdtr.limit = (short)(*(&gdt_limit));
	//gdtr.addr = (int)(*(&gdt_addr));
	//gdtr.addr += 0x80000000;
	//put_str("gdt address:%x\n",gdtr.addr);
	
	//memcpy((void *)ADR_GDT, (void *)gdtr.addr, gdtr.limit);
	gdt = (struct descriptor 	*) ADR_GDT;
	idt = (struct gate    		*) ADR_IDT;
	
	int i;
	//put_str(">init gdt\n");
	
	for (i = 0; i <= LIMIT_GDT/8; i++) {
		set_segment_descriptor(gdt + i, 0, 0, 0);
	}
	
	set_segment_descriptor(gdt + 1, 0xffffffff,   0x00000000, AR_CODE32_ER);
	set_segment_descriptor(gdt + 2, 0xffffffff,   0x00000000, AR_DATA32_RW);
	set_segment_descriptor(gdt + 3, sizeof(tss), (uint32_t )&tss, DA_386TSS);
	set_segment_descriptor(gdt + 4, 0xffffffff, 0x00000000, DA_CR| DA_DPL3|DA_32);
	set_segment_descriptor(gdt + 5, 0xffffffff, 0x00000000, DA_DRW| DA_DPL3|DA_32);
	set_segment_descriptor(gdt + 6, 0xffffffff, 0x00000000, DA_CR| DA_DPL1|DA_32);
	set_segment_descriptor(gdt + 7, 0xffffffff, 0x00000000, DA_DRW| DA_DPL1|DA_32);
  
	load_gdtr(LIMIT_GDT ,ADR_GDT);
	//printk(" Gdt addr %x limit %x\n", gdtr.addr ,gdtr.limit);

	//put_str(">init idt\n");

	//enable_irq(15);
	/* IDT初始化 */
	for (i = 0; i <= LIMIT_IDT / 8; i++) {
		set_gate_descriptor(idt + i, 0, 0, 0, 0);
	}
	
	set_gate_descriptor(idt + 0x00, (int )&divide_error, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x01, (int )&single_step_exception, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x02, (int )&nmi, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x03, (int )&breakpoint_exception, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x04, (int )&overflow, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x05, (int )&bounds_check, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x06, (int )&inval_opcode, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x07, (int )&copr_not_available, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x08, (int )&double_fault, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x09, (int )&copr_seg_overrun, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x0a, (int )&inval_tss, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x0b, (int )&segment_not_present, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x0c, (int )&stack_exception, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x0d, (int )&general_protection, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x0e, (int )&page_fault, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x0f, (int )&copr_error, 0x08, DA_386IGate, 0);
	
	set_gate_descriptor(idt + 0x20+CLOCK_IRQ, (int )&IRQ_clock, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+KEYBOARD_IRQ, (int )&IRQ_keyboard, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+IRQ_PORT9, (int )&IRQ_port9, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+ETHERNET_IRQ, (int )&IRQ_ethernet, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+MOUSE_IRQ, (int )&IRQ_mouse, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+PRIMARY_IRQ, (int )&IRQ_primary_channel, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+SECONDARY_IRQ, (int )&IRQ_secondary_channel, 0x08, DA_386IGate, 0);
	
	/* IDT设定 */
	/*set_gate_descriptor(idt + 0x20+CLOCK_IRQ, (int )&IRQ_clock, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+KEYBOARD_IRQ, (int )&IRQ_keyboard, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+MOUSE_IRQ, (int )&IRQ_mouse, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+PRIMARY_IRQ, (int )&IRQ_primary_channel, 0x08, DA_386IGate, 0);
	set_gate_descriptor(idt + 0x20+SECONDARY_IRQ, (int )&IRQ_secondary_channel, 0x08, DA_386IGate, 0);
	
	set_gate_descriptor(idt + 0x80, (int )&intrrupt_sys_call, 0x08, DA_386IGate, 3);
	*/
	set_gate_descriptor(idt + 0x80, (int )&intrrupt_sys_call, 0x08, DA_386IGate, 3);
	
	for(i = 0; i < NR_IRQ; i++){
		irq_table[i] = default_irq_handler;
	}
	init_8259a();
	
	/*打开从盘*/
	enable_irq(CASCADE_IRQ);
	
	load_idtr(LIMIT_IDT, ADR_IDT);

	//printk(" Idt addr %x limit %x\n", ADR_IDT ,LIMIT_IDT);
}

/*******************************************************************************
*Function name: descriptor_alloc
*Description: alloc a empty descriptor
*Input: None
*Output: None
*Return: (integer) if above (0), sucess! if (-1), faild
*Auther: Eric hu
*Date: 2018/12/5
*Version: 0.1
*******************************************************************************/
int descriptor_alloc()
{
	int i;
	struct descriptor *des;
	/*Find from DESCRIPTOR_ALLOC_START to LIMIT_GDT/8*/
	for (i = DESCRIPTOR_ALLOC_START; i < LIMIT_GDT/8; i++) {
		des = gdt + i;
		/*If it hasn't used*/
		if(!des->limit_low && !des->limit_high && !des->access_right){
			
			return i;
		}
	}
	return -1;
}

void descriptor_free(int idx)
{
	int i;
	/*Find from DESCRIPTOR_ALLOC_START to LIMIT_GDT/8*/
	for (i = DESCRIPTOR_ALLOC_START; i < LIMIT_GDT/8; i++) {
		if(i == idx){
			set_segment_descriptor(gdt + i, 0, 0, 0);
			return;
		}
	}
}

void set_segment_descriptor(struct descriptor *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gate_descriptor(struct gate *gd, uint32_t offset, uint32_t selector, uint32_t ar, uint8_t privilege)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->datacount     = (ar >> 8) & 0xff;
	gd->attr		= ar | (privilege << 5);
	gd->offset_high  = (offset >> 16) & 0xffff;
}

void put_irq_handler(int irq, irq_handler_t handler)
{
	irq_table[irq] = handler;
}
/*
创建一个窗口并显示异常
*/
void exception_handler(int esp, int vec_no, int err_code, int eip, int cs, int eflags)
{
	char err_description[][64] = {	"#DE Divide Error",
					"#DB RESERVED",
					"—  NMI Interrupt",
					"#BP Breakpoint",
					"#OF Overflow",
					"#BR BOUND Range Exceeded",
					"#UD Invalid Opcode (Undefined Opcode)",
					"#NM Device Not Available (No Math Coprocessor)",
					"#DF Double Fault",
					"    Coprocessor Segment Overrun (reserved)",
					"#TS Invalid TSS",
					"#NP Segment Not Present",
					"#SS Stack-Segment Fault",
					"#GP General Protection",
					"#PF Page Fault",
					"—  (Intel reserved. Do not use.)",
					"#MF x87 FPU Floating-Point Error (Math Fault)",
					"#AC Alignment Check",
					"#MC Machine Check",
					"#XF SIMD Floating-Point Exception"
				};
	/*struct task *task = task_current();
	printk("\nExpection in task %s !");
	printk(task->name);
*/
	io_cli();
	
	struct thread *thread = thread_current();
	
	console_set_color(MAKE_COLOR(BLACK,RED));
	
	printk("\nExpection in thread %s stack:%x!\n", thread->name, thread->kernel_stack);

	printk("ERROR:%s\n",err_description[vec_no]);
	
	printk("EFLAGS:%x CS:%x EIP:%x ESP:%x\n",eflags, cs, eip, esp);
	
	if(err_code != 0xFFFFFFFF){
		printk("Error code:%x\n", err_code);
		
		if(err_code&1){
			printk("    External Event: NMI,hard interruption,ect.\n");
		}else{
			printk("    Not External Event: inside.\n");
		}
		if(err_code&(1<<1)){
			printk("    IDT: selector in idt.\n");
		}else{
			printk("    IDT: selector in gdt or ldt.\n");
		}
		if(err_code&(1<<2)){
			printk("    TI: selector in ldt.\n");
		}else{
			printk("    TI: selector in gdt.\n");
		}
		printk("    Selector: idx %d\n", (err_code&0xfff8)>>3);
	}
	
	printk("kernel stack:%x", (uint32 )thread->kernel_stack);



	io_hlt();
	while(1);
	
	
}

void default_irq_handler(int irq)
{
	/*printk("!default_irq_handler!");
	put_int(irq);
	printk("\n");*/
}


#endif //_CONFIG_ARCH_X86_


