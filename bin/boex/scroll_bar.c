#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "scroll_bar.h"

void scroll_bar_init(scroll_bar_t *scroll_bar, int x, int y, int len, int type)
{
	scroll_bar->x = x;
	scroll_bar->y = y;
	scroll_bar->type = type;
	
	scroll_bar->thick = SCROLL_BAR_THICKNESS_DEFAULT;
	scroll_bar->len = len;
	
	scroll_bar->min = scroll_bar->max = 0;   //范围
    scroll_bar->page = 0;   //页面大小
    scroll_bar->pos = 0;    //位置
	scroll_bar->flags = SCROLL_BAR_DISABLENOSCROLL; 
	
	scroll_bar->off = -1;	//初始化为-1，表示没有偏移

	scroll_bar->block.moving = 0;	//没有移动

	scroll_bar->status = SCROLL_BAR_STATUS_IDLE;
	scroll_bar->mouse_down = NULL;
	scroll_bar->mouse_up = NULL;
	scroll_bar_color_status(scroll_bar, SCROLL_BAR_COLOR_IDLE, SCROLL_BAR_COLOR_TOUCH, SCROLL_BAR_COLOR_CLICK);
	scroll_bar_color_face(scroll_bar, SCROLL_BAR_COLOR_BACK, SCROLL_BAR_COLOR_FRONT);

}

void scroll_bar_set_range(scroll_bar_t *scroll_bar, uint32_t min, uint32_t max)
{
	scroll_bar->min = min;
	scroll_bar->max = max;
	scroll_bar->flags |= SCROLL_BAR_RANGE;

	if (scroll_bar->type == SCROLL_BAR_VSCROLL) {	//垂直
		scroll_bar->block.width = scroll_bar->thick;
		scroll_bar->block.height = 20;	//计算求出
	} else if (scroll_bar->type == SCROLL_BAR_HSCROLL) {	//水平
		scroll_bar->block.width = 20;	//计算求出
		scroll_bar->block.height = scroll_bar->thick;
	}
	
}

void scroll_bar_set_pos(scroll_bar_t *scroll_bar, uint32_t pos)
{
	scroll_bar->pos = pos;
	scroll_bar->flags |= SCROLL_BAR_POSE;

	if (scroll_bar->type == SCROLL_BAR_VSCROLL) {	//垂直
		scroll_bar->block.x = scroll_bar->x;
		scroll_bar->block.y = scroll_bar->y + pos;	//计算得出
	} else if (scroll_bar->type == SCROLL_BAR_HSCROLL) {	//水平
		scroll_bar->block.x = scroll_bar->x + pos;	//计算得出
		scroll_bar->block.y = scroll_bar->y;	
	}

	
}

void scroll_bar_set_page(scroll_bar_t *scroll_bar, uint32_t page)
{
	scroll_bar->page = page;
	scroll_bar->flags |= SCROLL_BAR_PAGE;
}

void scroll_bar_set_flags(scroll_bar_t *scroll_bar, uint32_t flags)
{
	scroll_bar->flags = flags;
}
void scroll_bar_color_status(scroll_bar_t *scroll_bar, uint32_t idle, uint32_t touch, uint32_t click)
{
	scroll_bar->color_idle = idle;
	scroll_bar->color_touch = touch;
	scroll_bar->color_click = click;
}

void scroll_bar_color_face(scroll_bar_t *scroll_bar, uint32_t bcolor, uint32_t fcolor)
{
	scroll_bar->bcolor = bcolor;
	scroll_bar->fcolor = fcolor;
}

void scroll_bar_bind_even(scroll_bar_t *scroll_bar, void (*mouse_down)(), void (*mouse_up)())
{
	scroll_bar->mouse_down = mouse_down;
	scroll_bar->mouse_up = mouse_up;
}

void scroll_bar_change_status(scroll_bar_t *scroll_bar, int status)
{
	scroll_bar->status = status;
	scroll_bar_draw(scroll_bar);
}

