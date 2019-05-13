/*
File:		gui/bogui/bogui.c
Contains:	bogui init
Auther:		Hu Zicheng
Time:		2019/4/21
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <sys/gui.h>
#include <sys/dev.h>
#include <string.h>
#include <sys/debug.h>

bogui_container_manager_t *bogui_container_manager;

bogui_container_t *buttom_container, *top_container, *taskbar_container, *test_container;

bogui_message_t msg_in, msg_out;

void init_bogui()
{
	printk("> init bugui start.\n");

	//printk(" vram:%x width:%d height:%d\n", video_info.vram, video_info.width, video_info.height);
	
	/*初始化容器管理器*/
	bogui_container_manager = bogui_container_managerr_init(video_info.vram, video_info.width, video_info.height);

	/*初始化最底层容器（桌面）*/
	buttom_container = bogui_container_alloc(video_info.width*video_info.height*4);
	bogui_container_init(buttom_container,"buttom", 0, 0,video_info.width, video_info.height, 4);
	bogui_container_add_attribute(buttom_container, BOGUI_CONTAINER_BUTTOM);
	bogui_container_draw_rect(buttom_container, 0, 0, video_info.width, video_info.height, ARGB(255,220,220,220));
	bogui_container_axis(buttom_container, 0);
	bogui_container_refresh(buttom_container,0, 0, video_info.width, video_info.height);

	bogui_taskbar_init();

	/*初始化最高层容器（鼠标）*/
	top_container = bogui_container_alloc(16*16*4);
	bogui_container_init(top_container,"top",  video_info.width/2, video_info.height/2, 16, 16, 4);
	bogui_container_add_attribute(top_container, BOGUI_CONTAINER_TOP);
	bogui_container_draw_mouse(top_container);
	bogui_container_axis(top_container, 2);
	bogui_container_refresh(top_container,0, 0, 16, 16);
	
	//把当前的容器指向底层容器
	bogui_container_manager->current_container = buttom_container;
	bogui_container_manager->hand_container = NULL;
	bogui_container_manager->moveing_container = NULL;
	
	/*默认是可以扫描容器的*/
	bogui_container_manager->can_hand_container = 1;
	//没有容器在移动
	bogui_container_manager->is_container_moving = 0;

	/*以上完成了基本设定，下面就可以创建窗口了*/
	
	//初始化消息消息机制
	bogui_message_init();
	
	bogui_debug_init();
	
	printk("< init bugui end.\n");
}

