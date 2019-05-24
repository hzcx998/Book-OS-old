#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "invader.h"

void setdec8(char *s, int i);
void putstr(int x, int y, uint32_t color, char *s);
void wait(int i, timer_t *timer, char *keyflag);

static char charset[16 * 8] = {

	/* invader(0) */
	0x00, 0x00, 0x00, 0x43, 0x5f, 0x5f, 0x5f, 0x7f,
	0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x20, 0x3f, 0x00,

	/* invader(1) */
	0x00, 0x0f, 0x7f, 0xff, 0xcf, 0xcf, 0xcf, 0xff,
	0xff, 0xe0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0x00,

	/* invader(2) */
	0x00, 0xf0, 0xfe, 0xff, 0xf3, 0xf3, 0xf3, 0xff,
	0xff, 0x07, 0xff, 0xff, 0x03, 0x03, 0x03, 0x00,

	/* invader(3) */
	0x00, 0x00, 0x00, 0xc2, 0xfa, 0xfa, 0xfa, 0xfe,
	0xf8, 0xf8, 0xf8, 0xf8, 0x00, 0x04, 0xfc, 0x00,

	/* fighter(0) */
	0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x43, 0x47, 0x4f, 0x5f, 0x7f, 0x7f, 0x00,

	/* fighter(1) */
	0x18, 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff,
	0xff, 0xff, 0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0x00,

	/* fighter(2) */
	0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0xc2, 0xe2, 0xf2, 0xfa, 0xfe, 0xfe, 0x00,

	/* laser */
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00
};
/* invader:"abcd", fighter:"efg", laser:"h" */

int main(int argc, char *argv0[])
{
	//printf("start book os explorer!\n");
	int i, j, fx, laserwait, lx = 0, ly;
	int ix, iy, movewait0, movewait, idir;
	int invline, score, high, point;
	char invstr[32 * 6], s[12], keyflag[4], *p;
	static char invstr0[32] = " abcd abcd abcd abcd abcd ";
	timer_t *timer;

	uint32_t alien_color, fighter_color, laser_color;

	if (init_window(WIN_WIDTH, WIN_HEIGHT) == -1) {
		printf("invader init window failed!\n");
		return -1;
	}
	//分配定时器
	timer = talloc();

	//初始化颜色
	alien_color = RGB(40,200,90);	//亮绿
	fighter_color = RGB(0,255,255);	//天蓝
	laser_color = RGB(255,255,0);	//黄色

	//settimer(timer, 100);
/*	
	int t = 0;

	putstr(1, 3, ARGB_RED, "abcd");
	putstr(1, 5, ARGB_GREEN, "efg");
	putstr(1, 8, ARGB_BLUE, "h");
	
	putstr(2, 10, ARGB_BLUE, "HIGH");
	
	wait(100, timer, keyflag);
	printf("wait 0");

	wait(100, timer, keyflag);
	printf("wait 1");
	
	wait(100, timer, keyflag);
	printf("wait 2");

	wait(0, timer, keyflag);
	
	printf("wait 3");
*/
	high = 0;
	putstr(22, 0, window.fcolor, "HIGH:00000000");

restart:
	score = 0;
	point = 1;
	putstr(4, 0, window.fcolor, "SCORE:00000000");
	movewait0 = 20;
	fx = 18;
	putstr(fx, 13, fighter_color, "efg");
	wait(100, timer, keyflag);

next_group:
	wait(100, timer, keyflag);
	ix = 7;
	iy = 1;
	invline = 6;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 27; j++) {
			invstr[i * 32 + j] = invstr0[j];
		}
		putstr(ix, iy + i, alien_color, invstr + i * 32);
	}
	keyflag[0] = 0;
	keyflag[1] = 0;
	keyflag[2] = 0;

	ly = 0; /* 不显示 */
	laserwait = 0;
	movewait = movewait0;
	idir = +1;
	wait(100, timer, keyflag);

	for (;;) {
		if (laserwait != 0) {
			laserwait--;
			keyflag[2 /* space */] = 0;
		}

		wait(4, timer, keyflag);

		//按键控制移动
		if (keyflag[0 /* left */]  != 0 && fx > 0) {
			fx--;
			putstr(fx, 13, fighter_color, "efg ");
			keyflag[0 /* left */]  = 0;
		} else if (keyflag[1 /* right */] != 0 && fx < 37) {
			putstr(fx, 13, fighter_color, " efg");
			fx++;
			keyflag[1 /* right */] = 0;
		}
		if (keyflag[2 /* space */] != 0 && laserwait == 0) {
			laserwait = 15;
			lx = fx + 1;
			ly = 13;
		}

		/* 外星人的移动 */
		if (movewait != 0) {
			movewait--;
		} else {
			movewait = movewait0;
			if (ix + idir > 14 || ix + idir < 0) {
				if (iy + invline == 13) {
					break; /* GAME OVER */
				}
				idir = - idir;
				putstr(ix + 1, iy, alien_color, "                         ");
				iy++;
			} else {
				ix += idir;
			}
			for (i = 0; i < invline; i++) {
				putstr(ix, iy + i, alien_color, invstr + i * 32);
			}
			
		}

		/* 炮弹处理 */
		if (ly > 0) {
			if (ly < 13) {
				if (ix < lx && lx < ix + 25 && iy <= ly && ly < iy + invline) {
					putstr(ix, ly, alien_color, invstr + (ly - iy) * 32);
				} else {
					putstr(lx, ly, 0, " ");
				}
			}
			ly--;
			if (ly > 0) {
				putstr(lx, ly, laser_color, "h");
			} else {
				point -= 10;
				if (point <= 0) {
					point = 1;
				}
			}
			if (ix < lx && lx < ix + 25 && iy <= ly && ly < iy + invline) {
				p = invstr + (ly - iy) * 32 + (lx - ix);
				if (*p != ' ') {
					/* hit ! */
					score += point;
					point++;
					//显示分数
					sprintf(s, "%08d", score);
					putstr(10, 0, window.fcolor, s);
					if (high < score) {
						//显示最高分
						high = score;
						putstr(27, 0, window.fcolor, s);
					}
					for (p--; *p != ' '; p--) { }
					for (i = 1; i < 5; i++) {
						p[i] = ' ';
					}
					putstr(ix, ly, alien_color, invstr + (ly - iy) * 32);
					for (; invline > 0; invline--) {
						for (p = invstr + (invline - 1) * 32; *p != 0; p++) {
							if (*p != ' ') {
								goto alive;
							}
						}
					}
					/* 全部消灭 */
					movewait0 -= movewait0 / 3;
					goto next_group;
	alive:
					ly = 0;
				}
			}
		}
	}

	/* GAME OVER */
	putstr(15, 6, ARGB_RED, "GAME OVER");
	wait(0, timer, keyflag);
	for (i = 1; i < 14; i++) {
		putstr(0, i, window.fcolor, "                                        ");
	}
	goto restart;
	return 0;
}

