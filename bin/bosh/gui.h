#ifndef _BOSH_GUI_H
#define _BOSH_GUI_H

/*
BOSHELL 图形界面相关
*/
/*
用一个窗口结构体保存所有窗体信息
*/
struct window_s
{
	int width, height;	//窗口的宽高
	int char_width, char_height;	//字符的宽高
	int rows, columns;	//行数和列数
	
	uint32_t bcolor;	//背景颜色
	uint32_t fcolor;	//前景颜色

	int word_buf_size;	//字符缓冲的大小
	int frame_pages;	//帧页数

	char *word_buf;		//字符的缓冲区
	char *word_buf_current_start;	//从哪儿作为开始，是一个偏移

	//字符环境设置
	uint32_t char_tab_len;	//table 表示几个空格

}window;

struct cursor_s
{
	int x, y;	//光标的位置
	int width, height;	//光标的宽高
	int color;
	int lines;	//光标所在的行数

}cursor;

#define WIN_WIDTH 480
#define WIN_HEIGHT 360

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

#define FRAME_PAGES 10	//有多少个显存帧

#define CHAR_TABLE_LEN 4	//table表示多少个空格

#define SCROLL_UP 1	//向上滚屏
#define SCROLL_DOWN 2	//向下滚屏

void window_clean_screen();
void word_get(char *word, int x, int y);
void word_set(char word, int x, int y);

void cursor_draw();
void cursor_clear();
void cursor_position_check();
void cursor_move(int x, int y);

void window_scroll(int dir, int lines, int accord);
void window_load_word();

void window_put_word(char word);
void window_put_buf(char *buf, int len);
void window_put_visual(char word, int x, int y);

int gui_printf(const char *fmt, ...);


int init_window(int width, int height, int char_w, int char_h);

int can_scroll_down();
int can_scroll_up();


#endif  //_BOSHELL_GUI_H
