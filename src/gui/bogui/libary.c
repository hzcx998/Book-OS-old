/*
Contains:	graphic libary for usr process
Auther:		Jason Hu
Time:		2019/5/6
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <sys/gui.h>
#include <string.h>
#include <graphic.h>

/*
成功返回0
失败返回-1
*/
int bogui_guilib_init()
{
	struct thread *cur = thread_current();

	if (cur->guilib != NULL) {
		return -1;
	}
	cur->guilib = mm.malloc(sizeof(bogui_libary_t));
	if (cur->guilib == NULL) {
		return -1;
	}
	//设定默认值
	cur->guilib->x = 0;
	cur->guilib->y = 0;
	cur->guilib->width = 360;
	cur->guilib->height = 240;
	cur->guilib->color = 0;
	cur->guilib->container = NULL;
	cur->guilib->buffer = NULL;
	
	return 0;
}

void bogui_guilib_window_size(int width, int height)
{
	struct thread *cur = thread_current();

	cur->guilib->width = width;
	cur->guilib->height = height;
}

void bogui_guilib_window_position(int x, int y)
{
	struct thread *cur = thread_current();

	cur->guilib->x = x;
	cur->guilib->y = y;
}

int bogui_guilib_window_create2(const char *title)
{
	
	struct thread *cur = thread_current();

	bogui_message_t msg;
	memset(&msg, 0, sizeof(bogui_message_t));
	
	msg.id = BOGUI_MSG_WINDOW_ADD;

	msg.data.ptr[0] = (char *)mm.malloc(BOGUI_WINDOW_TITLE_LEN);
	memset(msg.data.ptr[0], 0, BOGUI_WINDOW_TITLE_LEN);
	strcpy(msg.data.ptr[0], title);

	msg.data.ptr[1] = NULL;

	msg.data.i32[0] = cur->guilib->x;
	msg.data.i32[1] = cur->guilib->y;
	msg.data.i32[2] = cur->guilib->width;
	msg.data.i32[3] = cur->guilib->height;
	
	bogui_msg_send(&msg);

	//阻塞，等待创建窗口
	thread_block(THREAD_BLOCKED);
	
	//获取返回状态
	if (cur->exit_status == 1) {
		cur->exit_status = -1;
		printk("add ok!\n");
		return 0;
	} else if (cur->exit_status == 0) {
		cur->exit_status = -1;
		printk("add bad!\n");
		return -1;
	}

	return 0;
}

int bogui_guilib_window_create(const char *title)
{
	struct thread *cur = thread_current();

	//调整位置
	if (cur->guilib->y < BOGUI_TASKBAR_HEIGHT) {
		cur->guilib->y = BOGUI_TASKBAR_HEIGHT;
	} else if (cur->guilib->y > bogui_container_manager->height - BOGUI_WINDOW_BAR_HEIGHT) {
		cur->guilib->y = bogui_container_manager->height - BOGUI_WINDOW_BAR_HEIGHT;
	}

	//创建容器
	bogui_container_t *container = bogui_add_window((char *)title, cur->guilib->x, cur->guilib->y, \
		cur->guilib->width, cur->guilib->height);

	if (container == NULL) {
		
		return -1;
	}
	
	//创建窗口控制
	bogui_winctl_t *winctl = bogui_winctl_add((char *)title);

	if (winctl == NULL) {
		//错误就关闭窗口
		bogui_close_window(container->window);
		return -1;
	}
	/*互相绑定*/
	bogui_winctl_bind_window(winctl, container->window);
	bogui_window_bind_winctl(container->window, winctl);
	
	/*
	线程通过guilib绑定container
	容器直接绑定线程
	*/
	bogui_guilib_bind_container(cur->guilib, container);
	bogui_container_bind_thread(container, cur);
	bogui_window_bind_guilib(container->window, cur->guilib);	//绑定图形库
	
	/*
	发送一个改变窗口聚焦的消息
	然后会在里面把窗口控制聚焦
	*/
	bogui_message_t msg;
	msg.id = BOGUI_MSG_WINDOW_FOUCS;
	msg.data.ptr[0] = container;

	bogui_msg_send(&msg);

	return 0;
}

int bogui_guilib_window_close2()
{
	struct thread *cur = thread_current();
	//检测关闭环境
	if (cur->guilib == NULL) {
		return -1;
	}
	if (cur->guilib->container == NULL) {
		return -1;
	}

	//释放图形库

	bogui_message_t gmsg;
	/*发送一个添加窗口的消息*/
	gmsg.id = BOGUI_MSG_WINDOW_CLOSE;

	//设置关闭的窗口
	gmsg.data.ptr[0] = cur->guilib->container->window;
	
	gmsg.data.i32[0] = BOGUI_WINBTN_CLOSE_ACCORD;
						
	bogui_msg_send(&gmsg);
	return 0;
}

