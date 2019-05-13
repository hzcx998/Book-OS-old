#ifndef _GUI_COLOR_H_
#define _GUI_COLOR_H_

#include <sys/config.h>


/*把颜色转换成显存数据*/
#define COLOR_ARGB16(a,r,g,b) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

#define COLOR_ARGB24(a,r,g,b) ((r<<16) | (g<<8) | b)

#ifdef _VIDEO_16_MODE_
	#define COLOR_ARGB(a,r,g,b) COLOR_ARGB16(a,r,g,b)
	#define COLOR_RGB(r,g,b) COLOR_ARGB16(255,r,g,b)
#elif defined _VIDEO_24_MODE_
	#define COLOR_ARGB(a,r,g,b) COLOR_ARGB24(a,r,g,b)
	#define COLOR_RGB(r,g,b) COLOR_ARGB24(255,r,g,b)
#endif

#define COLOR_RED COLOR_RGB(255,0,0)
#define COLOR_GREEN COLOR_RGB(0,255,0)
#define COLOR_BLUE COLOR_RGB(0,0,255)
#define COLOR_WHITE COLOR_RGB(255,255,255)
#define COLOR_BLACK COLOR_RGB(0,0,0)
#define COLOR_GRAY COLOR_RGB(195,195,195)
#define COLOR_LEAD COLOR_RGB(127,127,127)
#define COLOR_YELLOW COLOR_RGB(255,255,127)

/*对原始颜色进行处理*/
#define ARGB(a,r,g,b) ((a<<24) | (r<<16) | (g<<8) | b)
#define RGB(r,g,b) ARGB(255,r,g,b)

#define ARGB_RED ARGB(255, 255,0,0)
#define ARGB_GREEN ARGB(255, 0,255,0)
#define ARGB_BLUE ARGB(255, 0,0,255)
#define ARGB_WHITE ARGB(255, 255,255,255)
#define ARGB_BLACK ARGB(255, 0,0,0)
#define ARGB_GRAY ARGB(255, 195,195,195)
#define ARGB_LEAD ARGB(255, 127,127,127)
#define ARGB_YELLOW ARGB(255, 255,255,127)
#define ARGB_POTATO_RED ARGB(255, 50, 0, 40);


#define ARGB_NONE ARGB(0, 0,0,0)

typedef struct rgba_s
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
}rgba_t;



#endif

