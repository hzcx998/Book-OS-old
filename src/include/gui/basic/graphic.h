#ifndef _GUI_BASIC_GRAPHIC_H
#define _GUI_BASIC_GRAPHIC_H

#include <stdint.h>
#include <gui/bogui/libary.h>

#define GRAPH_POINT		 1
#define GRAPH_LINE		 2
#define GRAPH_RECT		 3
#define GRAPH_TEXT		 4
#define GRAPH_WORD		 5

struct guiatom
{
	int x, y;
	int width, height;
	int x0, y0, x1, y1;
	uint32 color;
	char *string;
	uint8 *buffer;
	char word;
};

struct point
{
	int x;
	int y;
	
};

struct line
{
	int x0;
	int y0;
	int x1;
	int y1;
	
};

struct rect
{
	int x;
	int y;
	int width;
	int height;
	
};

struct aera
{
	int x0;
	int y0;
	int x1;
	int y1;
	
};


struct text
{
	int x;
	int y;
	char *string;
	
};

int sys_create_window(struct guiatom *atom);
void sys_close_window();

void graph_write_pixel(uint8 *buffer, int x, int y, uint32 color);
void graph_read_pixel(uint8 *buffer, int x, int y, uint32 *color);

void graph_draw_rect(uint8 *buffer, int x, int y, uint32 width,uint32 height, uint32 color);
void graph_draw_string(uint8 *buffer, int x,int y, char *s, uint32 color);
void graph_draw_word(uint8 *buffer, int x,int y,uint32 color, char ch);
void graph_draw_char_bit(uint8 *buffer, int x, int y , uint32 color, uint8 *ascii);
void graph_draw_line(uint8 *buffer,int x0, int y0, int x1, int y1, uint32 color);
void graph_draw_buffer(uint8 *buffer, int x,int y, int width, int height, uint8 *data_buf);

void graph_refresh(uint8 *buffer, int x0, int y0, int x1,int y1);
void switch_video_buffer(struct video_buffer *vidbuf);

int sys_init_graphic(void );
void sys_graphic_exit(void );
void sys_graph_point(struct guiatom *atom);
void sys_graph_line(struct guiatom *atom);
void sys_graph_rect(struct guiatom *atom);
void sys_graph_text(struct guiatom *atom); 
void sys_graph_buffer(struct guiatom *atom);
void sys_graph_refresh(struct guiatom *atom);
void sys_graph_char(struct guiatom *atom);
int sys_graph_bmp(struct guiatom *atom);
int sys_graph_jpeg(struct guiatom *atom);

#endif //_GUI_BASIC_GRAPHIC_H

