/*
File:		gui/bogui/bogui.c
Contains:	bogui init
Auther:		Hu axisicheng
Time:		2019/4/21
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <string.h>
#include <sys/gui.h>

static bogui_message_t window_msg_out;

bogui_window_t *bogui_create_window(char *title, int x, int y, int width, int height)
{

	bogui_window_t *window = (bogui_window_t *)mm.malloc(sizeof(bogui_window_t));
	if (window == NULL) {
		return NULL;
	}
	window->x = x;
	window->y = y;
	window->width = width;
	window->height = height;
	window->status = BOGUI_WINDOW_INACTIVE;

	window->bc_inactive = BOGUI_WINDOW_COLOR_INACTIVE;
	window->bc_active = BOGUI_WINDOW_COLOR_ACTIVE;

	window->container = NULL;
	window->winctl = NULL;
	window->guilib = NULL;

	memset(window->title, 0, BOGUI_WINDOW_TITLE_LEN);
	strcpy(window->title, title);

	window->frame.x = 1;
	window->frame.y = BOGUI_WINDOW_BAR_HEIGHT;
	window->frame.width = width-2;
	window->frame.height = height-25;

	//创建时没有图标
	window->winicon.data = NULL;

	return window;
}

void bogui_close_window(bogui_window_t *window)
{
	bogui_container_t *container = window->container;

	//释放窗口结构体
	mm.free(container->window);

	//释放窗口的图标
	mm.free(window->winicon.data);

	//释放容器本身
	bogui_container_free(container);

	//发送关闭进程
	
}

void bogui_window_bind_container(bogui_window_t *window, struct  bogui_container_s *container)
{
	window->container = container;
}

void bogui_window_bind_winctl(bogui_window_t *window, struct bogui_winctl_s *winctl)
{
	window->winctl = winctl;
}

void bogui_window_bind_guilib(bogui_window_t *window, struct  bogui_libary_s *guilib)
{
	window->guilib = guilib;
}

void bogui_window_draw_body(bogui_window_t *window, char activate)
{
	int title_len;

	bogui_container_t *container = window->container;

	uint32_t color_bar,color_title,color_border;
	if(!activate){	// 0
		color_bar = window->bc_inactive;
		color_title = BOGUI_WINDOW_COLOR_TITLE_INACTIVE;
		color_border = window->bc_inactive + 0x202020;
	}else{
		color_bar = window->bc_active;
		color_title = BOGUI_WINDOW_COLOR_TITLE_ACTIVE;
		color_border = window->bc_active + 0x202020;
	}

	//绘制窗体及边框
	bogui_container_draw_rect(container, 1, 0, container->width-2,BOGUI_WINDOW_BAR_HEIGHT, color_bar);
	
	bogui_container_draw_rect(container, 0, 0, 1,container->height, color_border);
	bogui_container_draw_rect(container, 0, 0, container->width,1, color_border);
	bogui_container_draw_rect(container, container->width-1, 0, 1,container->height, color_border);
	bogui_container_draw_rect(container, 0, container->height-1, container->width,1, color_border);
	
	bogui_container_draw_rect(container, 0, 0, container->width,1, color_border);
	
	//绘制标题
	title_len = strlen(window->title);
	bogui_container_draw_string(container,  1+16+4, 4,window->title, color_title);

	/*绘制图标，按钮*/
	bogui_container_draw_winicon( container, container, 4, 4, color_border);

	bogui_winbtn_t *winbtn = &window->winbtn[0];
	bogui_container_draw_button_minimize(container, winbtn->x + 2, winbtn->y + 3, color_bar);
	winbtn = &window->winbtn[1];
	bogui_container_draw_button_close(container, winbtn->x + 3, winbtn->y + 3, color_bar);
	
} 

void bogui_window_draw_frame(bogui_window_t *window)
{
	bogui_frame_t *frame = &window->frame;

	//绘制FRAME
	bogui_container_draw_rect(window->container, frame->x, frame->y, frame->width, frame->height, ARGB(255, 200,200,200));

} 

void bogui_inactive_all_window()
{
	int j;
	bogui_container_t *container;

	/*从最底层的上一层到最顶层的下一层*/
	for(j = bogui_container_manager->top - 1; j > 0; j--){
		container = bogui_container_manager->container_addr[j];
		//是一个窗口容器
		if (container->flags&BOGUI_CONTAINER_WINDOW) {
			//并且是已经激活的窗口
			if (container->window->status == BOGUI_WINDOW_ACTIVE) {
				//就把窗口设置成未激活
				container->window->status = BOGUI_WINDOW_INACTIVE;
				//把窗口样式修改
				bogui_window_draw_body(container->window, container->window->status);
				bogui_container_refresh(container, 0, 0, container->width, container->height);
				//只有一个激活窗口，所以跳出
				//printk("inactive ");
				break;
			}
		}
	}
}