void scroll_bar_draw(scroll_bar_t *scroll_bar)
{
	uint32_t color;
	if (scroll_bar->status == SCROLL_BAR_STATUS_IDLE) {
		color = scroll_bar->color_idle;
	} else if (scroll_bar->status == SCROLL_BAR_STATUS_TOUCH) {
		color = scroll_bar->color_touch;
	} else if (scroll_bar->status == SCROLL_BAR_STATUS_CLICK) {
		color = scroll_bar->color_click;
	}

	//绘制底层
	gui_color(scroll_bar->bcolor);

	if (scroll_bar->type == SCROLL_BAR_VSCROLL) {	//垂直
		gui_rect(scroll_bar->x, scroll_bar->y, scroll_bar->thick, scroll_bar->len);

		//绘制滑块
		gui_color(color);
		gui_rect(scroll_bar->block.x, scroll_bar->block.y, scroll_bar->block.width, scroll_bar->block.height);

		//刷新
		gui_draw(scroll_bar->x, scroll_bar->y, scroll_bar->x + scroll_bar->thick, scroll_bar->y + scroll_bar->len);

	} else if (scroll_bar->type == SCROLL_BAR_HSCROLL) {	//水平

		gui_rect(scroll_bar->x, scroll_bar->y, scroll_bar->len, scroll_bar->thick);

		//绘制滑块
		gui_color(color);
		gui_rect(scroll_bar->block.x, scroll_bar->block.y, scroll_bar->block.width, scroll_bar->block.height);

		//刷新
		gui_draw(scroll_bar->x, scroll_bar->y, scroll_bar->x + scroll_bar->len, scroll_bar->y + scroll_bar->thick);

	}
	
	
}

void scroll_bar_update(scroll_bar_t *scroll_bar, int mousex, int mousey, int status)
{
	scrool_bar_blocl_t *block = &scroll_bar->block;
	
	//判断鼠标位置
	if (block->x <= mousex && mousex < block->x + block->width && \
		block->y <= mousey && mousey < block->y + block->height ) 
	{
		//在范围内
		
		//改变颜色
		if (scroll_bar->status == SCROLL_BAR_STATUS_IDLE) {
			scroll_bar->status = SCROLL_BAR_STATUS_TOUCH;
			scroll_bar_draw(scroll_bar);
		}

		//如果鼠标点击
		if (status == MOUSE_DOWN) {

			//先做内部操作
			scroll_bar_change_status(scroll_bar, SCROLL_BAR_STATUS_CLICK);
			
			if (scroll_bar->type == SCROLL_BAR_VSCROLL) {	//垂直

				//第一次点击的时候捕捉偏移
				if (scroll_bar->off == -1 && !scroll_bar->block.moving) {
					scroll_bar->off = mousey - block->y;
					scroll_bar->block.moving = 1;
				}
				
			} else if (scroll_bar->type == SCROLL_BAR_HSCROLL && !scroll_bar->block.moving) {	//水平
				//第一次点击的时候捕捉偏移
				if (scroll_bar->off == -1) {
					scroll_bar->off = mousex - block->x;
					scroll_bar->block.moving = 1;
				}
				
			}

			if (scroll_bar->mouse_down != NULL) {
				//执行鼠标其它事件
				scroll_bar->mouse_down();
			}
		} /*else if (status == MOUSE_UP) {	//在范围内弹起
			//先做内部操作
			scroll_bar_change_status(scroll_bar, SCROLL_BAR_STATUS_TOUCH);
			
			//清除偏移
			scroll_bar->off = -1;
			scroll_bar->block.moving = 0;

			if (scroll_bar->mouse_up != NULL) {
				//执行鼠标其它事件
				scroll_bar->mouse_up();
			}
		} */

	} else {
		//改变颜色
		if (scroll_bar->status != SCROLL_BAR_STATUS_IDLE) {
			scroll_bar->status = SCROLL_BAR_STATUS_IDLE;
			scroll_bar_draw(scroll_bar);
		}
		
	}


	//滑动块不一定在范围内弹起
	if (status == MOUSE_UP) {	//不在范围内弹起
		//清除偏移
		scroll_bar->off = -1;
		//不移动
		scroll_bar->block.moving = 0;

		if (scroll_bar->mouse_up != NULL) {
			//执行鼠标其它事件
			scroll_bar->mouse_up();
		}
	} 

	if (scroll_bar->block.moving == 1) {
		if (scroll_bar->type == SCROLL_BAR_VSCROLL) {	//垂直
			//滑块y和鼠标一样
			block->y = mousey - scroll_bar->off;

			//修复位置
			if (block->y < scroll_bar->y) {
				block->y = scroll_bar->y;
			}
			if (block->y > scroll_bar->y + scroll_bar->len - block->height) {
				block->y = scroll_bar->y + scroll_bar->len - block->height;
			}

		} else if (scroll_bar->type == SCROLL_BAR_HSCROLL) {	//水平
			//滑块x和鼠标一样
			block->x = mousex - scroll_bar->off;

			//修复位置
			if (block->x < scroll_bar->x) {
				block->x = scroll_bar->x;
			}
			if (block->x > scroll_bar->x + scroll_bar->len - block->width) {
				block->x = scroll_bar->x + scroll_bar->len - block->width;
			}
			
		}
		scroll_bar_draw(scroll_bar);
			
	}

	
}





