#ifndef _DEVICE_VGA_H
#define _DEVICE_VGA_H

#include <stdint.h>

#define VGA_VRAM 0x000b8000

/* VGA */
#define	CRTC_ADDR_REG	0x3D4	/* CRT Controller Registers - Addr Register */
#define	CRTC_DATA_REG	0x3D5	/* CRT Controller Registers - Data Register */
#define	START_ADDR_H	0xC	/* reg index of video mem start addr (MSB) */
#define	START_ADDR_L	0xD	/* reg index of video mem start addr (LSB) */
#define	CURSOR_H	0xE	/* reg index of cursor position (MSB) */
#define	CURSOR_L	0xF	/* reg index of cursor position (LSB) */
#define	V_MEM_BASE	VGA_VRAM	/* base of color video memory */
#define	V_MEM_SIZE	0x8000	/* 32K: B8000H -> BFFFFH */

#define VIDEO_WIDTH		80
#define VIDEO_HEIGHT		25

#define VIDEO_SIZE		(VIDEO_WIDTH * VIDEO_HEIGHT)
/*
color set

MAKE_COLOR(BLUE, RED)
MAKE_COLOR(BLACK, RED) | BRIGHT
MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH

*/
#define BLACK   0x0     /* 0000 */
#define WHITE   0x7     /* 0111 */
#define RED     0x4     /* 0100 */
#define GREEN   0x2     /* 0010 */
#define BLUE    0x1     /* 0001 */
#define FLASH   0x80    /* 1000 0000 */
#define BRIGHT  0x08    /* 0000 1000 */
#define	MAKE_COLOR(x,y)	((x<<4) | y) /* MAKE_COLOR(Background,Foreground) */

//#define COLOR_DEFAULT	(MAKE_COLOR(BLACK, WHITE))
#define COLOR_DEFAULT	(MAKE_COLOR(BLACK, WHITE))
//#define COLOR_GRAY		(MAKE_COLOR(BLACK, BLACK))

uint16 get_cursor();
void set_cursor(uint16 cursor_pos);
void set_video_start_addr(uint16 addr);

#endif

