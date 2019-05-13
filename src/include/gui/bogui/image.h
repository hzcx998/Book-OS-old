#ifndef _GUI_BOGUI_IMAGE_H
#define _GUI_BOGUI_IMAGE_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <gui/bogui/container.h>

void bogui_container_draw_mouse(bogui_container_t *container);

void bogui_container_draw_button_close(bogui_container_t *container, int x0, int y0, uint32_t bcolor);
void bogui_container_draw_button_minimize(bogui_container_t *container, int x0, int y0, uint32_t bcolor);

void bogui_image_make_winicon(uint8_t *buffer, int width, int height, int bit_widht);
void bogui_container_draw_winicon(bogui_container_t *src_container, bogui_container_t *dst_container,int x0, int y0, uint32_t bcolor);


void bogui_image_write_pixel(uint8_t *buffer, int x, int y, int width, int height, int bit_width, uint32_t color);
void bogui_image_read_pixel(uint8_t *buffer, int x, int y, int width, int height, int bit_width, uint32_t *color);

#endif //_CONFIG_GUI_BOGUI_

#endif //_GUI_BOGUI_IMAGE_H

