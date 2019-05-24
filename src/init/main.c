/*
File:		init/main.c
Contains:	kernel main entry
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/arch.h>
#include <sys/core.h>
#include <sys/mm.h>
#include <sys/dev.h>
#include <sys/debug.h>
#include <sys/fs.h>
#include <sys/gui.h>
#include <sys/net.h>

#include <graphic.h>

#define WRITE_DISK 1

#define WRITE_ID 4

#if WRITE_ID == 1
	#define WRITE_NAME "C:/test"
	#define FILE_SECTORS 30
#elif WRITE_ID == 2
	#define WRITE_NAME "c:/bosh"
	#define FILE_SECTORS 100
#elif WRITE_ID == 3
	#define WRITE_NAME "c:/boex"	   
	#define FILE_SECTORS 100
#elif WRITE_ID == 4
	#define WRITE_NAME "c:/invader"	   
	#define FILE_SECTORS 50
#endif


#define FILE_PHY_ADDR 0x42000

void write_bin();
void shell();

int main()
{
	init_page();
	init_descriptor();
	
	init_print();
	init_console();
	
	init_mm();
	
	init_video();
	/*初始化图形界面后才显示图形，不然都是开机界面*/
	init_gui();

	init_cpu();
	init_thread();
	
	init_clock();
	/*we enable intrrupt and enable slave 8259A chip*/
	io_sti();
	
	init_pci(); 
	//while(1);
	init_net();
	//while(1);
	/*
	init disk device here.when we init disk device,
	we will install disk to disk device table
	*/
	init_disk_device();
	
	/*init harddisk device*/
	init_harddisk();
	/*init ramdisk device*/
	init_ramdisk();
	
	init_fs();
	
	//while(1);
	
	init_keyboard();
	init_mouse();
	
	init_pipe();

	thread_start("gui", 2, thread_gui, NULL);
	thread_start("keyboard", 2, thread_keyboard, NULL);
	thread_start("mouse", 2, thread_mouse, NULL);
	thread_start("timer", 2, thread_timer, NULL);
	/*
	int pipe = sys_pipe_create(64);
	printk("pipe %d \n", pipe);
	*/
	//while(1);
	/*
	初始化系统调用后才可以调用系统为用户提供的函数
	we need init syscall table before we start user process
	after init basic system.
	*/
	init_syscall();

	//while(1);

	//shell();

	if(WRITE_DISK){
		write_bin();
	}
	
	/*清空终端信息*/
	//terminal_clean();
	
	//printk("my pid:%d\n",sys_get_pid());

	process_execute(init, "init");
	
	thread_bus.main = 1;

	//char buf[64];

	while(1){
		/*if (pipe_read(pipe, buf) >= 0) {
			printk("pipe read get data:%s \n", buf);
		}*/
		/*如果有进程被击杀，在这里回收
		通常是被鼠标或者键盘强制停止的进程
		*/
		thread_recover();

		/*0.1秒检测一次*/
		msec_sleep(100);
	}
	return 0;
}

void write_bin()
{

	printk("> load file start.\n");
	
	//char *app_addr = (char *)FILE_PHY_ADDR;

	char *app_addr = (char *)mm.malloc(FILE_SECTORS*SECTOR_SIZE);
	if(app_addr == NULL){
		panic("alloc mem failed!\n");
	}
	//printk("app:%x\n", app_addr);
	
	memset(app_addr, 0, FILE_SECTORS*SECTOR_SIZE);
	
	//从硬盘上读取数据
	//选择设备类型
	select_disk_device_type("harddisk");
	//切换到第二个硬盘
	set_current_hd("hdb");
	//开始读取数据
	
	int i;
	for(i = 0; i < FILE_SECTORS; i++){
		sector_read(i, app_addr + i*SECTOR_SIZE, 1);
	}
	int fd;
	int written;
	/*写入到文件系统中去*/
	fd = fs.open(WRITE_NAME, O_CREAT|O_RDWR);
	int mod = fs.getmod(WRITE_NAME);
	mod |= IMODE_X;
	fs.setmod(WRITE_NAME, mod);
	written = fs.write(fd, app_addr, FILE_SECTORS*SECTOR_SIZE);
	printk("  write bin %s size:%d success!\n",WRITE_NAME, written);
	fs.close(fd);
	mm.free(app_addr);
	printk("< load file done.\n");
	
}
