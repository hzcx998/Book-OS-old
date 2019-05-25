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
#include "view.h"
#include "item.h"

path_bar_t path_bar;

button_t button_forward, button_backward;	//目录前进和后退按钮

button_t button_setting, button_view;	//工具栏和视图方式

status_bar_t status_bar;

item_manager_t catalog_item_manager, favorite_item_manager;

int main(int argc, char *argv0[])
{
	//printf("start book os explorer!\n");

	if (init_window(WIN_WIDTH, WIN_HEIGHT) == -1) {
		printf("book os explorer init window failed!\n");
		return -1;
	}
	//初始化窗口部件
	init_path_bar();
	
	init_button();
	init_status_bar();
	if (init_item() == -1) {
		printf("init item failed!\n");
		return -1;
	}
	
	//printf("init book os explorer done!\n");

	/*init_scroll_bar();
	init_view();*/

	int mx, my;
	int mouse_key, status;
	char s[40];
	memset(s, 0, 40);
	while (1) {
		
		if (!gui_mouse_move(&mx, &my)) {
			/*sprintf(s, "(%d, %d)", mx, my);

			gui_color(ARGB_GRAY);
			gui_rect(0, 300, 100, 20);

			gui_color(ARGB_WHITE);
			gui_text(1, 300+2, s);
			gui_draw(0, 300,100, 320);*/

			//按钮更新
			button_update(&button_backward, mx, my, status);
			button_update(&button_forward, mx, my, status);
			button_update(&button_setting, mx, my, status);
			button_update(&button_view, mx, my, status);

			//滚动栏更新
			/*scroll_bar_update(&vscroll_bar, mx, my, status);
			scroll_bar_update(&hscroll_bar, mx, my, status);*/

			item_manager_update(&catalog_item_manager, mx, my, status);
			item_manager_update(&favorite_item_manager, mx, my, status);

		}
		if (!gui_mouse_click(&mouse_key, &status, &mx, &my)) {	

			//按钮更新点击事件
			button_update(&button_backward, mx, my, status);
			button_update(&button_forward, mx, my, status);
			button_update(&button_setting, mx, my, status);
			button_update(&button_view, mx, my, status);

			//滚动栏更新
			/*scroll_bar_update(&vscroll_bar, mx, my, status);
			scroll_bar_update(&hscroll_bar, mx, my, status);*/
			item_manager_update(&catalog_item_manager, mx, my, status);
			item_manager_update(&favorite_item_manager, mx, my, status);

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
	gui_window_position(100, 24);
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
	path_bar_init(&path_bar, BUTTON_DEFAULT_WIDTH*2, 0, window.width-BUTTON_DEFAULT_WIDTH*4, PATH_BAR_HEIGHT);
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
	button_init(&button_backward, 0, 0, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
	button_set_text(&button_backward, "<");
	button_color_status(&button_backward, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
	button_color_face(&button_backward, BACK_COLOR, ARGB_BLACK);
	button_bind_even(&button_backward, mouse_down_even_backward, mouse_up_even_backward);
	button_draw(&button_backward);
	
	//向前进按钮
	button_init(&button_forward, BUTTON_DEFAULT_WIDTH, 0, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
	button_set_text(&button_forward, ">");
	button_color_status(&button_forward, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
	button_color_face(&button_forward, BACK_COLOR, ARGB_BLACK);
	button_bind_even(&button_forward, mouse_down_even_forward, mouse_up_even_forward);
	button_draw(&button_forward);
	
	//视图方式按钮（列表还是图标）
	button_init(&button_view, window.width-BUTTON_DEFAULT_WIDTH*2, 0, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
	button_set_text(&button_view, "#");
	button_color_status(&button_view, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
	button_color_face(&button_view, BACK_COLOR, ARGB_BLACK);
	button_bind_even(&button_view, mouse_down_even_view, mouse_up_even_view);
	button_draw(&button_view);
	
	//工具按钮，工具或者设置
	button_init(&button_setting, window.width-BUTTON_DEFAULT_WIDTH, 0, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
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
/*
void init_scroll_bar()
{
	scroll_bar_init(&vscroll_bar, window.width-SCROLL_BAR_THICKNESS_DEFAULT, BUTTON_DEFAULT_HEIGHT, \
		window.height-(BUTTON_DEFAULT_HEIGHT+STATUS_BAR_HEIGHT_DEFAULT+SCROLL_BAR_THICKNESS_DEFAULT), SCROLL_BAR_VSCROLL);
	scroll_bar_set_range(&vscroll_bar, 0, 600);
	scroll_bar_set_pos(&vscroll_bar, 0);
	scroll_bar_set_page(&vscroll_bar, window.height);
	scroll_bar_set_flags(&vscroll_bar, SCROLL_BAR_ALL);
	scroll_bar_bind_even(&vscroll_bar, mouse_down_even_vscroll_bar, mouse_up_even_vscroll_bar);

	scroll_bar_draw(&vscroll_bar);

	scroll_bar_init(&hscroll_bar, 0, window.height-STATUS_BAR_HEIGHT_DEFAULT-SCROLL_BAR_THICKNESS_DEFAULT, window.width-SCROLL_BAR_THICKNESS_DEFAULT, SCROLL_BAR_HSCROLL);
	scroll_bar_set_range(&hscroll_bar, 0, 800);
	scroll_bar_set_pos(&hscroll_bar, 0);
	scroll_bar_set_page(&hscroll_bar, window.width);
	scroll_bar_set_flags(&hscroll_bar, SCROLL_BAR_ALL);
	scroll_bar_bind_even(&hscroll_bar, mouse_down_even_hscroll_bar, mouse_up_even_hscroll_bar);

	scroll_bar_draw(&hscroll_bar);
	
}

void init_view()
{
	
	if (!view_init(&file_view, 0, BUTTON_DEFAULT_HEIGHT, \
		800, 600, \
		window.width-SCROLL_BAR_THICKNESS_DEFAULT, \
		window.height-(BUTTON_DEFAULT_HEIGHT+STATUS_BAR_HEIGHT_DEFAULT+SCROLL_BAR_THICKNESS_DEFAULT)-20) )
	{

		view_color_face(&file_view, BACK_COLOR, FRONT_COLOR);
		view_draw(&file_view);

		scroll_bar_bind_view(&hscroll_bar, &file_view);
		scroll_bar_bind_view(&vscroll_bar, &file_view);
		view_point(&file_view, 10, 10, ARGB_BLUE);
		view_point(&file_view, 20, 20, ARGB_RED);
		view_point(&file_view, 30, 30, ARGB_GREEN);
		
		view_point(&file_view, 400, 20, ARGB_RED);
		view_point(&file_view, 440, 30, ARGB_GREEN);
		view_point(&file_view, 480, 30, ARGB_GREEN);
		view_point(&file_view, 500, 30, ARGB_GREEN);
		
		view_rect(&file_view, 50, 50, 100, 50, ARGB_GREEN);

		view_rect(&file_view, 350, 350, 50, 100, ARGB_BLUE);
		
		view_rect(&file_view, 700, 500, 80, 60, ARGB_RED);
		view_refresh(&file_view, 0, 0, file_view.cur_width, file_view.cur_height);

	}
}
*/
int init_item()
{
	item_t *item;
	int i;
	/*
	初始化favorite item管理
	*/
	if (item_manager_init(&favorite_item_manager, MAX_FAST_ITEM_NR ,0, 24,ITEM_NAME_WIDTH+2*8, \
		window.height-(BUTTON_DEFAULT_HEIGHT+STATUS_BAR_HEIGHT_DEFAULT), ITEM_FAVORITE_WIDTH) == -1) {
		return -1;
	}

	for (i = 0; i < 8; i++) {
		item = item_manager_alloc(&favorite_item_manager);
		item_draw(&favorite_item_manager, item);
	}

	/*
	初始化catalog item管理
	*/
	if (item_manager_init(&catalog_item_manager, MAX_FILE_ITEM_NR ,ITEM_NAME_WIDTH+2*8, 24,window.width - ITEM_NAME_WIDTH+2*8, \
		window.height-(BUTTON_DEFAULT_HEIGHT+STATUS_BAR_HEIGHT_DEFAULT), ITEM_CATALOG_WIDTH) == -1) {
		return -1;
	}
	
	for (i = 0; i < 35; i++) {
		item = item_manager_alloc(&catalog_item_manager);
		item_draw(&catalog_item_manager, item);
	}
	

	return 0;
}


void mouse_down_even_vscroll_bar(int mx, int my)
{
	status_bar_message(&status_bar, "vscroll bar mouse down");

}

void mouse_up_even_vscroll_bar(int mx, int my)
{
	status_bar_message(&status_bar, "vscroll bar mouse up");
	
}


void mouse_down_even_hscroll_bar(int mx, int my)
{
	status_bar_message(&status_bar, "hscroll bar mouse down");

	/*file_view.cur_x = hscroll_bar.block.x;
	view_draw(&file_view);*/
}

void mouse_up_even_hscroll_bar(int mx, int my)
{
	status_bar_message(&status_bar, "hscroll bar mouse up");
	
}


void mouse_down_even_backward(int mx, int my)
{
	status_bar_message(&status_bar, "backward mouse down");

}

void mouse_up_even_backward(int mx, int my)
{
	status_bar_message(&status_bar, "backward mouse up");

}

void mouse_down_even_forward(int mx, int my)
{
	status_bar_message(&status_bar, "forward mouse down");

}

void mouse_up_even_forward(int mx, int my)
{
	status_bar_message(&status_bar, "forward mouse up");

}

void mouse_down_even_setting(int mx, int my)
{
	status_bar_message(&status_bar, "setting mouse down");

}

void mouse_up_even_setting(int mx, int my)
{
	status_bar_message(&status_bar, "setting mouse up");

}

void mouse_down_even_view(int mx, int my)
{
	status_bar_message(&status_bar, "view mouse down");

}

void mouse_up_even_view(int mx, int my)
{
	status_bar_message(&status_bar, "view mouse up");

}


