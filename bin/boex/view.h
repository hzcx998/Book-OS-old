#ifndef _VIEW_H
#define _VIEW_H

#include <stdint.h>
#include "scroll_bar.h"

#define VIEW_DEFAULT_WIDTH  24
#define VIEW_DEFAULT_HEIGHT  24

/*
view
*/
typedef struct view_s
{
    int x, y;   //视图起始位置
    int width, height;  //视图总大小
    int cur_x, cur_y;   //当前视图起始位置
    int cur_width, cur_height;  //当前视图总大小
    uint32_t *buffer;

    uint32_t bcolor, fcolor;    //背景色和前景色
    struct scroll_bar_s *scroll_bar;
}view_t;

int view_init(view_t *view, int x, int y, int width, int height, int cur_width, int cur_height);
void view_color_status(view_t *view, uint32_t idle, uint32_t touch, uint32_t click);
void view_color_face(view_t *view, uint32_t bcolor, uint32_t fcolor);
void view_draw(view_t *view);
void view_bind_scroll_bar(struct view_s *view, struct scroll_bar_s *scroll_bar);

void view_update(view_t *view, int mousex, int mousey, int status);

void view_point(view_t *view, int x, int y, uint32_t color);
void view_rect(view_t *view, int x, int y, int width, int height, uint32_t color);
void view_refresh(view_t *view, int x0, int y0, int x1, int y1);

#endif  //_VIEW_H
