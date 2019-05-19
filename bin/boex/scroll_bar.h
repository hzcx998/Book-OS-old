#ifndef _SCROLL_BAR_H
#define _SCROLL_BAR_H

#include <stdint.h>
#include "view.h"

#define SCROLL_BAR_STATUS_IDLE  0
#define SCROLL_BAR_STATUS_TOUCH  1
#define SCROLL_BAR_STATUS_CLICK  2

#define SCROLL_BAR_COLOR_IDLE  ARGB(255,200,200,200)
#define SCROLL_BAR_COLOR_TOUCH  ARGB(255,150,150,150)
#define SCROLL_BAR_COLOR_CLICK  ARGB(255,120,120,120)

#define SCROLL_BAR_COLOR_BACK  ARGB(255,220,220,220)
#define SCROLL_BAR_COLOR_FRONT  ARGB(255,255,255,255)

#define SCROLL_BAR_DEFAULT_WIDTH  24
#define SCROLL_BAR_DEFAULT_HEIGHT  24

#define SCROLL_BAR_DISABLENOSCROLL   0X01
#define SCROLL_BAR_PAGE              0X02
#define SCROLL_BAR_POSE              0X04
#define SCROLL_BAR_RANGE             0X08
#define SCROLL_BAR_ALL               (SCROLL_BAR_PAGE|SCROLL_BAR_POSE|SCROLL_BAR_RANGE)

#define SCROLL_BAR_VSCROLL           1  //垂直
#define SCROLL_BAR_HSCROLL           2  //水平

#define SCROLL_BAR_THICKNESS_DEFAULT           16

/*
scroll_bar
*/
typedef struct scrool_bar_blocl_s
{
    int x, y;
    int width, height;
    int moving;
}scrool_bar_blocl_t;
/*
scroll_bar
*/
typedef struct scrool_bar_s
{
    int x, y;
    int thick, len; 
    int min, max;   //范围
    int page;   //页面大小
    int pos;    //位置
    int status;
    int flags;
    int type;  //滚动条类型
    int off;

    uint32_t color_idle, color_touch, color_click;  //静止色，触碰色，点击色
    uint32_t bcolor, fcolor;    //背景色和前景色
    void (*mouse_down)(void);    //单击按钮的操作
    void (*mouse_up)(void);    //弹起按钮的操作

    struct scrool_bar_blocl_s block;    //滑动块
    struct view_s *view;    //滑动块
}scroll_bar_t;

void scroll_bar_init(scroll_bar_t *scroll_bar, int x, int y, int len, int type);
void scroll_bar_color_status(scroll_bar_t *scroll_bar, uint32_t idle, uint32_t touch, uint32_t click);
void scroll_bar_color_face(scroll_bar_t *scroll_bar, uint32_t bcolor, uint32_t fcolor);

void scroll_bar_set_range(scroll_bar_t *scroll_bar, uint32_t min, uint32_t max);
void scroll_bar_set_pos(scroll_bar_t *scroll_bar, uint32_t pos);
void scroll_bar_set_flags(scroll_bar_t *scroll_bar, uint32_t flags);
void scroll_bar_set_page(scroll_bar_t *scroll_bar, uint32_t page);

void scroll_bar_draw(scroll_bar_t *scroll_bar);

void scroll_bar_bind_even(scroll_bar_t *scroll_bar, void (*mouse_down)(), void (*mouse_up)());

void scroll_bar_bind_view(scroll_bar_t *scroll_bar, struct view_s *view);
void scroll_bar_update(scroll_bar_t *scroll_bar, int mousex, int mousey, int status);
void scroll_bar_change_status(scroll_bar_t *scroll_bar, int status);



#endif  //_SCROLL_BAR_H
