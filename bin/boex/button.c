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
	button->mouse_down = NULL;
	button->mouse_up = NULL;
	
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

void button_bind_even(button_t *button, void (*mouse_down)(), void (*mouse_up)())
{
	button->mouse_down = mouse_down;
	button->mouse_up = mouse_up;
}

void button_change_status(button_t *button, int status)
{
	button->status = status;
	button_draw(button);
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
	if (button->text[0] != 0) {
		gui_color(button->fcolor);
		int x, y;
		x = button->width/2 - strlen(button->text)*8/2;
		y = button->height/2 - 16/2;
		gui_text(button->x + x, button->y + y, button->text);
	}
	
	gui_draw(button->x, button->y, button->x + button->width, button->y + button->height);

}

void button_update(button_t *button, int mousex, int mousey, int status)
{
	//判断鼠标位置
	if (button->x <= mousex && mousex < button->x + button->width && \
		button->y <= mousey && mousey < button->y + button->height ) 
	{
		//在范围内
		
		//改变颜色
		if (button->status == BUTTON_STATUS_IDLE) {
			button->status = BUTTON_STATUS_TOUCH;
			button_draw(button);
		}

		//如果鼠标点击
		if (status == MOUSE_DOWN) {
			//先做内部操作
			button_change_status(button, BUTTON_STATUS_CLICK);

			if (button->mouse_down != NULL) {
				//执行鼠标其它事件
				button->mouse_down(mousex, mousey);
			}
		} else if (status == MOUSE_UP) {
			//先做内部操作
			button_change_status(button, BUTTON_STATUS_TOUCH);

			if (button->mouse_up != NULL) {
				//执行鼠标其它事件
				button->mouse_up(mousex, mousey);
			}
		} 

	} else {
		//改变颜色
		if (button->status != BUTTON_STATUS_IDLE) {
			button->status = BUTTON_STATUS_IDLE;
			button_draw(button);
		}
	}
	
}





