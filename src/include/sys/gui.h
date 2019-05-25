#ifndef _GUI_H
#define _GUI_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BASIC_
	
	#include <gui/basic/basic.h>
	#include <gui/basic/graphic.h>
	#include <gui/basic/bmp.h>
	#include <gui/basic/font.h>
	#include <gui/basic/image.h>
	#include <gui/basic/jpeg.h>

#endif

#ifdef _CONFIG_GUI_BOGUI_
	
	#include <gui/bogui/bogui.h>
	#include <gui/bogui/libary.h>
	#include <gui/bogui/container.h>
	#include <gui/bogui/image.h>
	#include <gui/bogui/window.h>
	#include <gui/bogui/message.h>
	#include <gui/bogui/taskbar.h>
	
#endif

#include <gui/color.h>

struct gui
{
	int (*init)();
	void (*window_size)(int width, int height);
	void (*window_position)(int x, int y);
	int (*window_create)(const char *title);
	int (*window_close)();

	void (*color)(unsigned int color);
	void (*point)(int x, int y);
	void (*line)(int x1, int y1, int x2, int y2);
	void (*rect)(int x, int y, int width, int height);
	void (*text)(int x, int y, char *text);

	void (*buffer)(unsigned char *buffer);
	void (*buffer_set)(int x, int y, int width, int height);
	void (*frame)(int x, int y, int width, int height);

	void (*draw)(int x1, int y1, int x2, int y2);

	int (*mouse_move)(int *x, int *y);
	int (*mouse_click)(int *key, int *status, int *x, int *y);
	int (*keyboard)(int *key);
	int (*get_modifiers)();
	
};

extern struct gui gui;

void init_gui();
void thread_gui(void *arg);
void gui_environment_init();
void gui_interface_init();

#endif //_GUI_H