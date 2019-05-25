/*
File:		kernel/syscall.c
Contains:	syscall set here 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/core.h>
#include <sys/mm.h>
#include <sys/debug.h>
#include <sys/fs.h>
#include <sys/dev.h>

/**
	添加一个系统调用步骤：
	1.sys_call_table中添加函数名
	2.系统调用函数体和头文件
	3.在user的syscall中添加中断调用和头文件
*/

sys_call_t sys_call_table[MAX_SYS_CALLS];

void init_syscall()
{
	sys_call_table[_NR_WRITE] = sys_write_str;
	sys_call_table[_NR_MALLOC] = mm.usr_malloc;
	sys_call_table[_NR_FREE] = mm.usr_free;
	sys_call_table[_NR_EXIT] = sys_exit;
	sys_call_table[_NR_GETCHAR] = sys_get_key;
	sys_call_table[_NR_PUTCHAR] = sys_writ_char;
	sys_call_table[_NR_EXECV] = sys_execv;
	sys_call_table[_NR_WAIT] = sys_wait;
	sys_call_table[_NR_CLEAR] = sys_clean_screen;
	sys_call_table[_NR_PS] = print_threads;
	
	sys_call_table[_NR_FOPEN] = fs.open;
	sys_call_table[_NR_FCLOSE] = fs.close;
	sys_call_table[_NR_FREAD] = fs.read;
	sys_call_table[_NR_FWRITE] = fs.write;
	sys_call_table[_NR_LSEEK] = fs.lseek;
	sys_call_table[_NR_FSTAT] = fs.stat;
	sys_call_table[_NR_UNLINK] = fs.unlink;
	sys_call_table[_NR_OPENDIR] = fs.opendir;
	sys_call_table[_NR_CLOSEDIR] = fs.closedir;
	sys_call_table[_NR_READDIR] = fs.readdir;
	sys_call_table[_NR_REWINDDIR] = fs.rewinddir;
	sys_call_table[_NR_MKDIR] = fs.mkdir;
	sys_call_table[_NR_RMDIR] = fs.rmdir;
	sys_call_table[_NR_CHDIR] = fs.chdir;
	sys_call_table[_NR_GETCWD] = fs.getcwd;
	sys_call_table[_NR_RENAME] = fs.rename;
	sys_call_table[_NR_COPY] = fs.copy;
	sys_call_table[_NR_MOVE] = fs.move;
	sys_call_table[_NR_ACCESS] = fs.access;
	sys_call_table[_NR_LSDIR] = fs.lsdir;

	
	sys_call_table[_NR_REBOOT] = sys_reboot;
	sys_call_table[_NR_GETTIME] = sys_gettime;
	sys_call_table[_NR_GET_MOUSE_POS] = sys_get_mouse_position;
	sys_call_table[_NR_GET_MOUSE_BTN] = sys_get_mouse_button;
	sys_call_table[_NR_GET_SCREEN] = sys_get_screen;
	sys_call_table[_NR_GET_MEMORY] = sys_get_memory;
	sys_call_table[_NR_GET_PID] = sys_get_pid;
	sys_call_table[_NR_GET_TICKS] = sys_get_ticks;
	sys_call_table[_NR_SLEEP] = msec_sleep;
	sys_call_table[_NR_LSDISK] = fs.lsdisk;
	
	sys_call_table[_NR_INIT_GRAPHIC] = sys_init_graphic;
	sys_call_table[_NR_GRAPH_POINT] = sys_graph_point;
	sys_call_table[_NR_GRAPH_REFRESH] = sys_graph_refresh;
	sys_call_table[_NR_GRAPH_LINE] = sys_graph_line;
	sys_call_table[_NR_GRAPH_RECT] = sys_graph_rect;
	sys_call_table[_NR_GRAPH_TEXT] = sys_graph_text;
	sys_call_table[_NR_GRAPHIC_EXIT] = sys_graphic_exit;
	sys_call_table[_NR_GRAP_BUFFER] = sys_graph_buffer;
	sys_call_table[_NR_GRAP_WORD] = sys_graph_char;

	sys_call_table[_NR_GRAP_DRAW_BMP] = sys_graph_bmp;
	sys_call_table[_NR_GRAP_DRAW_JPEG] = sys_graph_jpeg;
	
	sys_call_table[_NR_GUI_INIT] = gui.init;
	sys_call_table[_NR_GUI_WINDOW_SIZE] = gui.window_size;
	sys_call_table[_NR_GUI_WINDOW_POSITION] = gui.window_position;
	sys_call_table[_NR_GUI_WINDOW_CREATE] = gui.window_create;
	sys_call_table[_NR_GUI_WINDOW_CLOSE] = gui.window_close;
	sys_call_table[_NR_GUI_COLOR] = gui.color;
	sys_call_table[_NR_GUI_POINT] = gui.point;
	sys_call_table[_NR_GUI_LINE] = gui.line;
	sys_call_table[_NR_GUI_RECT] = gui.rect;
	sys_call_table[_NR_GUI_TEXT] = gui.text;
	sys_call_table[_NR_GUI_BUFFER] = gui.buffer;
	sys_call_table[_NR_GUI_FRAME] = gui.frame;
	sys_call_table[_NR_GUI_DRAW] = gui.draw;
	sys_call_table[_NR_GUI_MOUSE_MOVE] = gui.mouse_move;
	sys_call_table[_NR_GUI_MOUSE_CLICK] = gui.mouse_click;
	sys_call_table[_NR_GUI_KEYBOARD] = gui.keyboard;
	sys_call_table[_NR_GUI_GET_MODIFIERS] = gui.get_modifiers;
	sys_call_table[_NR_GUI_BUFFER_SET] = gui.buffer_set;

	sys_call_table[_NR_FS_LOAD_INODE] = fs.load_inode;
	sys_call_table[_NR_FS_DRIVE_CONNECT] = fs.drive_connect;
	sys_call_table[_NR_FS_DRIVE_GET] = fs.drive_get;

	sys_call_table[_NR_THREAD_CONNECT] = sys_thread_connect;
	sys_call_table[_NR_THREAD_GETINFO] = sys_thread_getinfo;
	
	sys_call_table[_NR_THREAD_TESTCANCEL] = thread_do_testcancel;

	sys_call_table[_NR_PIPE_CREATE] = sys_pipe_create;
	sys_call_table[_NR_PIPE_CONNECT] = sys_pipe_connect;
	sys_call_table[_NR_PIPE_WRITE] = sys_pipe_write;
	sys_call_table[_NR_PIPE_READ] = sys_pipe_read;
	sys_call_table[_NR_PIPE_CANCEL] = sys_pipe_cancel;
	sys_call_table[_NR_PIPE_CLOSE] = sys_pipe_close;

	sys_call_table[_NR_GET_PPID] = sys_get_ppid;

	sys_call_table[_NR_WAIT_CHILD_EXIT] = sys_wait_child_exit;

	sys_call_table[_NR_THREAD_KILL] = sys_thread_kill;

	sys_call_table[_NR_MM_REALLOC] = mm.usr_realloc;

	sys_call_table[_NR_TIMER_ALLOC] = timer_alloc;
	sys_call_table[_NR_TIMER_FREE] = timer_free;
	sys_call_table[_NR_TIMER_SETTM] = timer_settime;
	sys_call_table[_NR_TIMER_CANCEL] = timer_cancel;
	sys_call_table[_NR_TIMER_OCCUR] = timer_occur;


}