/*
激活一个不为空的窗口容器
*/
void bogui_window_active(struct bogui_container_s *container)
{

	container->window->status = BOGUI_WINDOW_ACTIVE;
	bogui_window_draw_body(container->window, container->window->status);
	bogui_container_refresh(container, 0, 0, container->width, container->height);
}
/*
获取一个最高的窗口容器，如果没有，返回空
*/
struct bogui_container_s *bogui_get_topest_window()
{
	int j;
	bogui_container_t *container;

	/*从最底层的上一层到最顶层的下一层*/
	for(j = bogui_container_manager->top - 1; j > 0; j--){
		container = bogui_container_manager->container_addr[j];
		//是一个窗口容器，但不是隐藏的
		if (container->flags&BOGUI_CONTAINER_WINDOW && !(container->flags&BOGUI_CONTAINER_HIDE)) {
			//并且是已经激活的窗口
			return container;
		}
	}
	return NULL;
}

struct bogui_container_s *bogui_add_window(char *title, int x, int y, int width, int height)
{
	int con_w = width + 2;
	int con_h = height + 25;
	//做一些内存分配，如果失败就直接返回
	uint8_t *winiocn_data = mm.malloc(BOGUI_WINICON_WIDHT*BOGUI_WINICON_HEIGHT*4);
	if (winiocn_data == NULL) {
		return NULL;
	}

	//容器的创建与初始化
	bogui_container_t *container = bogui_container_alloc(con_w*con_h*4);
	if (container == NULL) {
		mm.free(winiocn_data);
		return NULL;
	}
	bogui_container_init(container,"window", x, y,con_w, con_h, 4);
	bogui_container_add_attribute(container, BOGUI_CONTAINER_WINDOW);

	//窗口的创建与初始化
	bogui_window_t *window = bogui_create_window(title, x, y, con_w,con_h);
	if (window == NULL) {
		bogui_container_free(container);
		mm.free(winiocn_data);
		return NULL;
	}
	//窗口与容器互相绑定
	bogui_window_bind_container(window, container);
	bogui_container_bind_window(container, window);

	bogui_container_bind_listen(container, GOGUI_MOUSE_MOVE_LISTEN, bogui_window_mouse_move_listen);
	bogui_container_bind_listen(container, GOGUI_MOUSE_CLICK_LISTEN, bogui_window_mouse_click_listen);
	bogui_container_bind_listen(container, GOGUI_KEYBOARD_LISTEN, bogui_window_keyboard_listen);

	//绘制窗口前要提前绘制图标数据
	//默认创建窗口都有一个默认的图标，程序可以修改
	bogui_image_make_winicon(winiocn_data, BOGUI_WINICON_WIDHT,BOGUI_WINICON_HEIGHT, 4);
	bogui_window_bind_winicon(&window->winicon, winiocn_data, BOGUI_WINICON_WIDHT,BOGUI_WINICON_HEIGHT, 4);

	//初始化窗口按钮
	bogui_winbtn_init(&window->winbtn[0], container, window->width - 46, 1, 22, 22, BOGUI_WINBTN_MINIMIZE);
	bogui_winbtn_init(&window->winbtn[1], container, window->width - 23, 1, 22, 22, BOGUI_WINBTN_CLOSE);
	
	//绘制窗口
	bogui_window_draw_body(window, 0);
	bogui_window_draw_frame(window);

	//从系统中设定新建容器的高度，设置成顶层高度，顶层会+1
	bogui_container_axis(container, bogui_container_manager->top );

	bogui_container_slide(container, x, y);
	//刷新显示出来
	bogui_container_refresh(container,0, 0, con_w, con_h);

	return container;
}

void bogui_window_bind_winicon(bogui_winicon_t *winicon, uint8_t *buffer, int width, int height, int bit_width)
{
	winicon->width = width;
	winicon->height = height;
	winicon->bit_width = bit_width;

	winicon->data = (uint8_t *)mm.malloc(winicon->width*winicon->height*4);
	memcpy(winicon->data, buffer, winicon->width*winicon->height*4);
}

