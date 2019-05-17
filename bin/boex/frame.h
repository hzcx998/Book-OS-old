#ifndef _FRAME_H
#define _FRAME_H

#include <stdint.h>

#define FRAME_STATUS_IDLE  0
#define FRAME_STATUS_TOUCH  1
#define FRAME_STATUS_CLICK  2

#define FRAME_TEXT_LEN  16


#define FRAME_COLOR_IDLE  ARGB(255,220,220,220)
#define FRAME_COLOR_TOUCH  ARGB(255,240,240,240)
#define FRAME_COLOR_CLICK  ARGB(255,120,120,120)

#define FRAME_DEFAULT_WIDTH  24
#define FRAME_DEFAULT_HEIGHT  24

/*
frame
*/
typedef struct frame_s
{
    int x, y;
    int width, height;
    int status;
    uint32_t color_idle, color_touch, color_click;  //静止色，触碰色，点击色
    uint32_t bcolor, fcolor;    //背景色和前景色
    char text[FRAME_TEXT_LEN];
    void (*mouse_down)(void);    //单击按钮的操作
    void (*mouse_up)(void);    //弹起按钮的操作
}frame_t;

void frame_init(frame_t *frame, int x, int y, int width, int height);
void frame_color_status(frame_t *frame, uint32_t idle, uint32_t touch, uint32_t click);
void frame_color_face(frame_t *frame, uint32_t bcolor, uint32_t fcolor);
void frame_draw(frame_t *frame);
void frame_set_text(frame_t *frame, char *text);
void frame_bind_even(frame_t *frame, void (*mouse_down)(), void (*mouse_up)());
void frame_update(frame_t *frame, int mousex, int mousey, int status);
void frame_change_status(frame_t *frame, int status);

#endif  //_FRAME_H
