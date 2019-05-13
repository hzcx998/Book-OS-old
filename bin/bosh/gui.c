#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>

#include "gui.h"
#include "bosh.h"

void bosh();
/*
用一个字符缓冲区保存所有要显示的字符。
*/
void bosh()
{
	//初始化窗口信息
	if (init_window(WIN_WIDTH, WIN_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT) == -1) {
		return;
	}

	int key, modi;

	while(1){
		
		if (!gui_keyboard(&key)) {
			modi = gui_get_modifiers();
			/*
			控制按键
			*/
			
			if(key == GUI_KEY_F5) {
				//F5 刷新窗口，清屏
				window_clean_screen();

			} else if(key == GUI_KEY_UP) {
				//ctrl + UP
				if (modi&GUI_ACTIVE_CTRL) {
					window_scroll(SCROLL_UP, 1, 0);
				}
				//UP
				cursor_move(cursor.x, cursor.y-1);
			} else if(key == GUI_KEY_DOWN){
				//ctrl + DOWN
				if (modi&GUI_ACTIVE_CTRL) {
					window_scroll(SCROLL_DOWN, 1, 0);
				}
				//DOWN
				cursor_move(cursor.x, cursor.y+1);
			} else if(key == GUI_KEY_LEFT){
				
				//LEFT
				cursor_move(cursor.x-1, cursor.y);
			} else if(key == GUI_KEY_RIGHT){
				
				//RIGHT
				cursor_move(cursor.x+1, cursor.y);

			/*
			ASICLL键码
			*/
			
			} else if (key < 255 || key&FLAG_PAD) {
				//是按键就直接显示出来
				window_put_word(key);
			}

			key = 0;
		}
	}

}

int init_window(int width, int height, int char_w, int char_h)
{
	window.width = width;
	window.height = height;
	
	window.char_width = char_w;
	window.char_height = char_h;
	
	window.rows = window.width/window.char_width;
	window.columns = window.height/window.char_height;

	//背景色为黑色
	window.bcolor = ARGB_POTATO_RED;
	//前景色为白色
	window.fcolor = ARGB(255,255,255,255);
	
	//设定字符缓冲区大小
	window.word_buf_size = window.rows*window.columns*FRAME_PAGES;
	//为字符缓冲分配内存
	window.word_buf = (char *)malloc(window.word_buf_size);
	//如果失败就返回
	if (window.word_buf == NULL) {
		return -1;
	}

	//把分配的buf清空
	memset(window.word_buf, 0, window.word_buf_size);

	//设定指针指向
	window.word_buf_current_start = window.word_buf;

	window.frame_pages = FRAME_PAGES;
	window.char_tab_len = CHAR_TABLE_LEN;

	//初始化光标
	cursor.x = 0;
	cursor.y = 0;

	cursor.lines = 0;

	cursor.width = char_w;
	cursor.height = char_h;
	
	//光标颜色为白色
	cursor.color = ARGB(255,255,255,255);

	if (gui_init() == -1) {
		//失败返回前要把之前分配的buf释放
		free(window.word_buf);
		return -1;
	}
	gui_window_size(window.width, window.height);
	gui_window_position(300, 24);
	if (gui_window_create(BOSH_NAME) == -1) {
		//失败返回前要把之前分配的buf释放
		free(window.word_buf);
		return -1;
	}

	//创建完后进行基础绘制

	//清空背景
	gui_color(window.bcolor);
	gui_rect(0, 0, window.width, window.height);

	cursor_draw();

	//全部显示出来
	gui_draw(0,0,window.width, window.height);

	return 0;
}

void word_get(char *word, int x, int y)
{

	*word = window.word_buf_current_start[y*window.rows+x];
}

void word_set(char word, int x, int y)
{
	//保存字符
	window.word_buf_current_start[y*window.rows+x] = word;
}

/*
向上或者向下滚动屏幕
dir 是方向
lines 是几行
accord 是不是自动
*/
void window_scroll(int dir, int lines, int accord)
{
	if (dir == SCROLL_UP) {
		//判断是否能滚屏
		if (!can_scroll_up()) {
			//如果不能向下滚屏就返回
			return;
		}

		//清空背景
		gui_color(window.bcolor);
		gui_rect(0, 0, window.width, window.height);

		//修改显存起始位置
		window.word_buf_current_start -= window.rows*lines;

		//把字符全部加载到窗口
		window_load_word();

		//加载完后显示
		gui_draw(0, 0, window.width, window.height);
		
		
		
		cursor.y++;
		if (cursor.y > window.columns - 1) {
			cursor.y = window.columns - 1;
		}
		//修改光标位置
		cursor_draw();
	} else if (dir == SCROLL_DOWN) {
		
		//判断是否能滚屏
		if (!can_scroll_down()) {
			//如果不能向下滚屏就返回
			return;
		}
		
		//清空背景
		gui_color(window.bcolor);
		gui_rect(0, 0, window.width, window.height);

		//修改显存起始位置
		window.word_buf_current_start += window.rows*lines;

		//把字符全部加载到窗口
		window_load_word();

		if (!accord) {
			cursor.y--;
			
			if (cursor.y < 0) {
				cursor.y = 0;
			}

		}

		//加载完后显示
		gui_draw(0, 0, window.width, window.height);

		//修改光标位置
		cursor_draw();

	}

}

