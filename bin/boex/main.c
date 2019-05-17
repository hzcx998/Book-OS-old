#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "button.h"
#include "status_bar.h"
#include "path_bar.h"
#include "frame.h"
#include "scroll_bar.h"

path_bar_t path_bar;

button_t button_forward, button_backward;	//目录前进和后退按钮

button_t button_setting, button_view;	//工具栏和视图方式

status_bar_t status_bar;

scroll_bar_t vscroll_bar, hscroll_bar;



int main(int argc, char *argv0[])
{
	printf("start book os explorer!\n");

	if (init_window(WIN_WIDTH, WIN_HEIGHT) == -1) {
		printf("book os explorer init window failed!\n");
		return -1;
	}
	//初始化窗口部件
	init_path_bar();
	init_button();
	init_status_bar();
	init_scroll_bar();


	int mx, my;
	int mouse_key, status;
	char s[40];
	memset(s, 0, 40);
	while (1) {
		
		if (!gui_mouse_move(&mx, &my)) {
			
			sprintf(s, "(%d, %d)", mx, my);

			gui_color(ARGB_GRAY);
			gui_rect(0, 300, 100, 20);

			gui_color(ARGB_WHITE);
			gui_text(1, 300+2, s);
			gui_draw(0, 300,100, 320);

			//按钮更新
			button_update(&button_backward, mx, my, status);
			button_update(&button_forward, mx, my, status);
			button_update(&button_setting, mx, my, status);
			button_update(&button_view, mx, my, status);

			//滚动栏更新
			scroll_bar_update(&vscroll_bar, mx, my, status);
			scroll_bar_update(&hscroll_bar, mx, my, status);
		}
		if (!gui_mouse_click(&mouse_key, &status, &mx, &my)) {	

			//按钮更新点击事件
			button_update(&button_backward, mx, my, status);
			button_update(&button_forward, mx, my, status);
			button_update(&button_setting, mx, my, status);
			button_update(&button_view, mx, my, status);

			//滚动栏更新
			scroll_bar_update(&vscroll_bar, mx, my, status);
			scroll_bar_update(&hscroll_bar, mx, my, status);
		}
		
		mouse_key = 0;
		status = 0;
	}
	return 0;
}

int init_window(int width, int height)
{
	window.width = width;
	window.height = height;
	
	//背景色为黑色
	window.bcolor = BACK_COLOR;
	//前景色为白色
	window.fcolor = FRONT_COLOR;
	
	if (gui_init() == -1) {
		return -1;
	}
	gui_window_size(window.width, window.height);
	gui_window_position(300, 24);
	if (gui_window_create(BOEX_NAME) == -1) {
		return -1;
	}

	//清空背景
	gui_color(window.bcolor);
	gui_rect(0, 0, window.width, window.height);

	//全部显示出来
	gui_draw(0,0,window.width, window.height);

	return 0;
}

void init_path_bar()
{
	path_bar_init(&path_bar, BUTTON_DEFAULT_WIDTH*2, 2, window.width-BUTTON_DEFAULT_WIDTH*4, PATH_BAR_HEIGHT);
	path_bar_color_face(&path_bar, BACK_COLOR, MIDDLE_COLOR, FRONT_COLOR);

	path_bar_set_path(&path_bar, "c:/");

	//绘制到屏幕
	gui_color(path_bar.bcolor);
	
	gui_rect(path_bar.x, path_bar.y, path_bar.width, path_bar.height);

	path_bar_draw(&path_bar);
}

void init_button()
{
	//向后退按钮
	button_init(&button_backward, 0, 2, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
	button_set_text(&button_backward, "<");
	button_color_status(&button_backward, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
	button_color_face(&button_backward, BACK_COLOR, ARGB_BLACK);
	button_bind_even(&button_backward, mouse_down_even_backward, mouse_up_even_backward);
	button_draw(&button_backward);
	
	//向前进按钮
	button_init(&button_forward, BUTTON_DEFAULT_WIDTH, 2, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
	button_set_text(&button_forward, ">");
	button_color_status(&button_forward, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
	button_color_face(&button_forward, BACK_COLOR, ARGB_BLACK);
	button_bind_even(&button_forward, mouse_down_even_forward, mouse_up_even_forward);
	button_draw(&button_forward);
	
	//视图方式按钮（列表还是图标）
	button_init(&button_view, window.width-BUTTON_DEFAULT_WIDTH*2, 2, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
	button_set_text(&button_view, "#");
	button_color_status(&button_view, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
	button_color_face(&button_view, BACK_COLOR, ARGB_BLACK);
	button_bind_even(&button_view, mouse_down_even_view, mouse_up_even_view);
	button_draw(&button_view);
	
	//工具按钮，工具或者设置
	button_init(&button_setting, window.width-BUTTON_DEFAULT_WIDTH, 2, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
	button_set_text(&button_setting, "@");
	button_color_status(&button_setting, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
	button_color_face(&button_setting, BACK_COLOR, ARGB_BLACK);
	button_bind_even(&button_setting, mouse_down_even_setting, mouse_up_even_setting);
	button_draw(&button_setting);
	
}

void init_status_bar()
{
	status_bar_init(&status_bar, 0, window.height-STATUS_BAR_HEIGHT_DEFAULT, window.width, STATUS_BAR_HEIGHT_DEFAULT);
	status_bar_color_status(&status_bar, BACK_COLOR, MIDDLE_COLOR, FRONT_COLOR);
	status_bar_color_face(&status_bar, BACK_COLOR, ARGB_BLACK);

	status_bar_set_text(&status_bar, "init done");
	status_bar_draw(&status_bar);
	
}

void init_scroll_bar()
{
	scroll_bar_init(&vscroll_bar, window.width-SCROLL_BAR_THICKNESS_DEFAULT, BUTTON_DEFAULT_HEIGHT, window.height-BUTTON_DEFAULT_HEIGHT*2, SCROLL_BAR_VSCROLL);
	scroll_bar_set_range(&vscroll_bar, 0, window.height);
	scroll_bar_set_pos(&vscroll_bar, 0);
	scroll_bar_set_page(&vscroll_bar, window.height);

	scroll_bar_draw(&vscroll_bar);

	scroll_bar_init(&hscroll_bar, 0, window.height-STATUS_BAR_HEIGHT_DEFAULT-SCROLL_BAR_THICKNESS_DEFAULT, window.width-SCROLL_BAR_THICKNESS_DEFAULT, SCROLL_BAR_HSCROLL);
	scroll_bar_set_range(&hscroll_bar, 0, window.width-SCROLL_BAR_THICKNESS_DEFAULT);
	scroll_bar_set_pos(&hscroll_bar, 0);
	scroll_bar_set_page(&hscroll_bar, window.width);

	scroll_bar_draw(&hscroll_bar);
	
}

void mouse_down_even_backward()
{
	status_bar_message(&status_bar, "backward mouse down");

}

void mouse_up_even_backward()
{
	status_bar_message(&status_bar, "backward mouse up");

}

void mouse_down_even_forward()
{
	status_bar_message(&status_bar, "forward mouse down");

}

void mouse_up_even_forward()
{
	status_bar_message(&status_bar, "forward mouse up");

}

void mouse_down_even_setting()
{
	status_bar_message(&status_bar, "setting mouse down");

}

void mouse_up_even_setting()
{
	status_bar_message(&status_bar, "setting mouse up");

}

void mouse_down_even_view()
{
	status_bar_message(&status_bar, "view mouse down");

}

void mouse_up_even_view()
{
	status_bar_message(&status_bar, "view mouse up");

}
