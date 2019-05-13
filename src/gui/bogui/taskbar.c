/*
Auther:		Jason Hu
Time:		2019/5/4
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <sys/gui.h>
#include <sys/mm.h>
#include <string.h>

struct bogui_taskbar_s bogui_taskbar;

bogui_message_t taskbar_msg_out;

void bogui_taskbar_init()
{

	/*初始化任务栏*/
	bogui_taskbar.container = bogui_container_alloc(video_info.width* BOGUI_TASKBAR_HEIGHT* 4);
	bogui_container_init(bogui_taskbar.container,"fixed", 0, 0,video_info.width, BOGUI_TASKBAR_HEIGHT, 4);
	bogui_container_add_attribute(bogui_taskbar.container, BOGUI_CONTAINER_FIXED);
	bogui_container_draw_rect(bogui_taskbar.container, 0, 0, \
		bogui_taskbar.container->width, bogui_taskbar.container->height, BOGUI_TASKBAR_BCOLOR);
	bogui_container_axis(bogui_taskbar.container, 1);
	bogui_container_refresh(bogui_taskbar.container,0, 0, bogui_taskbar.container->width, bogui_taskbar.container->height);

	bogui_container_bind_listen(bogui_taskbar.container, GOGUI_MOUSE_MOVE_LISTEN, bogui_taskbar_mouse_move_listen);
	bogui_container_bind_listen(bogui_taskbar.container, GOGUI_MOUSE_CLICK_LISTEN, bogui_taskbar_mouse_click_listen);

	bogui_taskbar.winctl_header = NULL;
	bogui_taskbar.winctl_count = 0;
	bogui_taskbar.winctl_activing = NULL;	//没有激活的
	bogui_taskbar.bcolor = BOGUI_TASKBAR_BCOLOR;
	bogui_taskbar.ftcolor = ARGB_WHITE;

	bogui_taskbar.width = video_info.width;
	bogui_taskbar.height = BOGUI_TASKBAR_HEIGHT;
	

	/*bogui_winctl_t *winctl;

	winctl = bogui_winctl_create("win1");
	bogui_winctl_append(winctl);
	bogui_winctl_bind_container(winctl, bogui_taskbar.container);

	bogui_winctl_draw(winctl);

	winctl = bogui_winctl_create("win2");
	bogui_winctl_append(winctl);
	bogui_winctl_bind_container(winctl, bogui_taskbar.container);

	bogui_winctl_draw(winctl);

	winctl = bogui_winctl_create("win3");
	bogui_winctl_append(winctl);
	bogui_winctl_bind_container(winctl, bogui_taskbar.container);

	bogui_winctl_draw(winctl);
*/
	//bogui_container_refresh(bogui_taskbar.container,0, 0, bogui_taskbar.container->width, bogui_taskbar.container->height);

	/*
	printk("winctl:%s \n", bogui_taskbar.winctl_header->name);
	printk("winctl:%s \n", bogui_taskbar.winctl_header->next->name);
	printk("winctl:%s \n", bogui_taskbar.winctl_header->next->next->name);
	
	bogui_winctl_sub(bogui_taskbar.winctl_header->next);
	bogui_winctl_sub(bogui_taskbar.winctl_header);
	//bogui_winctl_sub(bogui_taskbar.winctl_header);

	printk("winctl:%s \n", bogui_taskbar.winctl_header->name);
	printk("winctl:%s \n", bogui_taskbar.winctl_header->next->name);
*/

}

bogui_winctl_t *bogui_winctl_add(char *title)
{
	bogui_winctl_t *winctl = bogui_winctl_create(title);
	bogui_winctl_append(winctl);
	bogui_winctl_bind_container(winctl, bogui_taskbar.container);
	return winctl;
}

bogui_winctl_t *bogui_winctl_create(char *title)
{
	bogui_winctl_t *winctl = mm.malloc(sizeof(bogui_winctl_t));
	memset(winctl, 0, sizeof(bogui_winctl_t));
	
	memset(winctl->name, 0, BOGUI_WINCTL_TITLE_LEN);
	strcpy(winctl->name, title);
	
	winctl->x = bogui_taskbar.winctl_count*BOGUI_WINCTL_WIDTH + 1*bogui_taskbar.winctl_count;
	winctl->y = 0;
	
	winctl->width = BOGUI_WINCTL_WIDTH;
	winctl->height = BOGUI_WINCTL_HEIGHT;
	winctl->next = NULL;

	winctl->container = NULL;
	winctl->window = NULL;

	winctl->color_idle = BOGUI_TASKBAR_BCOLOR;
	winctl->color_touch = ARGB(255, 128,128,128);
	winctl->color_active = ARGB(255, 80,80,80);

	winctl->flags = 0;
	winctl->status = BOGUI_WINCTL_IDLE;
	return winctl;
}

