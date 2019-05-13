#ifndef _IO_H_
#define _IO_H_
#include <types.h>
#include <stdint.h>

/*输入输出操作，以及一些寄存器操作*/
	int io_in8(int port);
	int io_in16(int port);
	int io_in32(int port);
	void io_out8(int port, int data);
	void io_out16(int port, int data);
	void io_out32(int port, int data);
	void io_cli(void);
	void io_sti(void);
	void io_hlt(void);
	void io_stihlt(void);
	void load_tr(int tr);
	int32_t read_cr0(void );
	int32_t read_cr3(void );
	void write_cr0(uint32_t address);
	void write_cr3(uint32_t address);
	void store_gdtr(uint32_t gdtr);
	void load_gdtr(int limit, int addr);
	void store_idtr(uint32_t idtr);
	void load_idtr(int limit, int addr);
	void enable_irq(int irq);
	void disable_irq(int irq);
	int io_load_eflags(void);
	void io_store_eflags(int eflags);
	
	void port_read(uint16_t port, void* buf, int n);
	void port_write(uint16_t port, void* buf, int n);
	
	void disable_irq(int irq);
	void enable_irq(int irq);
	
	
	
	
	
	
	
	
#endif