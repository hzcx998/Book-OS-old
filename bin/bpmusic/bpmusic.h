#include <string.h>	//strlen()

#define WINDOW_NAME "Beep Music"

#define WIN_WIDTH 256
#define WIN_HEIGHT 112
#define BACK_COLOR ARGB(255,254,254,254)
#define MIDDLE_COLOR ARGB(255,190,190,190)
#define FRONT_COLOR ARGB(255,0,0,0)

/*
用一个窗口结构体保存所有窗体信息
*/
struct window_s{
	int width, height;	//窗口的宽高
	uint32_t bcolor;	//背景颜色
	uint32_t fcolor;	//前景颜色
}window;

int init_window(int width, int height);
int api_window(char *winbuf,int width,int height,int focus,char *title);
void putstr(int x, int y, uint32_t color, char *s);
void wait(int i, timer_t *timer, char *keyflag);
void force_exit_check();