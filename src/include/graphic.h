#ifndef _LIB_GRAPHIC_H_
#define _LIB_GRAPHIC_H_

#include <sys/gui.h>
#include <sys/dev.h>

#define GUI_KEY_ESC 		ESC
#define GUI_KEY_TAB 		TAB
#define GUI_KEY_ENTER 		ENTER
#define GUI_KEY_BACKSPACE 	BACKSPACE
#define GUI_KEY_ESC 		ESC
#define GUI_KEY_SHIFT_L		SHIFT_L
#define GUI_KEY_SHIFT_R		SHIFT_R
#define GUI_KEY_CTRL_L		CTRL_L	
#define GUI_KEY_CTRL_R		CTRL_R	
#define GUI_KEY_ALT_L		ALT_L
#define GUI_KEY_ALT_R		ALT_R
#define GUI_KEY_F1			F1	
#define GUI_KEY_F2			F2
#define GUI_KEY_F3			F3
#define GUI_KEY_F4			F4	
#define GUI_KEY_F5			F5	
#define GUI_KEY_F6			F6	
#define GUI_KEY_F7			F7
#define GUI_KEY_F8			F8	
#define GUI_KEY_F9			F9	
#define GUI_KEY_F10			F10	
#define GUI_KEY_F11			F11
#define GUI_KEY_F12			F12

#define GUI_KEY_PRINTSCREEN	PRINTSCREEN	
#define GUI_KEY_PAUSEBREAK	PAUSEBREAK	
#define GUI_KEY_INSERT		INSERT	
#define GUI_KEY_DELETE		DELETE	
#define GUI_KEY_HOME		HOME	
#define GUI_KEY_END			END	
#define GUI_KEY_PAGEUP		PAGEUP	
#define GUI_KEY_PAGEDOWN	PAGEDOWN	
#define GUI_KEY_UP			UP	
#define GUI_KEY_DOWN		DOWN	
#define GUI_KEY_LEFT		LEFT	
#define GUI_KEY_RIGHT		RIGHT	


#define GUI_ACTIVE_SHIFT 	0x01
#define GUI_ACTIVE_CTRL 	0x02
#define GUI_ACTIVE_ALT	 	0x04


/*系统调用*/
int init_graphic(void );
void graphic_exit();
void guipoint(struct guiatom *atom);
void guirefresh(struct guiatom *atom);
void guiline(struct guiatom *atom);
void guirect(struct guiatom *atom);
void guitext(struct guiatom *atom);
void guibuffer(struct guiatom *atom);
void guichar(struct guiatom *atom);
int guibmp(struct guiatom *atom);
int guijpeg(struct guiatom *atom);

/*封装的间接图形库*/
void draw_point(int x, int y, uint32 color);
void draw_string(int x, int y, char *str, uint32 color);
void draw_rect(int x, int y, int width, int height, uint32 color);
void draw_line(int x0, int y0, int x1, int y1, uint32 color);
void draw_buffer(int x, int y, int width, int height, uint8 *data_buf);
void draw_char(int x, int y, char ch, uint32 color);
int draw_bmp(const char *filename, int x, int y);
int draw_jpeg(const char *filename, int x, int y);

void refresh(int x0, int y0, int x1, int y1);

void get_screen(int *x, int *y);

/*标准图形库*/
/*
窗口
*/
int gui_init();
void gui_window_size(int width, int height);
void gui_window_position(int x, int y);
int gui_window_create(const char *title);
int gui_window_close();

/*
图形图像
*/
void gui_color(unsigned int color);
void gui_point(int x, int y);
void gui_line(int x1, int y1, int x2, int y2);
void gui_rect(int x, int y, int width, int height);
void gui_text(int x, int y, char *text);

void gui_buffer(unsigned char *buffer);
void gui_frame(int x, int y, int width, int height);
void gui_draw(int x1, int y1, int x2, int y2);

int gui_mouse_move(int *x, int *y);
int gui_mouse_click(int *key, int *status, int *x, int *y);
int gui_keyboard(int *key);
int gui_get_modifiers();

#endif  /*_GUI_GRAPHIC_H_*/