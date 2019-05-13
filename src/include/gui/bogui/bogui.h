#ifndef _GUI_BOGUI_H
#define _GUI_BOGUI_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <gui/bogui/container.h>

extern bogui_container_t *buttom_container, *top_container;

void init_bogui();

void bogui_do_thread();

extern int bogui_thread_running, bogui_thread_block;


/*
mouse listen
首先在bogui中定义了监听事件的入口
通过这些入口可以查询某个容器是否有监听事件，如果有，就去执行。
*/
void bogui_mouse_move_listen(int mousex, int mousey);
void bogui_mouse_click_listen( int mousex, int mousey, int key, int status);

void bogui_draw_point(int32 x, int32 y, uint32 color);
void bogui_draw_rect(int32 x, int32 y, uint32 width, uint32 height, uint32 color);
void bogui_draw_line(int32 x0, int32 y0, int32 x1, int32 y1, uint32 color);
void bogui_draw_buffer(int32 x, int32 y, uint32 width, uint32 height, uint32 *buffer);

#endif //_CONFIG_GUI_BOGUI_

#endif //_GUI_BOGUI2_H