void bogui_do_thread()
{
	
	memset(&msg_in, 0, sizeof(bogui_message_t));
	if (!bogui_msg_receive(&msg_in)) {

		if (msg_in.id == BOGUI_MSG_UNKNOW) {

		} else if (msg_in.id == BOGUI_MSG_MOUSE_MOVE) {
			/*
			消息数据：
			i32[0] = mouse.x
			i32[1] = mouse.y

			ptr[0] = mouse container
			*/
			bogui_container_slide(msg_in.data.ptr[0] , msg_in.data.i32[0], msg_in.data.i32[1]);

		} else if (msg_in.id == BOGUI_MSG_WINDOW_MOVE) {
			/*
			消息数据：
			i32[0] = window x
			i32[1] = window y

			ptr[0] = hand container
			*/
			/*int x = msg_in.data.i32[0] - bogui_container_manager->win_off_x;
			int y = msg_in.data.i32[1] - bogui_container_manager->win_off_y;
			*/

			bogui_container_t *container = msg_in.data.ptr[0];	

			if (msg_in.data.i32[1] < BOGUI_TASKBAR_HEIGHT) {
				msg_in.data.i32[1] = BOGUI_TASKBAR_HEIGHT;
			} else if (msg_in.data.i32[1] > bogui_container_manager->height - BOGUI_WINDOW_BAR_HEIGHT) {
				msg_in.data.i32[1] = bogui_container_manager->height - BOGUI_WINDOW_BAR_HEIGHT;
			} 
			
			/*如果鼠标选中某个图层，然后弹起后，窗口移动到鼠标的位置*/
			bogui_container_slide(container , msg_in.data.i32[0], msg_in.data.i32[1]);

			//移动结束的时候才能刷新图层
			container->refresh_lock = 0;

			//移动窗口时，要聚焦
			bogui_taskbar_winctl_focus(bogui_container_manager->current_container->window->winctl);

		} else if (msg_in.id == BOGUI_MSG_WINDOW_FOUCS) {
			/*
			消息数据：
			ptr[0] = hand container
			*/

			/*如果容器是窗口， 把容器置顶,不管点击的容器的哪个地方*/
			bogui_container_axis(msg_in.data.ptr[0], bogui_container_manager->top - 1);
			//先把已有的聚焦窗口的标题栏变成未激活状态
			bogui_inactive_all_window();
			//把当前抓取的容器设置成激活状态
			bogui_window_active(msg_in.data.ptr[0]);
			//把当前容器指向抓取的这个容器
			bogui_container_manager->current_container = msg_in.data.ptr[0];

			//聚焦窗口后，要聚焦一个窗口控制
			bogui_taskbar_winctl_focus(bogui_container_manager->current_container->window->winctl);

			//printk("cur :%s \n", bogui_container_manager->current_container->window->title);

		} else if (msg_in.id == BOGUI_MSG_WINDOW_CLOSE) {
			printk("> close window! ");
			/*
			消息数据：
			ptr[0] = container		要关闭哪个容器
			
			i32[0] = type		关闭的类型，（强制还是自动）	
			
			*/
			
			bogui_container_t *container = msg_in.data.ptr[0];
			bogui_window_t *window = container->window;
			struct thread *thread = container->thread;
	
			if (window->guilib != NULL) {
				printk("1 ");	
				//释放图形库
				mm.free(window->guilib);
			}
			
			if (window != NULL) {
				printk("2 ");
				bogui_winctl_close(window->winctl);
			}
						
			/*
			关闭要操作的窗口容器
			*/
			//bogui_close_window(window);
			printk("con:%x name:%s\n",container, container->name);
			printk("win:%x title:%s\n",window, window->title);

			printk("thread:%x name:%s\n",thread, thread->name);

			printk("3 ");
			//printk("a:%x ", window->container->thread);
			//如果有进程
			if (thread != NULL) {
				printk("close thread :%s \n", thread->name);

				//根据类型做出不同的操作
				if (msg_in.data.i32[0] == BOGUI_WINBTN_CLOSE_FORCE) {
					//强制关闭窗口

					//关闭线程
					//thread_kill(thread);

					printk("force thread :%s \n", thread->name);
					//window->container->thread = NULL;
					
				} else if (msg_in.data.i32[0] == BOGUI_WINBTN_CLOSE_ACCORD) {
					//自动关闭窗口
					
					//让程序自己关闭
					printk("accord thread :%s \n", thread->name);

				}
			}
			printk("4 ");

			//激活最高层的容器
			container = bogui_get_topest_window();

			//当前容器指向新的容器
			if (container != NULL) {
				bogui_window_active(container);
				bogui_container_manager->current_container = container;
			} else {
				bogui_container_manager->current_container = buttom_container;
			}

			//关闭窗口后，会聚焦一个新的窗口，所以要聚焦一个窗口控制
			bogui_taskbar_winctl_focus(bogui_container_manager->current_container->window->winctl);
			printk("5\n");	
			//打印任务列表
			//print_threads();

		} else if (msg_in.id == BOGUI_MSG_WINDOW_ADD) {
			/*
			消息数据：
			ptr[0] = title	标题是申请的一个系统中的空间
			ptr[1] = caller thread	创建窗口请求者
	
			i32[0] = x
			i32[1] = y
			i32[2] = width
			i32[3] = height

			*/
		
			/*调整位置*/
			if (msg_in.data.i32[1] < BOGUI_TASKBAR_HEIGHT) {
				msg_in.data.i32[1] = BOGUI_TASKBAR_HEIGHT;
			} else if (msg_in.data.i32[1] > bogui_container_manager->height - BOGUI_WINDOW_BAR_HEIGHT) {
				msg_in.data.i32[1] = bogui_container_manager->height - BOGUI_WINDOW_BAR_HEIGHT;
			} 
			struct thread *thread = msg_in.data.ptr[1];
			//创建容器
			bogui_container_t *container = bogui_add_window(msg_in.data.ptr[0], msg_in.data.i32[0], msg_in.data.i32[1], \
				msg_in.data.i32[2], msg_in.data.i32[3]);
			if (container == NULL) {
				if (thread != NULL) {	
					thread->exit_status	= 0;
					//解除调用者的阻塞
					if (thread->status == THREAD_BLOCKED) {
						thread_unblock(thread);

					}
				}
				return;
			}
	
			//创建窗口控制
			bogui_winctl_t *winctl = bogui_winctl_add(msg_in.data.ptr[0]);
			if (winctl == NULL) {
				//错误就关闭窗口
				bogui_close_window(container->window);

				if (thread != NULL) {	
					thread->exit_status	= 0;				
					//解除调用者的阻塞
					if (thread->status == THREAD_BLOCKED) {
						thread_unblock(thread);
					}
				}
				return;
			}
			/*互相绑定*/
			bogui_winctl_bind_window(winctl, container->window);
			bogui_window_bind_winctl(container->window, winctl);

			//释放掉传送过来的标题
			mm.free(msg_in.data.ptr[0]);
			
			//如果说创建者有线程，我们就把它唤醒
			if (thread != NULL) {
				//绑定线程
				/*bogui_thread_bind_container(msg_in.data.ptr[1], container);
				bogui_container_bind_thread(container, msg_in.data.ptr[1]);*/

				/*
				线程通过guilib绑定container
				容器直接绑定线程
				*/
				bogui_guilib_bind_container(thread->guilib, container);
				bogui_container_bind_thread(container, thread);

				thread->exit_status	= 1;
				//解除调用者的阻塞
				if (thread->status == THREAD_BLOCKED) {
					thread_unblock(thread);
				}
			}
			/*
			发送一个改变窗口聚焦的消息
			然后会在里面把窗口控制聚焦
			*/
			/*msg_out.id = BOGUI_MSG_WINDOW_FOUCS;
			msg_out.data.ptr[0] = container;

			bogui_msg_send(&msg_out);
			*/

			/*如果容器是窗口， 把容器置顶,不管点击的容器的哪个地方*/
			bogui_container_axis(container, bogui_container_manager->top - 1);
			//先把已有的聚焦窗口的标题栏变成未激活状态
			bogui_inactive_all_window();
			//把当前抓取的容器设置成激活状态
			bogui_window_active(container);
			//把当前容器指向抓取的这个容器
			bogui_container_manager->current_container = container;

			//聚焦窗口后，要聚焦一个窗口控制
			bogui_taskbar_winctl_focus(bogui_container_manager->current_container->window->winctl);


		} else if (msg_in.id == BOGUI_MSG_WINDOW_HIDE) {
			/*
			消息数据：
			ptr[0] = hand container
			*/
			//先把已有的聚焦窗口的标题栏变成未激活状态
			bogui_inactive_all_window();

			bogui_container_t *container = msg_in.data.ptr[0];

			bogui_taskbar_winctl_cancel(container->window->winctl);

			/*容器隐藏*/
			bogui_container_axis(container, -1);
			
			//获取一个最高的窗口
			container = bogui_get_topest_window();

			//把当前抓取的容器设置成激活状态
			bogui_window_active(container);
			//把当前容器指向抓取的这个容器
			bogui_container_manager->current_container = container;

			//隐藏一个窗口后，可能还有其它窗口在显示，所以要聚焦一个窗口控制
			bogui_taskbar_winctl_focus(bogui_container_manager->current_container->window->winctl);

		} else if (msg_in.id == BOGUI_MSG_WINDOW_SHOW) {
			/*
			消息数据：
			ptr[0] = hand container
			*/

			/*如果容器是窗口， 把容器置顶,不管点击的容器的哪个地方*/
			bogui_container_axis(msg_in.data.ptr[0], bogui_container_manager->top - 1);
			//先把已有的聚焦窗口的标题栏变成未激活状态
			bogui_inactive_all_window();

			//不知道为什么要再设置一次才可以将它显示在最上面，可能第一次调整位置高度，第二次整体提升高度
			bogui_container_axis(msg_in.data.ptr[0], bogui_container_manager->top - 1);

			//把当前抓取的容器设置成激活状态
			bogui_window_active(msg_in.data.ptr[0]);

			//把当前容器指向抓取的这个容器
			bogui_container_manager->current_container = msg_in.data.ptr[0];
		
			//显示一个窗口后，要把这个窗口对应的窗口控制聚焦
			bogui_taskbar_winctl_focus(bogui_container_manager->current_container->window->winctl);
		} else if (msg_in.id == BOGUI_MSG_WINDOW_TOPEST) {	//把最顶层激活
			/*
			消息数据：
			NONE
			*/
			//激活最高层的容器
			bogui_container_t *container = bogui_get_topest_window();

			//当前容器指向新的容器
			if (container != NULL) {
				bogui_window_active(container);
				bogui_container_manager->current_container = container;
			} else {
				bogui_container_manager->current_container = buttom_container;
			}

			//关闭窗口后，会聚焦一个新的窗口，所以要聚焦一个窗口控制
			bogui_taskbar_winctl_focus(bogui_container_manager->current_container->window->winctl);
				
		}

	}
	
}

void bogui_mouse_move_listen(int mousex, int mousey)
{
	int j;
	bogui_container_t *container;

	//循环检测每一个容器
	for(j = bogui_container_manager->top - 1; j >= 0; j--){
		container = bogui_container_manager->container_addr[j];
		//有鼠标监听就跳转到鼠标监听
		if (container->mouse_move_listen != NULL) {
			//传递鼠标在容器上的偏移
			container->mouse_move_listen(container, mousex -container->x, mousey - container->y);
		}
	}
}

void bogui_mouse_click_listen( int mousex, int mousey, int key, int status)
{
	int j;
	bogui_container_t *container;
	//循环检测每一个容器
	for(j = bogui_container_manager->top - 1; j >= 0; j--){
		container = bogui_container_manager->container_addr[j];
		//有鼠标监听就跳转到鼠标监听, 如果是聚焦的容器才会去执行
		if (container->mouse_click_listen != NULL) {
			//传递鼠标在容器上的偏移
			container->mouse_click_listen(container, mousex -container->x, mousey - container->y, key, status);
		}
	}
}

#endif //_CONFIG_GUI_BOGUI_
