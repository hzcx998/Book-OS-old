/*
File:		kernel/debug.c
Contains:	debug for kernel
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/debug.h>
#include <sys/core.h>
#include <sys/dev.h>
#include <sys/gui.h>
#include <stdarg.h>
#include <stdio.h>

//停机并输出大量信息
void panic(const char *fmt, ...)
{
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	vsprintf(buf, fmt, arg);
	console_set_color(MAKE_COLOR(BLACK,RED));
	printk("\n>panic: %s", buf);
	
	while(1);
	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
//断言
void assertion_failure(char *exp, char *file, char *base_file, int line)
{
	console_set_color(MAKE_COLOR(BLACK,RED));
	printk("\nassert(%s) failed:\nfile: %s\nbase_file: %s\nln%d",
	exp, file, base_file, line);

	spin(">assertion_failure()");

	/* should never arrive here */
        __asm__ __volatile__("ud2");
}
//停机显示函数名
void spin(char * func_name)
{
	printk("\nspinning in %s", func_name);
	while(1);
}

bogui_debug_t bogui_debug;

void bogui_debug_init()
{
	bogui_debug.cursor_x = 0;
	bogui_debug.cursor_y = 0;

	bogui_debug.cursor_width = DEBUG_CHAR_WIDTH;
	bogui_debug.cursor_height = DEBUG_CHAR_HEIGHT;

	bogui_debug.win_width = DEBUG_WINDOW_WIDTH;
	bogui_debug.win_height = DEBUG_WINDOW_HEIGHT;

	bogui_debug.char_width = DEBUG_CHAR_WIDTH;
	bogui_debug.char_height = DEBUG_CHAR_HEIGHT;
	
	bogui_debug.rows = bogui_debug.win_width/bogui_debug.char_width;
	bogui_debug.columns = bogui_debug.win_height/bogui_debug.char_height;
	
	bogui_debug.bcolor = ARGB_POTATO_RED;
	bogui_debug.fcolor = ARGB(255, 220, 220, 220);

	bogui_debug.cursor_color = ARGB(255, 255, 255, 255);	

	bogui_debug.frame_pages = DEBUG_FRAME_PAGES;
	
	bogui_debug.word_buf_size = bogui_debug.rows*bogui_debug.columns*bogui_debug.frame_pages;

	bogui_debug.word_buf = (char *)mm.malloc(bogui_debug.word_buf_size);
	if (bogui_debug.word_buf == NULL) {
		panic("> init debug window failed!\n");
	}
	bogui_debug.word_buf_current_start = bogui_debug.word_buf;

	/*初始化调试窗口*/

	//创建容器
	bogui_debug.container = bogui_add_window(DEBUG_WINDOW_NAME, 0, 24, \
		bogui_debug.win_width, bogui_debug.win_height);

	if (bogui_debug.container == NULL) {
		panic("> init debug ->container failed!\n");
	}
	
	//创建窗口控制
	bogui_winctl_t *winctl = bogui_winctl_add(DEBUG_WINDOW_NAME);

	if (winctl == NULL) {
		//错误就关闭窗口
		bogui_close_window(bogui_debug.container->window);
		panic("> init debug ->winctl failed!\n");
	}
	/*互相绑定*/
	bogui_winctl_bind_window(winctl, bogui_debug.container->window);
	bogui_window_bind_winctl(bogui_debug.container->window, winctl);


	bogui_debug.window = bogui_debug.container->window;

	/*
	发送一个改变窗口聚焦的消息
	然后会在里面把窗口控制聚焦
	*/
	/*bogui_message_t msg;
	msg.id = BOGUI_MSG_WINDOW_FOUCS;
	msg.data.ptr[0] = bogui_debug.container;

	bogui_msg_send(&msg);
	*/
	bogui_container_axis(bogui_debug.container, bogui_container_manager->top - 1);
	
	//先把已有的聚焦窗口的标题栏变成未激活状态
	bogui_inactive_all_window();

	//把当前容器设置成激活状态
	bogui_window_active(bogui_debug.container);
	
	//把当前容器指向抓取的这个容器
	bogui_container_manager->current_container = bogui_debug.container;

	//聚焦窗口后，要聚焦一个窗口控制
	bogui_taskbar_winctl_focus(winctl);

	/*指向图形模式的文字显示*/
	display_char_func = debug_display_char;
	sys_clean_screen = debug_window_clean;

	//清屏
	debug_window_clean();

}

void debug_word_get(char *word, int x, int y)
{
	*word = bogui_debug.word_buf_current_start[y*bogui_debug.rows+x];
}

void debug_word_set(char word, int x, int y)
{
	//保存字符
	bogui_debug.word_buf_current_start[y*bogui_debug.rows+x] = word;
}

void debug_display_char(char ch)
{
	//清空原来位置的光标
	debug_cursor_clean();

	switch(ch){
		case '\n':
			//光标的位置设定一个字符
			debug_word_set(ch, bogui_debug.cursor_x, bogui_debug.cursor_y);

			//改变光标位置
			bogui_debug.cursor_x = 0;
			bogui_debug.cursor_y++;

			debug_cursor_check();

			//绘制新的光标
			debug_cursor_draw();

			break;
		case '\b':
			
			//改变光标位置
			bogui_debug.cursor_x--;

			debug_cursor_check();

			debug_word_set(0, bogui_debug.cursor_x, bogui_debug.cursor_y);

			//绘制新的光标
			debug_cursor_draw();

			break;
		default: 
			debug_word_set(ch, bogui_debug.cursor_x, bogui_debug.cursor_y);

			//绘制字符
			debug_char_draw(ch);

			//改变位置
			bogui_debug.cursor_x++;
			debug_cursor_check();

			//绘制新的光标
			debug_cursor_draw();


			break;
	}
	
	
}