void bogui_winbtn_init(bogui_winbtn_t *winibtn, struct bogui_container_s *container, \
	int x, int y, int width, int height, uint8_t type)
{
	winibtn->x = x;
	winibtn->y = y;
	winibtn->width = width;
	winibtn->height = height;

	winibtn->status = BOGUI_WINBTN_IDLE;

	winibtn->container = container;
	winibtn->type = type;

	winibtn->color = 0;

	//根据类型初始化触摸时的颜色
	if (type == BOGUI_WINBTN_MINIMIZE) {
		winibtn->color_touch = ARGB(255, 120,120,120);
	} else if (type == BOGUI_WINBTN_CLOSE) {
		winibtn->color_touch = ARGB(255, 240,80,0);
	}

}

/*
鼠标移动监听事件
*/
void bogui_window_mouse_move_listen(struct bogui_container_s *container, int mousex, int mousey)
{
	//判断鼠标是否在容器里面,不是的话就直接返回
	/*if (mousex < 0 || mousey < 0 || mousex >= container->width || mousey >= container->height ) {
		return;
	}*/
	//printk("%d,%d ", mousex, mousey);

	bogui_window_t *window = container->window;
	bogui_winbtn_t *winbtn;
	bogui_frame_t *frame = &window->frame;
	

	bogui_container_t *touch = bogui_get_container_where_mouse_on(mousex + container->x, mousey + container->y);
	
	/*
	鼠标与按钮
	*/
	int i;

	for (i = 0; i < BOGUI_WINBTN_NR; i++) {
		winbtn = &window->winbtn[i];
		//在范围内
		if (winbtn->x <= mousex \
			&& mousex < winbtn->x + winbtn->width \
			&& winbtn->y <= mousey \
			&& mousey < winbtn->y + winbtn->height) {
			
			//printk("1 ");
			if (touch == container) { //判断鼠标是否在上面
				if (winbtn->status != BOGUI_WINBTN_TOUCH) {
					winbtn->status = BOGUI_WINBTN_TOUCH;
					//printk("2 ");

					bogui_winbtn_draw(winbtn);
				}
			}
			
		//不在范围内
		} else if (winbtn->status != BOGUI_WINBTN_IDLE) {
			winbtn->status = BOGUI_WINBTN_IDLE;
			//printk("3 ");

			bogui_winbtn_draw(winbtn);
		}
	}

	/*
	如果是在frame窗口里面
	*/
	if (frame->x <= mousex && mousex < frame->x + frame->width && \
		frame->y <= mousey && mousey < frame->y + frame->height ) {
		if (touch == container) {	//判断鼠标是否在上面
			//传递鼠标数据
			frame->catch_mouse_move = 1;

			frame->mouse_x = mousex - frame->x;
			frame->mouse_y = mousey - frame->y;
		}	
	} else {
		//不传递鼠标数据
		window->frame.catch_mouse_move = 0;
	}
}

/*
鼠标点击监听事件			
*/
void bogui_window_mouse_click_listen(struct bogui_container_s *container, int mousex, int mousey, int key, int status)
{
	bogui_window_t *window = container->window;
	bogui_winbtn_t *winbtn;
	bogui_frame_t *frame = &window->frame;
	bogui_container_t *touch = bogui_get_container_where_mouse_on(mousex + container->x, mousey + container->y);
	
	/*
	如果是在frame窗口里面
	*/
	if (frame->x <= mousex && mousex < frame->x + frame->width && \
		frame->y <= mousey && mousey < frame->y + frame->height ) {
		if (touch == container) { //判断鼠标是否在上面
			//传递鼠标数据
			frame->catch_mouse_click = 1;

			frame->mouse_x = mousex - frame->x;
			frame->mouse_y = mousey - frame->y;
			
			frame->mouse_key = key;
			frame->mouse_status = status;
		}
	} else {
		//不传递鼠标数据
		frame->catch_mouse_click = 0;
	}

	//如果没有抓取，就返回，不执行窗口标题栏操作，不再检测
	if (bogui_container_manager->can_hand_container == 0) {
		return;
	}
	/*
	鼠标与按钮点击
	*/
	if (touch != container) {
		return;		
	}
	int i;
	if (key == MOUSE_LEFT) {
		if (status == MOUSE_DOWN) {
			
			//窗口按钮处理
			for (i = 0; i < BOGUI_WINBTN_NR; i++) {
				winbtn = &window->winbtn[i];
				//在范围内
				if (winbtn->x <= mousex \
					&& mousex < winbtn->x + winbtn->width \
					&& winbtn->y <= mousey \
					&& mousey < winbtn->y + winbtn->height) {
					//memset(&window_msg_out, 0, sizeof(bogui_message_t));
					//点击哪种类型的按钮
					if (winbtn->type == BOGUI_WINBTN_MINIMIZE) {
						//先发送一个隐藏窗口的消息
						window_msg_out.id = BOGUI_MSG_WINDOW_HIDE;

						window_msg_out.data.ptr[0] = container;
						bogui_msg_send(&window_msg_out);

					} else if (winbtn->type == BOGUI_WINBTN_CLOSE) {	//关闭窗口按钮
						//取消线程
						thread_cancel(container->thread);

					}
					/*
					点击后就不能继续监测其它的点击，必须等待下一次点击
					用于避免几个窗口叠加在一起，最上层操作完后会监测下面一层，
					从而又会执行类似的操作
					*/
					bogui_container_manager->can_hand_container = 0;
				} 
			}
			
		} else if (status == MOUSE_UP) {

		}

	} else if (key == MOUSE_RIGHT) {
		if (status == MOUSE_DOWN) {

		} else if (status == MOUSE_UP) {

		}
	} else if (key == MOUSE_MIDDLE) {
		if (status == MOUSE_DOWN) {

		} else if (status == MOUSE_UP) {

		}
	}

}

