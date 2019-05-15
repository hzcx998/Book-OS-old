#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "path_bar.h"

void path_bar_init(path_bar_t *path_bar, int x, int y, int width, int height)
{
	path_bar->x = x;
	path_bar->y = y;
	path_bar->width = width;
	path_bar->height = height;
	
}

void path_bar_set_path(path_bar_t *path_bar, char *path)
{
	memset(path_bar->path, 0, PATH_BAR_NAME_LEN);
	strcpy(path_bar->path, path);
}

void path_bar_color_face(path_bar_t *path_bar, uint32_t bcolor, uint32_t mcolor, uint32_t fcolor)
{
	path_bar->bcolor = bcolor;
	path_bar->mcolor = mcolor;
	path_bar->fcolor = fcolor;
}

void path_bar_draw(path_bar_t *path_bar)
{
	//绘制到屏幕
	gui_color(path_bar->bcolor);
	gui_rect(path_bar->x, path_bar->y, path_bar->width, path_bar->height);
	//绘制四周
	gui_color(path_bar->mcolor);
	gui_rect(path_bar->x, path_bar->y, path_bar->width, 1);
	gui_rect(path_bar->x, path_bar->y, 1, path_bar->height);
	gui_rect(path_bar->x + path_bar->width - 1, path_bar->y, 1, path_bar->height);
	gui_rect(path_bar->x, path_bar->y + path_bar->height - 1, path_bar->width, 1);

	gui_color(path_bar->fcolor);
	gui_text(path_bar->x+1, path_bar->y+4, path_bar->path);

	gui_draw(path_bar->x, path_bar->y,path_bar->x + path_bar->width, path_bar->y + path_bar->height);
}