int bogui_winctl_append(bogui_winctl_t *winctl)
{
	if (winctl == NULL) {
		return -1;
	}

	//先判断窗口控制头是否为空
	if (bogui_taskbar.winctl_header == NULL) {
		//添加到队首
		bogui_taskbar.winctl_header = winctl;
		bogui_taskbar.winctl_header->next = NULL;
		bogui_taskbar.winctl_count++;

		return 0;
	}

	bogui_winctl_t *p;

	p = bogui_taskbar.winctl_header;
	//队列寻找，直到发现一个下一个为NULL的
	while(p->next != NULL){
		p = p->next;
	}

	//添加到后面
	p->next = winctl;
	winctl->next = NULL;
	bogui_taskbar.winctl_count++;

	return 0;
}

int bogui_winctl_close(bogui_winctl_t *winctl)
{
	
	if (winctl == NULL) {
		return -1;
	}

	//从链表上脱去
	if (!bogui_winctl_sub(winctl)) {
		winctl->container = NULL;
		winctl->window = NULL;
		
		//把winctl释放
		mm.free(winctl);
		return 0;
	}

	return -1;
}


int bogui_winctl_sub(bogui_winctl_t *winctl)
{
	if (winctl == NULL) {
		return -1;
	}

	bogui_winctl_t *p = bogui_taskbar.winctl_header, *q;

	if (p == NULL) {
		//队首都是空的
		return -1;
	}
	//队首就是要删除的
	if (p == winctl) {
		//在删除之前要把图层上的内容清除
		/*winctl->status = BOGUI_WINCTL_CLOSE;
		bogui_winctl_draw(winctl);*/

		//队首指向下一个
		bogui_taskbar.winctl_header = bogui_taskbar.winctl_header->next;

		bogui_taskbar.winctl_count--;

		//调整位置
		bogui_winctl_sort();

		return 0;
	}
	//队中删除
	while (p != NULL) {
		//指向下一个，保存上一个
		q = p;
		p = p->next;

		//在队列中找到了
		if (p == winctl) {

			//在删除之前要把图层上的内容清除
			/*winctl->status = BOGUI_WINCTL_CLOSE;
			bogui_winctl_draw(winctl);*/

			//上一个的下一个指向当前的下一个，这样，当前的就从队列中脱去
			q->next = p->next;

			bogui_taskbar.winctl_count--;

			//调整位置
			bogui_winctl_sort();

			return 0;
		}
	}
	//没有找到，失败
	return -1;
}
/*
绑定在哪个容器上
*/
void bogui_winctl_bind_container(bogui_winctl_t *winctl, struct bogui_container_s *container)
{
	winctl->container = container;
}

void bogui_winctl_bind_window(bogui_winctl_t *winctl, struct bogui_window_s *window)
{
	winctl->window = window;
}
void bogui_winctl_draw(bogui_winctl_t *winctl)
{
	//printk("w:%s\n", winctl->name);
	uint32_t color;
	if (winctl->status == BOGUI_WINCTL_IDLE) {
		color = winctl->color_idle;
	} else if (winctl->status == BOGUI_WINCTL_TOUCH) {
		color = winctl->color_touch;
	} else if (winctl->status == BOGUI_WINCTL_ACTIVE) {
		color = winctl->color_active;	
	} else if (winctl->status == BOGUI_WINCTL_CLOSE) {
		color = bogui_taskbar.bcolor;	
	}

	//绘制背景
	bogui_container_draw_rect(winctl->container, winctl->x, winctl->y, winctl->width, winctl->height, color);
	
	//把窗口容器里面的图标绘制到任务栏容器
	bogui_container_draw_winicon(winctl->window->container, winctl->container, winctl->x+1, winctl->y+4, color);

	//绘制文字，截取部分来绘制
	char s[9];	//最多绘制8个字符
	memset(s, 0, 9);

	int i;
	for (i = 0; i < 8; i++) {
		s[i] = winctl->name[i];
	}

	bogui_container_draw_string(winctl->container, winctl->x + 2 + 16, winctl->y+4, s, bogui_taskbar.ftcolor);
	
	//刷新
	bogui_container_refresh(winctl->container, winctl->x, winctl->y, winctl->x + winctl->width, winctl->y + winctl->height);
}
/*
重新调整位置
*/
void bogui_winctl_sort()
{
	bogui_winctl_t *p = bogui_taskbar.winctl_header;

	//先把任务栏清空，在排列

	//绘制背景
	bogui_container_draw_rect(bogui_taskbar.container, 0, 0, bogui_taskbar.container->width, bogui_taskbar.container->height, bogui_taskbar.bcolor);
	//bogui_container_refresh(bogui_taskbar.container, 0, 0,bogui_taskbar.container->width, bogui_taskbar.container->height);

	//调整位置
	int i = 0;
	while (p != NULL) {
		p->x = i*BOGUI_WINCTL_WIDTH + i;
		i++;
		//显示再新的位置
		bogui_winctl_draw(p);

		//指向下一个
		p = p->next;
	}
	//最后来个总的刷新
	bogui_container_refresh(bogui_taskbar.container, 0, 0,bogui_taskbar.container->width, bogui_taskbar.container->height);
}

