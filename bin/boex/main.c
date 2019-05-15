#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "button.h"

path_bar_t path_bar;

button_t button_forward, botton_backward;

int main(int argc, char *argv0[])
{
	printf("start book os explorer!\n");

	if (init_window(WIN_WIDTH, WIN_HEIGHT) == -1) {
		printf("book os explorer init window failed!\n");
		return -1;
	}

	init_path_bar();

	init_path_button();

	int mx, my;
	char s[40];
	memset(s, 0, 40);
	while (1) {
		if (!gui_mouse_move(&mx, &my)) {
			sprintf(s, "(%d, %d)", mx, my);

			gui_color(ARGB_GRAY);
			gui_rect(0, 300, 100, 20);

			gui_color(COLOR_WHITE);
			gui_text(1, 300+2, s);
			
			
		}
		gui_draw(0, 300,100, 320);
	}
	return 0;
}

void init_path_bar()
{
	path_bar.x = 40;
	path_bar.y = 0;
	path_bar.width = PATH_BAR_WIDTH;
	path_bar.height = PATH_BAR_HEIGHT;
	
	path_bar.bcolor = ARGB(255,128,128,128);
	path_bar.fcolor = ARGB(255,255,255,255);
	
	memset(path_bar.path, 0, PATH_NAME_LEN);
	strcpy(path_bar.path, "c:/");

	//绘制到屏幕
	gui_color(path_bar.bcolor);
	gui_rect(path_bar.x, path_bar.y, path_bar.width, path_bar.height);

	gui_color(path_bar.fcolor);
	gui_text(path_bar.x+1, path_bar.y+2, path_bar.path);

	gui_draw(path_bar.x, path_bar.y,path_bar.x + path_bar.width, path_bar.y + path_bar.height);
}

void path_bar_refresh()
{
	//绘制到屏幕
	gui_color(path_bar.bcolor);
	gui_rect(path_bar.x, path_bar.y, path_bar.width, path_bar.height);

	gui_color(path_bar.fcolor);
	gui_text(path_bar.x+1, path_bar.y+2, path_bar.path);

	gui_draw(path_bar.x, path_bar.y,path_bar.x + path_bar.width, path_bar.y + path_bar.height);
}

int init_window(int width, int height)
{
	window.width = width;
	window.height = height;
	
	//背景色为黑色
	window.bcolor = ARGB_POTATO_RED;
	//前景色为白色
	window.fcolor = ARGB(255,255,255,255);
	
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

void init_path_button()
{
	//向后退按钮
	button_init(&botton_backward, 0, 0, 20, 20);
	button_set_text(&botton_backward, "<");
	button_color_status(&botton_backward, ARGB_GRAY, ARGB_GREEN, ARGB_BLUE);
	button_color_face(&botton_backward, ARGB_BLACK, ARGB_WHITE);
	button_draw(&botton_backward);
	
	//向前进按钮
	button_init(&button_forward, 20, 0, 20, 20);
	button_set_text(&button_forward, ">");
	button_color_status(&button_forward, ARGB_GRAY, ARGB_GREEN, ARGB_BLUE);
	button_color_face(&button_forward, ARGB_BLACK, ARGB_WHITE);
	button_draw(&button_forward);
	
}