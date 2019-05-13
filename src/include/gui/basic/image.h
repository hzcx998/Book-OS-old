#ifndef _GUI_BASIC_IMAGE_H
#define _GUI_BASIC_IMAGE_H

#include <stdint.h>
void display_start_logo();

int jpeg_draw_picture(uint8 *buffer, const char *filename, int x, int y);


void vram_draw_word_bits(int x, int y , uint8 *ascii, uint32 color);
void vram_draw_word(int x, int y, char ch, uint32 color);
void vram_draw_string(int x, int y, char *s, uint32 color);

#endif  //_GUI_BASIC_IMAGE_H

