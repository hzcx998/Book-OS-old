#ifndef _BOEX_H
#define _BOEX_H

/*
book OS explorer BOEX 核心
*/


#define BOEX_NAME "boex"

#define PATH_NAME_LEN 256

#define PATH_BAR_WIDTH 400
#define PATH_BAR_HEIGHT 20


typedef struct path_bar_s
{
    int x, y;
    int width, height;
    uint32_t bcolor, fcolor;
    char path[PATH_NAME_LEN];
}path_bar_t;

void init_path_bar();
void path_bar_refresh();

#define WIN_WIDTH 480
#define WIN_HEIGHT 360

/*
用一个窗口结构体保存所有窗体信息
*/
struct window_s
{
	int width, height;	//窗口的宽高
	uint32_t bcolor;	//背景颜色
	uint32_t fcolor;	//前景颜色
}window;

int init_window(int width, int height);

void init_path_button();

#endif  //_BOEX_H