int init_window(int width, int height)
{
	window.width = width;
	window.height = height;
	
	//背景色为黑色
	window.bcolor = BACK_COLOR;
	//前景色为白色
	window.fcolor = FRONT_COLOR;
	
	if (gui_init() == -1) {
		return -1;
	}
	gui_window_size(window.width, window.height);
	gui_window_position(100, 24);
	if (gui_window_create(WINDOW_NAME) == -1) {
		return -1;
	}

	//清空背景
	gui_color(window.bcolor);
	gui_rect(0, 0, window.width, window.height);

	//全部显示出来
	gui_draw(0,0,window.width, window.height);

	return 0;
}

void wait(int i, timer_t *timer, char *keyflag)
{
	int j;
	if (i > 0) {
		/* 等待一段时间 */
		settimer(timer, i);
		i = 128;
	} else {
		i = '\n'; /* Enter */
	}
	for (;;) {
		//捕捉键盘
		if (!gui_keyboard(&j) ) {
			//如果i是'\n'，并且按下了回车，就会直接返回
			if (i == j) {
				break;
			}
			if(j == GUI_KEY_LEFT){
				//LEFT
				keyflag[0 /* left */]  = 1;
			}
			if(j == GUI_KEY_RIGHT){
				keyflag[1 /* right */] = 1;
			}
			if (j == '4') {
				keyflag[0 /* left */]  = 1;
			}
			if (j == '6') {
				keyflag[1 /* right */] = 1;
			}
			if (j == ' ') {
				keyflag[2 /* space */] = 1;
			}
		}
		//清除按键

		j = 0;
		//时钟发生，退出循环
		if (toccur(timer)) {
			break;
		}
		
	}
}

void putstr(int x, int y, uint32_t color, char *s)
{
	int c, x0, y0, i, len;
	char *p, t[2];
	//传入的xy需要转换成屏幕坐标
	x = x * 8;
	y = y * 16;
	//保存转换后的坐标
	x0 = x;
	y0 = y;
	//获取字符串长度
	len = strlen(s);
	//填充背景色
	gui_color(window.bcolor);
	gui_rect(x, y, len * 8, 16);
	
	//设置字的颜色
	gui_color(color);

	t[1] = 0;
	//循环获取字符，每次获取一个字符
	for (;;) {
		c = *s;
		if (c == 0) {
			break;
		}
		if (c != ' ') {
			//把图形处理成文字的方式
			if ('a' <= c && c <= 'h') {
				//获取字模数据
				p = charset + 16 * (c - 'a');

				//获取单行
				for (i = 0; i < 16; i++) {
					if ((p[i] & 0x80) != 0) { gui_point(x + 0, y); }
					if ((p[i] & 0x40) != 0) { gui_point(x + 1, y); }
					if ((p[i] & 0x20) != 0) { gui_point(x + 2, y); }
					if ((p[i] & 0x10) != 0) { gui_point(x + 3, y); }
					if ((p[i] & 0x08) != 0) { gui_point(x + 4, y); }
					if ((p[i] & 0x04) != 0) { gui_point(x + 5, y); }
					if ((p[i] & 0x02) != 0) { gui_point(x + 6, y); }
					if ((p[i] & 0x01) != 0) { gui_point(x + 7, y); }
					
					//改变y，显示到下一行
					y++;
				}
			} else {	//其它字符
				t[0] = *s;
				//绘制其它字符
				gui_text(x, y0, t);
				
			}
		}
		//指向下一个字符
		s++;
		//改变x，用于显示下一个字符
		x += 8;
		//恢复y
		y = y0;
	}
	//刷新
	gui_draw(x0, y0, x0 + len*8, y0 + 16);
}
/*把数值转换成字符*/
void setdec8(char *s, int i)
{
	int j;
	for (j = 7; j >= 0; j--) {
		s[j] = '0' + i % 10;
		i /= 10;
	}
	s[8] = 0;
}
