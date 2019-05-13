#ifndef _GUI_BOGUI_LIBARY_H
#define _GUI_BOGUI_LIBARY_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <gui/bogui/container.h>

/*
BookOS上层统一的图形界面函数库

【窗口】
    void gui_init();
    void gui_window_size(int width, int height);
    void gui_window_position(int x, int y);
    void gui_window_create(const char *title);

    void gui_window_close();

【图形】
    void gui_color(uint32_t color);

    void gui_point(int x, int y);
    void gui_line(int x1, int y1, int x2, int y2);
    void gui_rect(int x, int y, int width, int height);

    void gui_text(int x, int y, char *text);
    void gui_buffer(int x, int y, int width, uint8_t *buffer);
    void gui_draw(int x1, int y1, int x2, int y2);

*/

#define GUI_ACTIVE_SHIFT 	0x01
#define GUI_ACTIVE_CTRL 	0x02
#define GUI_ACTIVE_ALT	 	0x04


typedef struct bogui_libary_s
{
    int x, y;
    int width, height;
    uint32_t color;
    uint8_t *buffer;
    
    struct bogui_container_s *container;
} bogui_libary_t;

void bogui_guilib_bind_container(bogui_libary_t *guilib, bogui_container_t *container);

/*
窗口
*/
int bogui_guilib_init();
void bogui_guilib_window_size(int width, int height);
void bogui_guilib_window_position(int x, int y);
int bogui_guilib_window_create(const char *title);
int bogui_guilib_window_close();

/*
图形图像
*/
void bogui_guilib_color(unsigned int color);
void bogui_guilib_point(int x, int y);
void bogui_guilib_line(int x1, int y1, int x2, int y2);
void bogui_guilib_rect(int x, int y, int width, int height);
void bogui_guilib_text(int x, int y, char *text);

void bogui_guilib_buffer(unsigned char *buffer);
void bogui_guilib_frame(int x, int y, int width, int height);

void bogui_guilib_draw(int x1, int y1, int x2, int y2);

int bogui_guilib_mouse_move(int *x, int *y);
int bogui_guilib_mouse_click(int *key, int *status, int *x, int *y);
int bogui_guilib_keyboard(int *key);
int bogui_guilib_get_modifiers();

#endif //_CONFIG_GUI_BOGUI_

#endif //_GUI_BOGUI_LIBARY_H