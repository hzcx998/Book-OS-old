#ifndef _GUI_BOGUI_TASKBAR_H
#define _GUI_BOGUI_TASKBAR_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <gui/bogui/container.h>
#include <gui/bogui/window.h>

#define BOGUI_TASKBAR_HEIGHT 24

#define BOGUI_TASKBAR_BCOLOR ARGB(240,30,30,30)

#define BOGUI_WINCTL_TITLE_LEN 128

#define BOGUI_WINCTL_WIDTH (20+64+2)
#define BOGUI_WINCTL_HEIGHT 24

#define BOGUI_WINCTL_IDLE   1
#define BOGUI_WINCTL_TOUCH  2
#define BOGUI_WINCTL_ACTIVE 3
#define BOGUI_WINCTL_CLOSE  4   //处于关闭的状态

/*
任务栏中的每个任务对应的小窗口控制
*/
typedef struct bogui_winctl_s
{
	int32_t x, y;   //窗口的位置
	uint32_t width, height;	//窗口的大小
    uint32_t flags;
    uint8_t status;

    uint32_t color_idle, color_touch, color_active;

    struct bogui_container_s *container;    //所在的容器
    struct bogui_window_s *window;    //要控制的窗口
    
    char name[BOGUI_WINCTL_TITLE_LEN];    //窗口的标题
    struct bogui_winctl_s *next;    //下一个窗口控制
}bogui_winctl_t;

/*
对任务栏进行总的管理
*/
typedef struct bogui_taskbar_s
{
    int winctl_count;
    bogui_winctl_t *winctl_header;
    bogui_winctl_t *winctl_activing;
    
    struct bogui_container_s *container;

    uint32_t bcolor;    //任务栏的背景颜色
    uint32_t ftcolor;    //字体颜色
    
    uint32_t width; //任务栏宽度
    uint32_t height; //任务栏高度
}bogui_taskbar_t;

extern struct bogui_taskbar_s bogui_taskbar;

bogui_winctl_t *bogui_winctl_create(char *title);
int bogui_winctl_append(bogui_winctl_t *winctl);
int bogui_winctl_sub(bogui_winctl_t *winctl);

void bogui_winctl_bind_container(bogui_winctl_t *winctl, struct bogui_container_s *container);
void bogui_winctl_bind_window(bogui_winctl_t *winctl, struct bogui_window_s *window);

void bogui_winctl_draw(bogui_winctl_t *winctl);

void bogui_taskbar_init();

void bogui_taskbar_mouse_move_listen(struct bogui_container_s *container, int mousex, int mousey);
void bogui_taskbar_mouse_click_listen(struct bogui_container_s *container, int mousex, int mousey, int key, int status);

void bogui_winctl_sort();

bogui_winctl_t *bogui_winctl_add(char *title);
int bogui_winctl_close(bogui_winctl_t *winctl);

void bogui_taskbar_winctl_focus(bogui_winctl_t *winctl);
void bogui_taskbar_winctl_cancel_all();
void bogui_taskbar_winctl_cancel(bogui_winctl_t *winctl);

#endif //_CONFIG_GUI_BOGUI_

#endif //_GUI_BOGUI_TASKBAR_H

