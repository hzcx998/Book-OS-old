#ifndef _BUTTON_H
#define _BUTTON_H

#include <stdint.h>

#define BUTTON_STATUS_IDLE  0
#define BUTTON_STATUS_TOUCH  1
#define BUTTON_STATUS_CLICK  2

#define BUTTON_TEXT_LEN  16


#define BUTTON_COLOR_IDLE  ARGB(255,220,220,220)
#define BUTTON_COLOR_TOUCH  ARGB(255,240,240,240)
#define BUTTON_COLOR_CLICK  ARGB(255,120,120,120)

#define BUTTON_DEFAULT_WIDTH  24
#define BUTTON_DEFAULT_HEIGHT  24

/*
button
*/
typedef struct button_s
{
    int x, y;
    int width, height;
    int status;
    uint32_t color_idle, color_touch, color_click;  //静止色，触碰色，点击色
    uint32_t bcolor, fcolor;    //背景色和前景色
    char text[BUTTON_TEXT_LEN];
    void (*mouse_down)(void);    //单击按钮的操作
    void (*mouse_up)(void);    //弹起按钮的操作
    
}button_t;

void button_init(button_t *button, int x, int y, int width, int height);
void button_color_status(button_t *button, uint32_t idle, uint32_t touch, uint32_t click);
void button_color_face(button_t *button, uint32_t bcolor, uint32_t fcolor);
void button_draw(button_t *button);
void button_set_text(button_t *button, char *text);
void button_bind_even(button_t *button, void (*mouse_down)(), void (*mouse_up)());
void button_update(button_t *button, int mousex, int mousey, int status);
void button_change_status(button_t *button, int status);

#endif  //_BUTTON_H