void window_load_word()
{
	int bx, by, x, y;
	char word;

	//设置字符颜色
	gui_color(window.fcolor);
	char s[2];
	s[1] = 0;

	for (by = 0; by < window.columns; by++) {
		for (bx = 0; bx < window.rows; bx++) {
			word_get(&word, bx, by);
			s[0] = word;

			x = bx * window.char_width;
			y = by * window.char_height;
			
			if (word == '\n') {
				
			} else {
				gui_text(x, y, s);
			}
		}
	}

	
}
/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
void window_clean_screen()
{
	//清空背景
	gui_color(window.bcolor);
	gui_rect(0, 0, window.width, window.height);
	gui_draw(0, 0, window.width, window.height);
	
	//清空字符缓冲区
	memset(window.word_buf, 0, window.word_buf_size);

	//修改字符缓冲区指针
	window.word_buf_current_start = window.word_buf;

	//重置光标
	cursor.x = 0;
	cursor.y = 0;

	//绘制光标
	cursor_draw();

}

/*
把光标从屏幕上去除
*/
void cursor_clear()
{
	//在光标位置绘制背景
	int x = cursor.x * window.char_width;
	int y = cursor.y * window.char_height;
	//绘制背景
	gui_color(window.bcolor);
	gui_rect(x, y, window.char_width, window.char_height);
	
	gui_draw(x, y, x + cursor.width, y + cursor.height);
}

void cursor_draw()
{
	//先在光标位置绘制背景，再绘制光标
	int x = cursor.x * window.char_width;
	int y = cursor.y * window.char_height;
	//绘制背景
	gui_color(window.bcolor);
	gui_rect(x, y, window.char_width, window.char_height);
	//绘制光标
	gui_color(cursor.color);
	gui_rect(x, y, cursor.width, cursor.height);

	gui_draw(x, y, x + cursor.width, y + cursor.height);
}

/*
光标移动一个位置
x是x方向上的移动
y是y方向上的移动
*/
void cursor_move(int x, int y)
{
	//先把光标消除
	cursor_clear();

	//把原来位置上的字符显示出来
	char word;
	word_get(&word, cursor.x, cursor.y);
	
	//文字颜色
	gui_color(window.fcolor);
	window_put_visual(word, cursor.x*window.char_width, cursor.y*window.char_height);

	//移动光标
	cursor.x = x;
	cursor.y = y;
	//修复位置
	cursor_position_check();

	//显示光标
	cursor_draw();
	//把光标所在的字符显示出来
	word_get(&word, cursor.x, cursor.y);
	
	//背景的颜色
	gui_color(window.bcolor);
	window_put_visual(word, cursor.x*window.char_width, cursor.y*window.char_height);
	
}
void window_put_buf(char *buf, int len)
{
	while (*buf && len-- > 0) {
		window_put_word(*buf++);
	}
}
/*
显示一个可见字符
*/
void window_put_visual(char word, int x, int y)
{
	if (0x20 <= word && word <= 0x7e) {
		char s[2];
		s[0] = word;
		s[1] = 0;
		
		gui_text(x, y, s);
		gui_draw(x, y, x + window.char_width, y + window.char_height);
		
	}

}

void window_put_word(char word)
{
	//先把光标去除
	cursor_clear();

	//设置显示的文字的颜色
	gui_color(window.fcolor);
	
	//对字符进行设定，如果是可显示字符就显示
	switch (word) {
		case '\n':
			//光标的位置设定一个字符
			word_set(word, cursor.x, cursor.y);

			//改变位置

			//能否回车
			if (can_scroll_down()) {
				cursor_move(0, cursor.y+1);
			}
			break;
		case '\b':
			//改变位置
			cursor.x--;

			//改变位置后需要做检测，因为要写入字符
			cursor_position_check();

			//cursor_move(cursor.x-1, cursor.y);

			word_set(0, cursor.x, cursor.y);

			cursor_draw();
			break;
		case '\t':
			
			break;
		default :
			word_set(word, cursor.x, cursor.y);

			//可显示字符
			//转换成字符串显示
			/*s[0] = word;
			s[1] = 0;
			gui_text(x, y, s);
			
			//把字符绘制到窗口
			gui_draw(x, y, x + window.char_width, y + window.char_height);*/
			cursor_move(cursor.x+1, cursor.y);

			//改变位置
			//cursor.x++;
			break;
	}

	//对位置进行检测
	//cursor_position_check();
	//绘制出光标
	//cursor_draw();
}

void cursor_position_check()
{
	//如果光标向左移动超出，就切换到上一行最后
	if (cursor.x < 0) {
		
		if (cursor.y > 0) {
			//向左移动，如果发现y > 0，那么就可以移动行尾
			cursor.x = window.rows - 1;
		} else {
			//如果向左移动，发现y <= 0，那么就只能在行首
			cursor.x = 0;
		}
		//移动到上一行
		cursor.y--;
	}

	//如果光标向右移动超出，就切换到下一行
	if (cursor.x > window.rows - 1) {
		if (cursor.y < window.columns - 1) {
			//如果y 没有到达最后一行，就移动到行首
			cursor.x = 0;
		} else {
			//如果y到达最后一行，就移动到行尾
			cursor.x = window.rows - 1;
		}
		//移动到下一行
		cursor.y++;
	}

	//如果光标向上移动超出，就修复
	if (cursor.y < 0) {
		//做修复处理
		cursor.y = 0;

	}

	//如果光标向下移动超出，就向下滚动屏幕
	if (cursor.y > window.columns -1) {
		
		//暂时做修复处理
		cursor.y = window.columns-1;

		window_scroll(SCROLL_DOWN, 1, 1);

	}
}

int gui_printf(const char *fmt, ...)
{
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	window_put_buf(buf, i);
	return i;
}

int can_scroll_up()
{
	if (window.word_buf_current_start > window.word_buf) {
		return 1;
	}
	return 0;
}

int can_scroll_down()
{
	if (window.word_buf_current_start < window.word_buf + window.word_buf_size - window.rows*window.columns) {
		return 1;
	}
	return 0;
}
