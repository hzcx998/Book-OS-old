#ifndef _ITEM_H
#define _ITEM_H

#include <stdint.h>
#include "button.h"

#define ITEM_STATUS_UNUSED  0
#define ITEM_STATUS_IDLE  1
#define ITEM_STATUS_TOUCH  2
#define ITEM_STATUS_CLICK  3

#define ITEM_TEXT_LEN  64


#define ITEM_COLOR_IDLE  ARGB(255,220,220,220)
#define ITEM_COLOR_TOUCH  ARGB(255,180,180,180)
#define ITEM_COLOR_CLICK  ARGB(255,120,120,120)

#define ITEM_NAME_WIDTH  (16*8) //16个字符宽度
#define ITEM_DATE_WIDTH  (18*8) //18个字符宽度
#define ITEM_SIZE_WIDTH  (12*8) //18个字符宽度

#define ITEM_CATALOG_WIDTH  (ITEM_NAME_WIDTH+ITEM_DATE_WIDTH+ITEM_SIZE_WIDTH+4*8)
#define ITEM_FAVORITE_WIDTH  (ITEM_NAME_WIDTH+2*8)


#define ITEM_DEFAULT_HEIGHT  24

#define ITEM_CTRL_BUTTON_NR  5

/*
item
*/
typedef struct item_s
{
    int x, y;
    int id;
    int width, height;
    int status;
    int page;   //item 所在的页面
    uint32_t color_idle, color_touch, color_click;  //静止色，触碰色，点击色
    uint32_t bcolor, fcolor;    //背景色和前景色
    char text[ITEM_TEXT_LEN];
    void (*mouse_down)(void);    //单击按钮的操作
    void (*mouse_up)(void);    //弹起按钮的操作

}item_t;

typedef struct item_manager_s
{
    struct item_s *item_talbe;
    
    struct item_s *cur_item;

    int max_items;  //一共有多少个条目
    
    int page_idx;     //活动中的items
    int items;     //活动中的items
    int pages;     //条目需要多少页
    int page_items; //一个页中可以有多少个item
    int cur_page;   //当前页面
    int x, y;   //管理器的位置
    int width, height;  //管理器的大小
    uint32_t bcolor, fcolor;    //背景色和前景色
    /*
    有5个按钮
    0：首页 (home)
    1：左翻 (left)
    2：中间 (middle)
    3：右翻 (right)
    4：尾页 (trailer)
    */
    struct button_s *button;  
}item_manager_t;

int item_manager_init(item_manager_t *item_manager, int max_items, int x, int y, \
    int width, int height, int item_width);

item_t *item_manager_alloc(item_manager_t *item_manager);
void item_manager_free(item_manager_t *item_manager, item_t *item);
void item_manager_update(item_manager_t *item_manager, int mousex, int mousey, int status);
void item_manager_color_face(item_manager_t *item_manager, uint32_t bcolor, uint32_t fcolor);
void item_manager_change_page(item_manager_t *item_manager, int page);
bool is_item_manager_mouse_on(item_manager_t *item_manager, int mx, int my);

void item_init(item_t *item, int x, int y, int width, int height);
void item_color_status(item_t *item, uint32_t idle, uint32_t touch, uint32_t click);
void item_color_face(item_t *item, uint32_t bcolor, uint32_t fcolor);
void item_draw(item_manager_t *item_manager, item_t *item);

void item_set_text(item_t *item, char *text);
void item_bind_even(item_t *item, void (*mouse_down)(), void (*mouse_up)());
void item_update(item_manager_t *item_manager, item_t *item, int mousex, int mousey, int status);
void item_change_status(item_manager_t *item_manager, item_t *item, int status);
void item_set_pos(item_t *item, int page, int x, int y);

#endif  //_ITEM_H
