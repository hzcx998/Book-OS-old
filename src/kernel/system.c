/*
File:		kernel/system.c
Contains:	system 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/core.h>
#include <sys/arch.h>

int32_t sys_reboot(int reboot_type)
{
	//一直循环
	while(1){
		//先执行一次
		do_reboot(reboot_type);
		//如果没有成功，就选择其它方式
		switch(reboot_type){
			case BOOT_KBD:
				reboot_type = BOOT_CF9;
				break;
			case BOOT_CF9:
				reboot_type = BOOT_BIOS;
				break;

			case BOOT_BIOS:
				//回到第一个方法
				reboot_type = BOOT_KBD;
				break;
			default: 
				reboot_type = BOOT_KBD;
				break;
		}
	}
}

void do_reboot(int reboot_type)
{
	/*不能在switch语句中申请变量，所以放到外面来*/
	uint8_t reboot_code;
	uint8_t cf9;
	struct gate *idt;
	int i;
	
	switch(reboot_type){
		case BOOT_KBD:
			/*
			8042 键盘控制器的错误使用方法
			向 8042 键盘控制器（端口 0x64）写入 0xFE. 
			这会触发 CPU 的重置信号，并重启计算机。
			*/
			io_out8(0x64, 0xfe);
			break;
		case BOOT_CF9:
			/*
			向 PCI 的 0xCF9 端口发送重置信号
			Intel 的 ICH/PCH 南桥芯片同时负责部分电源工作。向 PCI 的 0xCF9 端口
			发送重置信号，可以要求南桥芯片重启计算机。
			*/
			reboot_code = 0x06;	/*热重启，冷重启0x0e*/
			cf9 = io_in8(0xcf9) & ~reboot_code;	
			io_out8(0xcf9, cf9|2);	/*Request hard reset*/
			io_out8(0xcf9, cf9|reboot_code);	/*Actually do the reset*/
			break;
		case BOOT_TRIPLE:
			io_cli();
			//清空中断描述符表
			idt = (struct gate *)ADR_IDT;
			for (i = 0; i <= LIMIT_IDT / 8; i++) {
				set_gate_descriptor(idt + i, 0, 0, 0, 0);
			}
			load_idtr(LIMIT_IDT, ADR_IDT);
			//触发中断
			__asm__ __volatile__("ud2");
			/* 此时发生中断，由于 IDT 为空，无论是中断的异常处理代码还是双重异常处理代码都不存在，触发了三重异常
			CPU 进入 SHUTDOWN 状态，主板重置 */
			break;
		default:
			break;
	}
}
