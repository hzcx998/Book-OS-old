/*
File:		gui/bogui/bogui.c
Contains:	bogui init
Auther:		Hu axisicheng
Time:		2019/4/21
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <sys/gui.h>

void bogui_container_draw_mouse(bogui_container_t *container)
{
	
	static char bogui_cursor_image[16][16] = {
		{1,0,0,0,0,0,0,0,0,0,0,0},
		{1,1,0,0,0,0,0,0,0,0,0,0},
		{1,2,1,0,0,0,0,0,0,0,0,0},
		{1,2,2,1,0,0,0,0,0,0,0,0},
		{1,2,2,2,1,0,0,0,0,0,0,0},
		{1,2,2,2,2,1,0,0,0,0,0,0},
		{1,2,2,2,2,2,1,0,0,0,0,0},
		{1,2,2,2,2,2,2,1,0,0,0,0},
		{1,2,2,2,2,2,2,2,1,0,0,0},
		{1,2,2,2,2,2,2,2,2,1,0,0},
		{1,2,2,2,2,1,1,1,1,1,1,0},
		{1,2,2,2,1,0,0,0,0,0,0,0},
		{1,2,2,1,0,0,0,0,0,0,0,0},
		{1,2,1,0,0,0,0,0,0,0,0,0},
		{1,1,0,0,0,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0,0,0,0,0},
	};

	int x, y;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (bogui_cursor_image[y][x] == 0) {
				bogui_container_write_pixel(container, x , y , ARGB_NONE);
			} else if (bogui_cursor_image[y][x] == 1) {
				bogui_container_write_pixel(container, x , y , ARGB_WHITE);
			} else if (bogui_cursor_image[y][x] == 2) {
				bogui_container_write_pixel(container, x , y , ARGB_BLACK);
			}
		}
	}
}

void bogui_container_draw_button_close(bogui_container_t *container, int x0, int y0,uint32_t bcolor)
{
	static char img[16][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,},
		{0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,},
		{0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,},
		{0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,},
		{0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,},
		{0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,},
		{0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
	};
	uint32_t color;
	
	if (container->window->status == BOGUI_WINDOW_ACTIVE) {
		color = BOGUI_WINDOW_COLOR_TITLE_ACTIVE;
	} else {
		color = BOGUI_WINDOW_COLOR_TITLE_INACTIVE;
	}
	int x, y;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (img[y][x] == 0) {
				bogui_container_write_pixel(container, x0 + x , y0 + y , bcolor);
			} else if (img[y][x] == 1) {
				bogui_container_write_pixel(container, x0 + x , y0 + y , color);
			}
		}
	}
}

void bogui_container_draw_button_minimize(bogui_container_t *container, int x0, int y0, uint32_t bcolor)
{
	static char img[16][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,},
		{0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,},
		{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,},
		{0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
	};
	
	uint32_t color;
	
	if (container->window->status == BOGUI_WINDOW_ACTIVE) {
		color = BOGUI_WINDOW_COLOR_TITLE_ACTIVE;
	} else {
		color = BOGUI_WINDOW_COLOR_TITLE_INACTIVE;
	}
	
	int x, y;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (img[y][x] == 0) {
				bogui_container_write_pixel(container, x0 + x , y0 + y , bcolor);
			} else if (img[y][x] == 1) {
				bogui_container_write_pixel(container, x0 + x , y0 + y , color);
			}
		}
	}
}

void bogui_image_make_winicon(uint8_t *buffer, int width, int height, int bit_widht)
{
	static char img[16][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,0,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
	};
	

	
	int x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (img[y][x] == 0) {
				bogui_image_write_pixel(buffer, x, y, width , height, bit_widht, ARGB_NONE);
			} else if (img[y][x] == 1) {
				bogui_image_write_pixel(buffer, x, y, width , height , bit_widht , ARGB_WHITE);
			}
		}
	}
}

/*
把容器里的图标
绘制到另外一个容器上
*/
void bogui_container_draw_winicon(bogui_container_t *src_container, bogui_container_t *dst_container,int x0, int y0, uint32_t bcolor)
{
	//获取窗口图标的buffer
	uint8_t *buffer = src_container->window->winicon.data;
	int width = src_container->window->winicon.width;
	int height = src_container->window->winicon.height;
	
	rgba_t color;

	int x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			//从图像中读取数据
			bogui_image_read_pixel(buffer, x, y, width, height, 4, (uint32_t *)&color);

			//写入到容器
			if (color.alpha == 0) {
				bogui_container_write_pixel(dst_container, x0 + x , y0 + y , bcolor);
			} else {
				bogui_container_write_pixel(dst_container, x0 + x , y0 + y , *((uint32_t *)&color));
			}
		}
	}
}

/*
从缓冲区里读取一个颜色
*/
void bogui_image_read_pixel(uint8_t *buffer, int x, int y, int width, int height, int bit_width, uint32_t *color)
{
	if (x < 0 || x < 0 || x >= width || \
		y >= height) {
		return;
	}
	uint8_t *buf = (uint8_t *)(buffer + (y * width + x)* bit_width);
	
	switch (bit_width) {
		case 1:
			*color = buf[0];
			break;
		case 2:
			*color = buf[0] | (buf[1]<<8);
			break;
		case 3:
			*color = buf[0] | (buf[1]<<8) | (buf[2]<<16);
			break;
		case 4:
			*color = *((uint32_t *)buf);
			break;
	}
}
/*
把一个颜色写入到一个缓冲区
*/
void bogui_image_write_pixel(uint8_t *buffer, int x, int y, int width, int height, int bit_width, uint32_t color)
{
	if (x < 0 || x < 0 || x >= width || \
		y >= height) {
		return;
	}
	uint8_t *buf = (uint8_t *)(buffer + (y * width + x)* bit_width);
	switch (bit_width) {
		case 1:
			*buf = color&0xff;
			break;
		case 2:
			*buf++ = color&0xff;
			*buf = (color>>8)&0xff;
			break;
		case 3:
			*buf++ = color&0xff;
			*buf++ = (color>>8)&0xff;
			*buf = (color>>16)&0xff;
			break;
		case 4:
			*((uint32_t *)buf) = color;
			break;
	}
}

#endif //_CONFIG_GUI_BOGUI_

