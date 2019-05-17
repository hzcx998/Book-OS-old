#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "frame.h"

void frame_init(frame_t *frame, int x, int y, int width, int height)
{
	frame->x = x;
	frame->y = y;
	frame->width = width;
	frame->height = height;
	frame->status = FRAME_STATUS_IDLE;
	frame->mouse_down = NULL;
	frame->mouse_up = NULL;
}

void frame_set_text(frame_t *frame, char *text)
{
	memset(frame->text, 0, FRAME_TEXT_LEN);
	strcpy(frame->text, text);
}

void frame_color_status(frame_t *frame, uint32_t idle, uint32_t touch, uint32_t click)
{
	frame->color_idle = idle;
	frame->color_touch = touch;
	frame->color_click = click;
}

void frame_color_face(frame_t *frame, uint32_t bcolor, uint32_t fcolor)
{
	frame->bcolor = bcolor;
	frame->fcolor = fcolor;
}

void frame_bind_even(frame_t *frame, void (*mouse_down)(), void (*mouse_up)())
{
	frame->mouse_down = mouse_down;
	frame->mouse_up = mouse_up;
}

void frame_change_status(frame_t *frame, int status)
{
	frame->status = status;
	frame_draw(frame);
}

void frame_draw(frame_t *frame)
{
	uint32_t color;
	if (frame->status == FRAME_STATUS_IDLE) {
		color = frame->color_idle;
	} else if (frame->status == FRAME_STATUS_TOUCH) {
		color = frame->color_touch;
	} else if (frame->status == FRAME_STATUS_CLICK) {
		color = frame->color_click;
	}

	//绘制底层
	gui_color(color);
	gui_rect(frame->x, frame->y, frame->width, frame->height);

	//绘制按钮文字
	gui_color(frame->fcolor);
	int x, y;
	x = frame->width/2 - strlen(frame->text)*8/2;
	y = frame->height/2 - 16/2;
	gui_text(frame->x + x, frame->y + y, frame->text);

	gui_draw(frame->x, frame->y, frame->x + frame->width, frame->y + frame->height);

}

void frame_update(frame_t *frame, int mousex, int mousey, int status)
{
	//判断鼠标位置
	if (frame->x <= mousex && mousex < frame->x + frame->width && \
		frame->y <= mousey && mousey < frame->y + frame->height ) 
	{
		//在范围内
		
		//改变颜色
		if (frame->status == FRAME_STATUS_IDLE) {
			frame->status = FRAME_STATUS_TOUCH;
			frame_draw(frame);
		}

		//如果鼠标点击
		if (status == MOUSE_DOWN) {
			//先做内部操作
			frame_change_status(frame, FRAME_STATUS_CLICK);

			if (frame->mouse_down != NULL) {
				//执行鼠标其它事件
				frame->mouse_down();
			}
		} else if (status == MOUSE_UP) {
			//先做内部操作
			frame_change_status(frame, FRAME_STATUS_TOUCH);

			if (frame->mouse_up != NULL) {
				//执行鼠标其它事件
				frame->mouse_up();
			}
		} 

	} else {
		//改变颜色
		if (frame->status != FRAME_STATUS_IDLE) {
			frame->status = FRAME_STATUS_IDLE;
			frame_draw(frame);
		}
	}
	
}





