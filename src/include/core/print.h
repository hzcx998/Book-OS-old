#ifndef _PRINT_H_
#define _PRINT_H_

#include <types.h>
#include <stdint.h>


void init_print();

/*显示字符的函数指针*/
//void (*display_char_func)(char ch);
//void (*sys_clean_screen)();

int buffer_of_print(char* buf, int len);
int printk(const char *fmt, ...);

int sys_write_str(char *str);
void sys_writ_char(char ch);


#endif //_PRINT_H_

