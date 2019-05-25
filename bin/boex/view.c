#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "view.h"


/*
视图初始化，成功返回0
失败返回-1
*/
int view_init(view_t *view, int x, int y, int width, int height, int cur_width, int cur_height)
{
	view->x = x;
	view->y = y;
	view->width = width;
	view->height = height;

	view->cur_x = 0;	//偏移位置
	view->cur_y = 0;	
	view->cur_width = cur_width;
	view->cur_height = cur_height;
	view->scroll_bar = NULL;

	view->buffer = (uint32_t *)malloc(width*height*4);
	
	if (view->buffer == NULL) {
		return -1;
	}
	memset32(view->buffer, ARGB(255,240,240,240), width*height);
	//初始化buffer信息
	gui_buffer_set(view->x, view->y, view->width, view->height);

	return 0;
}

void view_color_face(view_t *view, uint32_t bcolor, uint32_t fcolor)
{
	view->bcolor = bcolor;
	view->fcolor = fcolor;
}

void view_draw(view_t *view)
{
	//绘制底层
	gui_buffer((uint8_t *)view->buffer);
	
	//绘制buffer内容
	gui_frame(view->cur_x, view->cur_y, view->cur_width, view->cur_height);
	//刷新到屏幕
	gui_draw(view->x, view->y, view->x + view->cur_width, view->y + view->cur_height);

}

void view_bind_scroll_bar(struct view_s *view, struct scroll_bar_s *scroll_bar)
{
	view->scroll_bar = scroll_bar;
}

void view_update(view_t *view, int mousex, int mousey, int status)
{
	
}

void view_point(view_t *view, int x, int y, uint32_t color)
{
	if (x < 0 || y < 0 || x >= view->width || y >= view->height) {
		return;
	}

	uint32_t *buffer = view->buffer + y*view->width + x;

	*buffer = color;
}

void view_rect(view_t *view, int x, int y, int width, int height, uint32_t color)
{
	int i, j;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			view_point(view, x + i, y + j, color);
		}
	}
}


void view_refresh(view_t *view, int x0, int y0, int x1, int y1)
{
	int width, height;

	//刷新不能超出当前范围
	if (x1 > view->cur_width) {
		x1 = view->cur_width;
	}

	if (y1 >= view->cur_height) {
		y1 = view->cur_height;
	}
	
	width = x1 - x0;
	height = y1 - y0;
	
	//绘制buffer内容
	gui_frame(view->cur_x + x0, view->cur_y + y0, width, height);
	
	//刷新到屏幕，只在对应的区域里面
	gui_draw(view->x + x0, view->y + y0, view->x + x1, view->y + y1);
}

