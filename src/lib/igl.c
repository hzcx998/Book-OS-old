#include <graphic.h>
/*Indirect graphics library*/

void draw_point(int x, int y, uint32 color)
{
	struct guiatom _point;
	_point.x = x;
	_point.y = y;
	_point.color = color;
	guipoint(&_point);
}

void draw_char(int x, int y, char word, uint32 color)
{
	struct guiatom _char;
	_char.x = x;
	_char.y = y;
	_char.word = word;
	_char.color = color;
	guichar(&_char);
}

void draw_string(int x, int y, char *str, uint32 color)
{
	struct guiatom _text;
	_text.x = x;
	_text.y = y;
	_text.string = str;
	_text.color = color;
	guitext(&_text);
}

void draw_rect(int x, int y, int width, int height, uint32 color)
{
	struct guiatom _rect;
	_rect.x = x;
	_rect.y = y;
	_rect.width = width;
	_rect.height = height;
	_rect.color = color;
	guirect(&_rect);
}

void draw_line(int x0, int y0, int x1, int y1, uint32 color)
{
	struct guiatom _line;
	_line.x0 = x0;
	_line.y0 = y0;
	_line.x1 = x1;
	_line.y1 = y1;
	_line.color = color;
	guiline(&_line);
}
void draw_buffer(int x, int y, int width, int height, uint8 *data_buf)
{
	struct guiatom _buffer;
	_buffer.x = x;
	_buffer.y = y;
	_buffer.width = width;
	_buffer.height = height;
	_buffer.buffer = data_buf;
	guibuffer(&_buffer);
}

void refresh(int x0, int y0, int x1, int y1)
{
	struct guiatom _aera;
	_aera.x0 = x0;
	_aera.y0 = y0;
	_aera.x1 = x1;
	_aera.y1 = y1;
	guirefresh(&_aera);
}

int draw_bmp(const char *filename, int x, int y)
{
	struct guiatom _bmp;
	_bmp.string = (char *)filename;
	_bmp.x = x;
	_bmp.y = y;
	
	return guibmp(&_bmp);
}

int draw_jpeg(const char *filename, int x, int y)
{
	struct guiatom _jpeg;
	_jpeg.string = (char *)filename;
	_jpeg.x = x;
	_jpeg.y = y;
	return guijpeg(&_jpeg);
}
