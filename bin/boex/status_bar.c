#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "status_bar.h"

void status_bar_init(status_bar_t *status_bar, int x, int y, int width, int height)
{
	status_bar->x = x;
	status_bar->y = y;
	status_bar->width = width;
	status_bar->height = height;
	
}

void status_bar_color_status(status_bar_t *status_bar, uint32_t idle, uint32_t touch, uint32_t click)
{
	status_bar->color_idle = idle;
	status_bar->color_touch = touch;
	status_bar->color_click = click;
}

void status_bar_color_face(status_bar_t *status_bar, uint32_t bcolor, uint32_t fcolor)
{
	status_bar->bcolor = bcolor;
	status_bar->fcolor = fcolor;
}

void status_bar_set_text(status_bar_t *status_bar, char *text)
{
	memset(status_bar->text, 0, STATUS_BAR_TEXT_LEN);
	strcpy(status_bar->text, text);
}

void status_bar_draw(status_bar_t *status_bar)
{
	//绘制背景
	gui_color(status_bar->bcolor);
	gui_rect(status_bar->x, status_bar->y, status_bar->width, status_bar->height);

	//绘制文字
	gui_color(status_bar->fcolor);
	int x, y;
	x = 1;
	y = 4;
	gui_text(status_bar->x + x, status_bar->y + y, status_bar->text);

	//绘制按钮

	//绘制到屏幕上
	gui_draw(status_bar->x, status_bar->y, status_bar->x + status_bar->width, status_bar->y + status_bar->height);
}

void status_bar_message(status_bar_t *status_bar, const char *fmt, ...)
{
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	status_bar_set_text(status_bar, buf);
	status_bar_draw(status_bar);
	
}