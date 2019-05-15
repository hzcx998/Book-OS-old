#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <types.h>
#include <stdint.h>
#include <core/sync.h>

#define SCREEN_UP -1
#define SCREEN_DOWN 1

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

struct console
{
	uint32_t current_start_addr;
	uint32_t vram_addr;
	uint32_t vram_limit;
	uint32_t cursor;
	uint8 color;
	struct lock lock;
	
};

extern struct console console;

/*显示字符的函数指针*/
/*void (*display_char_func)(char ch);
void (*sys_clean_screen)();
*/

void init_console();

void out_char(char ch);
void scroll_screen(int direction);
void flush();

void console_clean();
void console_gotoxy(int8_t x, int8_t y);

int buffer_of_print(char* buf, int len);
int printk(const char *fmt, ...);

void console_set_color(uint8_t color);
int sys_write_str(char *str);
void sys_writ_char(char ch);


#endif //_DRIVER_CONSOLE_H_

