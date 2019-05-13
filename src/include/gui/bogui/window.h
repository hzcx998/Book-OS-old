#ifndef _GUI_BOGUI_WINDOW_H
#define _GUI_BOGUI_WINDOW_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <gui/bogui/container.h>
#include <gui/bogui/taskbar.h>

#include <gui/color.h>

#define BOGUI_WINICON_WIDHT 16
#define BOGUI_WINICON_HEIGHT 16

/*
窗口的图标
*/
typedef struct bogui_winicon_s
{
    uint32_t width, height;	//frame的宽度和高度
    uint8_t bit_width;
    uint8_t *data;     //图标data的指针
}bogui_winicon_t;

void bogui_window_bind_winicon(bogui_winicon_t *winicon, uint8_t *buffer, int width, int height, int bit_width);


#define BOGUI_WINBTN_NR 2

/*
按钮状态
*/
#define BOGUI_WINBTN_IDLE   1
#define BOGUI_WINBTN_TOUCH  2

/*
按钮类型
*/
#define BOGUI_WINBTN_CLOSE  1
#define BOGUI_WINBTN_MINIMIZE 2

#define BOGUI_WINBTN_CLOSE_FORCE  1
#define BOGUI_WINBTN_CLOSE_ACCORD  2

/*
窗口的按钮
*/
typedef struct bogui_winibtn_s
{
    int x, y;
    uint32_t width, height;	//frame的宽度和高度
    uint8_t status;

    uint32_t color, color_touch;

    struct bogui_container_s *container;    //窗口所在的容器

    uint8_t type;     //按钮类型
}bogui_winbtn_t;

void bogui_winbtn_init(bogui_winbtn_t *winibtn, struct bogui_container_s *container, \
    int x, int y, int width, int height, uint8_t type);

void bogui_winbtn_draw(bogui_winbtn_t *winbtn);
/*
frame是窗口的活动区域
*/
typedef struct bogui_frame_s
{
	int32_t x, y;   //frame相对于window的位置
    uint32_t width, height;	//frame的宽度和高度

    uint8_t catch_mouse_move, catch_mouse_click;    //抓取鼠标，鼠标是否进入窗口
    int mouse_x, mouse_y;
    int mouse_key, mouse_status;

    uint8_t catch_keyboard;
    int keyboard_key;

}bogui_frame_t;

#define BOGUI_WINDOW_COLOR_ACTIVE ARGB(255, 60,60,60)
#define BOGUI_WINDOW_COLOR_INACTIVE ARGB(255, 90,90,90)

#define BOGUI_WINDOW_COLOR_TITLE_ACTIVE ARGB(255, 255,255,255)
#define BOGUI_WINDOW_COLOR_TITLE_INACTIVE ARGB(255, 182,182,182)

#define BOGUI_WINDOW_TITLE_LEN 128

#define BOGUI_WINDOW_INACTIVE   0
#define BOGUI_WINDOW_ACTIVE     1

#define BOGUI_WINDOW_BAR_HEIGHT 24

typedef struct bogui_window_s 
{
	
	int32_t x, y;   //窗口的位置
	uint32_t width, height;	//窗口的大小
    uint32_t status;

    uint32_t bc_active, bc_inactive;    //窗口的背景颜色

    struct bogui_frame_s frame;     //窗口中的frame
    struct bogui_container_s *container;    //窗口所在的容器
    struct bogui_winctl_s *winctl;    //窗口所在的容器
    
    struct bogui_winicon_s winicon;    //窗口的图标
    struct bogui_winibtn_s winbtn[BOGUI_WINBTN_NR];   //窗口的按钮，默认2个

    struct bogui_libary_s *guilib;  //窗口对应的图形库环境

    char title[BOGUI_WINDOW_TITLE_LEN];    //窗口的标题
}bogui_window_t;

bogui_window_t *bogui_create_window(char *title, int x, int y, int width, int height);

void bogui_window_draw_body(bogui_window_t *window, char activate);
void bogui_window_draw_frame(bogui_window_t *window);
void bogui_close_window(bogui_window_t *window);

void bogui_window_on_flags(bogui_window_t *window, uint32_t flags);
void bogui_window_off_flags(bogui_window_t *window, uint32_t flags);

void bogui_inactive_all_window();
void bogui_window_active(struct bogui_container_s *container);
struct bogui_container_s *bogui_get_topest_window();

void bogui_window_bind_winctl(bogui_window_t *window, struct bogui_winctl_s *winctl);
void bogui_window_bind_container(bogui_window_t *window, struct  bogui_container_s *container);
void bogui_window_bind_guilib(bogui_window_t *window, struct  bogui_libary_s *guilib);

struct bogui_container_s *bogui_add_window(char *title, int x, int y, int width, int height);

void bogui_window_mouse_move_listen(struct bogui_container_s *container, int mousex, int mousey);
void bogui_window_mouse_click_listen(struct bogui_container_s *container, int mousex, int mousey, int key, int status);
void bogui_window_keyboard_listen(struct bogui_container_s *container, int key_data);

/*图形绘制*/
void bogui_window_refresh(bogui_window_t *window, int x0, int y0, int x1, int y1);
void bogui_window_draw_string(bogui_window_t *window, int x,int y, char *s, uint32_t color);
void bogui_window_draw_char(bogui_window_t *window, int x,int y, char ch, uint32_t color);
void bogui_window_draw_line(bogui_window_t *window, int x0, int y0, int x1, int y1, uint32_t color);
void bogui_window_draw_rect(bogui_window_t *window, int x, int y, int width, int height, uint32_t color);
void bogui_window_draw_point(bogui_window_t *window, int x, int y, uint32_t color);



#endif //_CONFIG_GUI_BOGUI_

#endif //_GUI_BOGUI_IMAGE_H

