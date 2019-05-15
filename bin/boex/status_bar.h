#ifndef _STATUS_BAR_H
#define _STATUS_BAR_H

#include <stdint.h>

#define STATUS_BAR_HEIGHT_DEFAULT 24

#define STATUS_BAR_COLOR_IDLE  ARGB(255,220,220,220)
#define STATUS_BAR_COLOR_TOUCH  ARGB(255,240,240,240)
#define STATUS_BAR_COLOR_CLICK  ARGB(255,192,192,192)

#define STATUS_BAR_TEXT_LEN 128

typedef struct status_bar_s
{
    int x, y;
    int width, height;
    uint32_t color_idle, color_touch, color_click;  //静止色，触碰色，点击色
    uint32_t bcolor, fcolor;    //背景色和前景色
    char text[STATUS_BAR_TEXT_LEN];
} status_bar_t;

void status_bar_init(status_bar_t *status_bar, int x, int y, int width, int height);
void status_bar_color_status(status_bar_t *status_bar, uint32_t idle, uint32_t touch, uint32_t click);
void status_bar_color_face(status_bar_t *status_bar, uint32_t bcolor, uint32_t fcolor);

void status_bar_set_text(status_bar_t *status_bar, char *text);
void status_bar_draw(status_bar_t *status_bar);
void status_bar_message(status_bar_t *status_bar, const char *fmt, ...);

#endif  //_STATUS_BAR_H