void debug_window_clean()
{
	//复原位置
	bogui_debug.cursor_x = 0;
	bogui_debug.cursor_y = 0;

	//清除屏幕
	bogui_window_draw_rect(bogui_debug.window, 0,0, \
		bogui_debug.win_width, bogui_debug.win_height, bogui_debug.bcolor);
	//绘制光标
	bogui_window_draw_rect(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		bogui_debug.cursor_width, bogui_debug.cursor_height, bogui_debug.cursor_color);
	//刷新到窗口上去
	bogui_window_refresh(bogui_debug.window, 0, 0, bogui_debug.win_width, bogui_debug.win_height);
}

void debug_cursor_clean()
{
	//清除光标
	bogui_window_draw_rect(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		bogui_debug.cursor_width, bogui_debug.cursor_height, bogui_debug.bcolor);

	//刷新到窗口上去
	bogui_window_refresh(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		bogui_debug.cursor_x*bogui_debug.cursor_width + bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height + bogui_debug.cursor_height);
}

void debug_char_draw(char word)
{
	//清除光标
	bogui_window_draw_rect(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		bogui_debug.cursor_width, bogui_debug.cursor_height, bogui_debug.bcolor);

	//显示文字
	bogui_window_draw_char(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		word, bogui_debug.fcolor);

	//刷新到窗口上去
	bogui_window_refresh(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		bogui_debug.cursor_x*bogui_debug.cursor_width + bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height + bogui_debug.cursor_height);
}

void debug_cursor_draw()
{
	//清除光标
	bogui_window_draw_rect(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		bogui_debug.cursor_width, bogui_debug.cursor_height, bogui_debug.cursor_color);

	//刷新到窗口上去
	bogui_window_refresh(bogui_debug.window, \
		bogui_debug.cursor_x*bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height, \
		bogui_debug.cursor_x*bogui_debug.cursor_width + bogui_debug.cursor_width, \
		bogui_debug.cursor_y*bogui_debug.cursor_height + bogui_debug.cursor_height);
}

void debug_window_scroll()
{
	//如果超出字符范围
	if (!(bogui_debug.word_buf_current_start < bogui_debug.word_buf + \
		bogui_debug.word_buf_size - bogui_debug.rows*bogui_debug.columns)) {
		return;
	}

	//清除屏幕
	bogui_window_draw_rect(bogui_debug.window, 0,0, \
		bogui_debug.win_width, bogui_debug.win_height, bogui_debug.bcolor);


	//修改显存起始位置
	bogui_debug.word_buf_current_start += bogui_debug.rows;

	//把字符全部加载到窗口
	debug_load_word();

	//显示出来
	bogui_window_refresh(bogui_debug.window, 0, 0, bogui_debug.win_width, bogui_debug.win_height);

	//修改光标位置
	debug_cursor_draw();

}

void debug_load_word()
{
	int bx, by, x, y;
	char word;

	for (by = 0; by < bogui_debug.columns; by++) {
		for (bx = 0; bx < bogui_debug.rows; bx++) {
			debug_word_get(&word, bx, by);

			x = bx *  bogui_debug.char_width;
			y = by *  bogui_debug.char_height;
			
			//加载的时候只显示可显示字符
			if (0x20 <= word && word <= 0x7e) {
				bogui_window_draw_char(bogui_debug.window, \
					x, y, word, bogui_debug.fcolor);
			}

		}
	}
}

void debug_cursor_check()
{
	//如果光标向左移动超出，就切换到上一行最后
	if (bogui_debug.cursor_x < 0) {
		
		if (bogui_debug.cursor_y > 0) {
			//向左移动，如果发现y > 0，那么就可以移动行尾
			bogui_debug.cursor_x = bogui_debug.rows - 1;
		} else {
			//如果向左移动，发现y <= 0，那么就只能在行首
			bogui_debug.cursor_x = 0;
		}
		//移动到上一行
		bogui_debug.cursor_y--;
	}

	//如果光标向右移动超出，就切换到下一行
	if (bogui_debug.cursor_x > bogui_debug.rows - 1) {
		if (bogui_debug.cursor_y < bogui_debug.columns - 1) {
			//如果y 没有到达最后一行，就移动到行首
			bogui_debug.cursor_x = 0;
		} else {
			//如果y到达最后一行，就移动到行尾
			bogui_debug.cursor_x = bogui_debug.rows - 1;
		}
		//移动到下一行
		bogui_debug.cursor_y++;
	}

	//如果光标向上移动超出，就修复
	if (bogui_debug.cursor_y < 0) {
		//做修复处理
		bogui_debug.cursor_y = 0;

	}

	//如果光标向下移动超出，就向下滚动屏幕
	if (bogui_debug.cursor_y > bogui_debug.columns -1) {
		
		//暂时做修复处理
		bogui_debug.cursor_y = bogui_debug.columns-1;

		debug_window_scroll();
	}
}
