#ifndef _GUI_BASIC_H
#define _GUI_BASIC_H

#include <stdint.h>
#include <stdarg.h>

#include <sys/dev.h>
#include <sys/core.h>

void init_gui_basic();
void thread_gui(void *arg);

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

#define MAX_VIDEO_BUF 12

struct terminal
{
	int x, y;
	uint32 cursor_color, font_color, bc_color;
	int x_size, y_size;
	//struct lock lock;
};

extern struct terminal terminal;

void terminal_clean();

struct video_buffer
{
	uint8 *buffer;
	uint32 flags;
	struct thread *thread;
	int id;
};

extern struct video_buffer *current_vidbuf;
extern struct video_buffer *vidbuf_console;

struct video_buffer *alloc_vidbuf();
void free_vidbuf(struct video_buffer *vidbuf);
struct video_buffer *get_next_vidbuf();



#endif	//_GUI_BASIC_H