int bogui_guilib_window_close()
{
	struct thread *cur = thread_current();

	bogui_libary_t *guilib = cur->guilib;

	//检测关闭环境
	if (cur->guilib == NULL) {
		return -1;
	}
	if (cur->guilib->container == NULL) {
		return -1;
	}

	//释放窗口控制
	bogui_window_t *window = guilib->container->window;
	if (window != NULL) {
		bogui_winctl_close(window->winctl);
	}

	//释放窗口
	//关闭要操作的窗口容器
	bogui_close_window(window);

	//释放图形库
	//mm.free(guilib);

	/*发送设置顶层容器的消息*/
	bogui_message_t gmsg;
	gmsg.id = BOGUI_MSG_WINDOW_TOPEST;
	
	bogui_msg_send(&gmsg);
	return 0;
}

void bogui_guilib_color(unsigned int color)
{
	struct thread *cur = thread_current();
	cur->guilib->color = color;

}

void bogui_guilib_point(int x, int y)
{
	struct thread *cur = thread_current();
	
	bogui_window_t *window = cur->guilib->container->window;

	bogui_container_write_pixel(cur->guilib->container, \
		window->frame.x + x, window->frame.y + y, cur->guilib->color);
	
}

void bogui_guilib_line(int x1, int y1, int x2, int y2)
{
	struct thread *cur = thread_current();
	
	bogui_window_t *window = cur->guilib->container->window;

	bogui_container_draw_line(cur->guilib->container, \
		window->frame.x + x1, window->frame.y + y1, \
		window->frame.x + x2, window->frame.y + y2, \
		cur->guilib->color);
	
}

void bogui_guilib_rect(int x, int y, int width, int height)
{
	struct thread *cur = thread_current();
	
	bogui_window_t *window = cur->guilib->container->window;

	bogui_container_draw_rect(cur->guilib->container, \
		window->frame.x + x, window->frame.y + y, width, height, cur->guilib->color);
	
}

void bogui_guilib_text(int x, int y, char *text)
{
	struct thread *cur = thread_current();
	
	bogui_window_t *window = cur->guilib->container->window;

	bogui_container_draw_string(cur->guilib->container, \
		window->frame.x + x, window->frame.y + y, text, cur->guilib->color);
	
}

void bogui_guilib_frame(int x, int y, int width, int height)
{
	struct thread *cur = thread_current();

	if (cur->guilib->buffer == NULL) {
		return;
	}
	bogui_window_t *window = cur->guilib->container->window;

	uint32_t *buf = (uint32_t *)cur->guilib->buffer;

	int i, j;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			bogui_container_write_pixel(cur->guilib->container, \
				window->frame.x + x + i, window->frame.y + y + j, *(buf + (j*width+i)));
		}
	}
}

void bogui_guilib_buffer(unsigned char *buffer)
{
	struct thread *cur = thread_current();
	cur->guilib->buffer = buffer;

}

void bogui_guilib_draw(int x1, int y1, int x2, int y2)
{
	struct thread *cur = thread_current();
	bogui_window_t *window = cur->guilib->container->window;

	if (cur->guilib->container->refresh_lock) {
		return;
	}
	bogui_container_refresh(cur->guilib->container, \
		window->frame.x + x1, window->frame.y + y1, window->frame.x + x2, window->frame.y + y2);

}

void bogui_guilib_bind_container(bogui_libary_t *guilib, bogui_container_t *container)
{
	guilib->container = container;

}

/*
捕捉鼠标位置
*/
int bogui_guilib_mouse_move(int *x, int *y)
{
	struct thread *cur = thread_current();
	
	bogui_window_t *window = cur->guilib->container->window;

	if (window->frame.catch_mouse_move) {
		*x = window->frame.mouse_x;
		*y = window->frame.mouse_y;
		window->frame.catch_mouse_move = 0;

		return 0;
	}
	return -1;
}

/*
捕捉鼠标点击
*/
int bogui_guilib_mouse_click(int *key, int *status, int *x, int *y)
{
	struct thread *cur = thread_current();
	
	bogui_window_t *window = cur->guilib->container->window;

	if (window->frame.catch_mouse_click) {
		*x = window->frame.mouse_x;
		*y = window->frame.mouse_y;

		*key = window->frame.mouse_key;
		*status = window->frame.mouse_status;

		window->frame.catch_mouse_click = 0;

		return 0;
	}

	return -1;
}

/*
捕捉键盘按下
*/
int bogui_guilib_keyboard(int *key)
{
	//检测线程取消点
	thread_do_testcancel();

	struct thread *cur = thread_current();
	
	bogui_window_t *window = cur->guilib->container->window;

	if (window->frame.catch_keyboard) {
		
		//只取字符和特殊键，其它标志取消，我们用其它函数处理其它标志
		*key = window->frame.keyboard_key&0x1ff;


		window->frame.catch_keyboard = 0;

		return 0;
	}
	
	return -1;
}
/*
获取shift，ctrl，alt按键的状态
*/
int bogui_guilib_get_modifiers()
{
	int data = 0;

	if (keyboard.shift_l || keyboard.shift_r) {
		data |= GUI_ACTIVE_SHIFT;
	}
	if (keyboard.ctrl_l || keyboard.ctrl_r) {
		data |= GUI_ACTIVE_CTRL;
	}
	if (keyboard.alt_l || keyboard.alt_r) {
		data |= GUI_ACTIVE_ALT;
	}
	return data;
}















#endif //_CONFIG_GUI_BOGUI_

