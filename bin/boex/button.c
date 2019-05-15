#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "button.h"

void button_init(button_t *button, int x, int y, int width, int height)
{
	button->x = x;
	button->y = y;
	button->width = width;
	button->height = height;
	button->status = BUTTON_STATUS_IDLE;

}

void button_set_text(button_t *button, char *text)
{
	memset(button->text, 0, BUTTON_TEXT_LEN);
	strcpy(button->text, text);
}

void button_color_status(button_t *button, uint32_t idle, uint32_t touch, uint32_t click)
{
	button->color_idle = idle;
	button->color_touch = touch;
	button->color_click = click;
}

void button_color_face(button_t *button, uint32_t bcolor, uint32_t fcolor)
{
	button->bcolor = bcolor;
	button->fcolor = fcolor;
}

void button_draw(button_t *button)
{
	uint32_t color;
	if (button->status == BUTTON_STATUS_IDLE) {
		color = button->color_idle;
	} else if (button->status == BUTTON_STATUS_TOUCH) {
		color = button->color_touch;
	} else if (button->status == BUTTON_STATUS_CLICK) {
		color = button->color_click;
	}

	//绘制底层
	gui_color(color);
	gui_rect(button->x, button->y, button->width, button->height);

	//绘制按钮文字
	gui_color(button->fcolor);
	int x, y;
	x = button->width/2 - strlen(button->text)*8/2;
	y = button->height/2 - 16/2;
	gui_text(button->x + x, button->y + y, button->text);

	gui_draw(button->x, button->y, button->width, button->height);

}

void button_update(button_t *button, int mousex, int mousey, int status)
{
	//判断鼠标位置
	if (button->x <= mousex && mousex < button->x + button->width && \
		button->y <= mousey && mousey < button->y + button->height ) 
	{

	}

}