/*
鼠标点击监听事件			
*/
void bogui_window_keyboard_listen(struct bogui_container_s *container, int key_data)
{
	//printk("window:%s data:%x\n", container->window->title, key_data);
	bogui_frame_t *frame = &container->window->frame;
	
	frame->catch_keyboard = 1;
	frame->keyboard_key = key_data;
	
}


void bogui_winbtn_draw(bogui_winbtn_t *winbtn)
{
	if (winbtn->status == BOGUI_WINBTN_IDLE) {
		/*
		如果是空闲状态，就绘制成背景颜色
		背景分为激活和未激活颜色
		*/
		if (winbtn->container->window->status&BOGUI_WINDOW_ACTIVE) {
			winbtn->color = winbtn->container->window->bc_active;
		} else {
			winbtn->color = winbtn->container->window->bc_inactive;
		}
	} else if (winbtn->status == BOGUI_WINBTN_TOUCH) {
		winbtn->color = winbtn->color_touch;
	}


	//绘制背景
	bogui_container_draw_rect(winbtn->container, winbtn->x, winbtn->y, winbtn->width, winbtn->height, winbtn->color);
	
	//把窗口容器里面的按钮绘制到任务栏容器
	if (winbtn->type == BOGUI_WINBTN_MINIMIZE) {
		
		bogui_container_draw_button_minimize(winbtn->container, winbtn->x + 2, winbtn->y + 3, winbtn->color);
	} else if (winbtn->type == BOGUI_WINBTN_CLOSE) {
		bogui_container_draw_button_close(winbtn->container, winbtn->x + 3, winbtn->y + 3, winbtn->color);
	}
	
	//刷新
	bogui_container_refresh(winbtn->container, winbtn->x, winbtn->y, winbtn->x + winbtn->width, winbtn->y + winbtn->height);
}


void bogui_window_draw_point(bogui_window_t *window, int x, int y, uint32_t color)
{
	bogui_frame_t *frame = &window->frame;
	//超出范围就返回
	if (x < 0 || x >= frame->width || y < 0 || y >= frame->height) {
		return;
	}
	
	bogui_container_write_pixel(window->container, frame->x + x, frame->y + y, color);

}

void bogui_window_draw_rect(bogui_window_t *window, int x, int y, int width, int height, uint32_t color)
{
	bogui_frame_t *frame = &window->frame;

	bogui_container_draw_rect(window->container, frame->x + x, frame->y + y, \
		width, height, color);

}

void bogui_window_draw_line(bogui_window_t *window, int x0, int y0, int x1, int y1, uint32_t color)
{
	bogui_frame_t *frame = &window->frame;

	bogui_container_draw_line(window->container, frame->x + x0, frame->y + y0, \
		frame->x + x1, frame->y + y1, color);

}
void bogui_window_draw_char(bogui_window_t *window, int x,int y, char ch, uint32_t color)
{
	bogui_frame_t *frame = &window->frame;

	bogui_container_draw_char(window->container, frame->x + x, frame->y + y, color, ch);

}
void bogui_window_draw_string(bogui_window_t *window, int x,int y, char *s, uint32_t color)
{
	bogui_frame_t *frame = &window->frame;

	bogui_container_draw_string(window->container, frame->x + x, frame->y + y, s, color);

}
void bogui_window_refresh(bogui_window_t *window, int x0, int y0, int x1, int y1)
{
	bogui_frame_t *frame = &window->frame;

	bogui_container_refresh(window->container, frame->x + x0, frame->y + y0, frame->x + x1, frame->y + y1);

}

#endif //_CONFIG_GUI_BOGUI_

