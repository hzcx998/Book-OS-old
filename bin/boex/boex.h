#ifndef _BOEX_H
#define _BOEX_H

/*
book OS explorer BOEX 核心
*/


#define BOEX_NAME "boex"

#define BACK_COLOR ARGB(255,240,240,240)
#define MIDDLE_COLOR ARGB(255,190,190,190)
#define FRONT_COLOR ARGB(255,0,0,0)

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
void init_button();
void init_status_bar();

void mouse_down_even_backward();
void mouse_up_even_backward();

void mouse_down_even_forward();
void mouse_up_even_forward();

void mouse_down_even_setting();
void mouse_up_even_setting();

void mouse_down_even_view();
void mouse_up_even_view();

#endif  //_BOEX_H
