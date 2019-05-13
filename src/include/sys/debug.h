#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <types.h>
#include <sys/gui.h>

//断言
#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

void spin(char * func_name);
void panic(const char *fmt, ...);

/*==========================================================================*/
/*
debug with bogui
*/
/*==========================================================================*/
#define DEBUG_CHAR_WIDTH 8
#define DEBUG_CHAR_HEIGHT 16

#define DEBUG_WINDOW_WIDTH 360
#define DEBUG_WINDOW_HEIGHT 480

#define DEBUG_WINDOW_NAME "Book Debuger"

#define DEBUG_FRAME_PAGES 10

/*
对任务栏进行总的管理
*/
typedef struct bogui_debug_s
{
    int cursor_x, cursor_y;
    int cursor_width, cursor_height;
    uint32_t cursor_color;

    int win_width, win_height;
    int char_width, char_height;
    
    int rows, columns;


    int word_buf_size;	//字符缓冲的大小
	int frame_pages;	//帧页数

	char *word_buf;		//字符的缓冲区
	char *word_buf_current_start;	//从哪儿作为开始，是一个偏移

    struct bogui_container_s *container;
    struct bogui_window_s *window;
    
    uint32_t bcolor;    //背景颜色
    uint32_t fcolor;    //字体颜色
    
}bogui_debug_t;


void bogui_debug_init();

void debug_display_char(char ch);

void debug_window_clean();

void debug_cursor_clean();
void debug_cursor_draw();

void debug_window_scroll();
void debug_char_draw(char word);

void debug_word_get(char *word, int x, int y);
void debug_word_set(char word, int x, int y);
void debug_load_word();
void debug_cursor_check();

#endif