/*
鼠标移动监听事件
*/
void bogui_taskbar_mouse_move_listen(struct bogui_container_s *container, int mousex, int mousey)
{
	//printk("%d,%d ", mousex, mousey);

	//对窗口控制的检测
	bogui_winctl_t *winctl = bogui_taskbar.winctl_header;
	while (winctl != NULL) {
		//进入范围
		if (winctl->x <= mousex && mousex < winctl->x + winctl->width && \
			winctl->y <= mousey && mousey < winctl->y + winctl->height ) {
			if (winctl->status != BOGUI_WINCTL_ACTIVE) {
				winctl->status = BOGUI_WINCTL_TOUCH;
				bogui_winctl_draw(winctl);
			}
		} else if (winctl->status != BOGUI_WINCTL_ACTIVE) {
			winctl->status = BOGUI_WINCTL_IDLE;
			bogui_winctl_draw(winctl);
		}
		winctl = winctl->next;
	}
	
}
/*
鼠标点击监听事件
*/
void bogui_taskbar_mouse_click_listen(struct bogui_container_s *container, int mousex, int mousey, int key, int status)
{
	//printk("%s ", container->name);
	if (key == MOUSE_LEFT ) {
		if (status == MOUSE_DOWN) {
			bogui_winctl_t *winctl = bogui_taskbar.winctl_header;
	
			while (winctl != NULL) {
				//在范围内就激活，不然就取消
				if (winctl->x <= mousex && mousex < winctl->x + winctl->width && \
					winctl->y <= mousey && mousey < winctl->y + winctl->height ) {

					//如果已经激活了，就取消
					if (winctl->status == BOGUI_WINCTL_ACTIVE) {
						winctl->status = BOGUI_WINCTL_IDLE;
						bogui_winctl_draw(winctl);

						//发送隐藏窗口消息
						taskbar_msg_out.id = BOGUI_MSG_WINDOW_HIDE;
						taskbar_msg_out.data.ptr[0] = winctl->window->container;

						bogui_msg_send(&taskbar_msg_out);

					} else {	//不然就激活
						winctl->status = BOGUI_WINCTL_ACTIVE;
						bogui_winctl_draw(winctl);
						//激活对应的窗口

						//发送显示隐藏窗口消息
						taskbar_msg_out.id = BOGUI_MSG_WINDOW_SHOW;
						taskbar_msg_out.data.ptr[0] = winctl->window->container;

						bogui_msg_send(&taskbar_msg_out);

					}
					
				} else if (winctl->status == BOGUI_WINCTL_ACTIVE) {
					//只改变窗口控制，而不隐藏窗口
					winctl->status = BOGUI_WINCTL_IDLE;
					bogui_winctl_draw(winctl);

				}
				winctl = winctl->next;
			}
		} else if (status == MOUSE_UP ) {

		}
	} else if (key == MOUSE_RIGHT ) {
		if (status == MOUSE_DOWN) {

		} else if (status == MOUSE_UP ) {

		}
	} else if (key == MOUSE_MIDDLE ) {
		if (status == MOUSE_DOWN) {

		} else if (status == MOUSE_UP ) {

		}
	}
}
/*
对具体的某个进行聚焦，其他的都不聚焦
*/
void bogui_taskbar_winctl_focus(bogui_winctl_t *winctl)
{
	//如果是空的就直接返回
	if (winctl == NULL) {
		return;
	}
	bogui_winctl_t *p = bogui_taskbar.winctl_header;
	while (p != NULL) {
		//是要聚焦的就激活
		if (p == winctl) {
			//如果没有激活才激活
			if (p->status != BOGUI_WINCTL_ACTIVE) {
				p->status = BOGUI_WINCTL_ACTIVE;
				bogui_winctl_draw(p);	

			}

		} else {
			//不是要聚焦的就idle
			p->status = BOGUI_WINCTL_IDLE;
			bogui_winctl_draw(p);
		}
		p = p->next;
	}
	
}

/*
把所有的窗口控制都设置成不聚焦
*/
void bogui_taskbar_winctl_cancel_all()
{
	bogui_winctl_t *p = bogui_taskbar.winctl_header;
	while (p != NULL) {
		//都设置成idle
		p->status = BOGUI_WINCTL_IDLE;
		bogui_winctl_draw(p);
		p = p->next;
	}
	
}


/*
对具体的某个进行不聚焦
*/
void bogui_taskbar_winctl_cancel(bogui_winctl_t *winctl)
{
	//设置成idle
	winctl->status = BOGUI_WINCTL_IDLE;
	bogui_winctl_draw(winctl);

}

#endif //_CONFIG_GUI_BOGUI_

