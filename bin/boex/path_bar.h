#ifndef _PATH_BAR_H
#define _PATH_BAR_H

#include <stdint.h>

#define PATH_BAR_NAME_LEN 256

#define PATH_BAR_HEIGHT 24


typedef struct path_bar_s
{
    int x, y;
    int width, height;
    uint32_t bcolor, mcolor, fcolor;
    char path[PATH_BAR_NAME_LEN];
}path_bar_t;

void init_path_bar();
void path_bar_draw();

void path_bar_init(path_bar_t *path_bar, int x, int y, int width, int height);
void path_bar_color_face(path_bar_t *path_bar, uint32_t bcolor, uint32_t mcolor, uint32_t fcolor);
void path_bar_draw(path_bar_t *path_bar);
void path_bar_set_path(path_bar_t *path_bar, char *path);

#endif  //_PATH_BAR_H
