/*
File:		gui/bogui/bogui.c
Contains:	gui module
Auther:		Hu Zicheng
Time:		2019/4/7
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/core.h>

#include <sys/dev.h>
#include <sys/gui.h>

#include <string.h>


struct gui gui;

void init_gui()
{
	gui_environment_init();
	gui_interface_init();
	
}

void thread_gui(void *arg)
{
	mouse.x = video_info.width/2 - 8;		//鼠标信息初始化
	mouse.y = video_info.height/2 - 8;
	
	while(1){
		#ifdef _CONFIG_GUI_BOGUI_
			bogui_do_thread();
			
		#endif

	}
}


void gui_environment_init()
{
	#ifdef _CONFIG_GUI_BASIC_
		init_gui_basic();
	#endif
	#ifdef _CONFIG_GUI_BOGUI_
		init_bogui();
	#endif
}


void gui_interface_init()
{
	#ifdef _CONFIG_GUI_BOGUI_
		gui.init = bogui_guilib_init;
		gui.window_size = bogui_guilib_window_size;
		gui.window_position = bogui_guilib_window_position;
		gui.window_create = bogui_guilib_window_create;
		gui.window_close = bogui_guilib_window_close;
		
		gui.color = bogui_guilib_color;
		gui.point = bogui_guilib_point;
		gui.line = bogui_guilib_line;
		gui.rect = bogui_guilib_rect;
		gui.text = bogui_guilib_text;

		gui.buffer = bogui_guilib_buffer;
		gui.frame = bogui_guilib_frame;

		gui.draw = bogui_guilib_draw;

		gui.mouse_move = bogui_guilib_mouse_move;
		gui.mouse_click = bogui_guilib_mouse_click;

		gui.keyboard = bogui_guilib_keyboard;
		gui.get_modifiers = bogui_guilib_get_modifiers;


	#endif
	
}
