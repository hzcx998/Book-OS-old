#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <types.h>

/*BASIC*/
#define _NR_WRITE	0
#define _NR_MALLOC	1
#define _NR_FREE	2
#define _NR_EXIT	3
#define _NR_GETCHAR	4
#define _NR_PUTCHAR	5
#define _NR_EXECV	6
#define _NR_WAIT	7
#define _NR_CLEAR	8
#define _NR_PS		9

/*FS*/
#define _NR_FOPEN	10
#define _NR_FCLOSE	11
#define _NR_FREAD	12
#define _NR_FWRITE	13
#define _NR_LSEEK	14
#define _NR_FSTAT	15
#define _NR_UNLINK	16
#define _NR_OPENDIR	17
#define _NR_CLOSEDIR 18
#define _NR_READDIR	19
#define _NR_REWINDDIR	20
#define _NR_MKDIR	21
#define _NR_RMDIR	22
#define _NR_CHDIR	23
#define _NR_GETCWD	24
#define _NR_RENAME	25
#define _NR_COPY	26
#define _NR_MOVE	27
#define _NR_ACCESS	28
#define _NR_LSDIR	29

/*SYSTEM*/
#define _NR_REBOOT	30
#define _NR_GETTIME	31
#define _NR_GET_MOUSE_POS	32
#define _NR_GET_MOUSE_BTN	33
#define _NR_GET_SCREEN		34
#define _NR_GET_MEMORY		35
#define _NR_GET_PID			36
#define _NR_GET_TICKS		37
#define _NR_SLEEP		    38
#define _NR_LSDISK		    39

/*GUI*/
#define _NR_INIT_GRAPHIC	40
#define _NR_GRAPH_POINT	41
#define _NR_GRAPH_REFRESH	42
#define _NR_GRAPH_LINE	43
#define _NR_GRAPH_RECT	44
#define _NR_GRAPH_TEXT	45
#define _NR_GRAPHIC_EXIT	46
#define _NR_GRAP_BUFFER	47
#define _NR_GRAP_WORD	48
#define _NR_GRAP_DRAW_BMP	49
#define _NR_GRAP_DRAW_JPEG	50


/*标准图形库*/
#define _NR_GUI_INIT 	51
#define _NR_GUI_WINDOW_SIZE 	52
#define _NR_GUI_WINDOW_POSITION 	53
#define _NR_GUI_WINDOW_CREATE 	54
#define _NR_GUI_WINDOW_CLOSE 	55
#define _NR_GUI_COLOR 	56
#define _NR_GUI_POINT 	57
#define _NR_GUI_LINE 	58
#define _NR_GUI_RECT 	59
#define _NR_GUI_TEXT 	60
#define _NR_GUI_BUFFER 	61
#define _NR_GUI_FRAME 	62
#define _NR_GUI_DRAW 	63
#define _NR_GUI_MOUSE_MOVE 	    64
#define _NR_GUI_MOUSE_CLICK 	65
#define _NR_GUI_KEYBOARD 	    66
#define _NR_GUI_GET_MODIFIERS 	67

#define _NR_FS_LOAD_INODE		70

#define _NR_FS_DRIVE_CONNECT	71
#define _NR_FS_DRIVE_GET		72

#define _NR_THREAD_CONNECT	    73
#define _NR_THREAD_GETINFO		74

#define _NR_THREAD_TESTCANCEL		75

#define _NR_PIPE_CREATE		76
#define _NR_PIPE_CONNECT	77
#define _NR_PIPE_WRITE	    78
#define _NR_PIPE_READ	    79
#define _NR_PIPE_CANCEL	    80
#define _NR_PIPE_CLOSE	    81

#define _NR_GET_PPID	    82
#define _NR_WAIT_CHILD_EXIT	83

#define _NR_THREAD_KILL		84

struct thread *thread_connect();
struct thread *thread_getinfo(struct thread *ptr);


#define MAX_SYS_CALLS 90

extern sys_call_t sys_call_table[MAX_SYS_CALLS];

void init_syscall();//初始化中断调用

/*interrupt.asm*/
void intrrupt_sys_call();//中断调用服务程序
void sendrec();


#endif

