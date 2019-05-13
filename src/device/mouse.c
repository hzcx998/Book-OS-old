/*
File:		device/mouse.c
Contains:	driver program for mouse
Auther:		Hu Zicheng
Time:		2019/2/12
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_MOUSE_

#include <sys/arch.h>
#include <sys/dev.h>
#include <sys/mm.h>
#include <sys/gui.h>
#include <sys/core.h>
#include <string.h>

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

struct mouse mouse;

#ifdef _CONFIG_GUI_BOGUI_
bogui_message_t mouse_gmsg_out;
bogui_message_t gui_msg_mouse;

#endif //_CONFIG_GUI_BOGUI_ 
/*
extern struct task *task_mouse;
extern struct layer *layer_desktop;
*/
void init_mouse()
{
	printk("> init mouse start.\n");
	
	//鼠标信息初始化
	mouse.old_x = mouse.x = video_info.width/2;
	mouse.old_y = mouse.y = video_info.height/2;

	mouse.key_left = MOUSE_IDLE;
	mouse.key_right = MOUSE_IDLE;
	mouse.key_middle = MOUSE_IDLE;
	
	mouse.key_left_continue = MOUSE_IDLE;
	mouse.key_right_continue = MOUSE_IDLE;
	mouse.key_middle_continue = MOUSE_IDLE;

	mouse.key_occur = 0;
	mouse.status = MOUSE_NORMAL;
	
	mouse.ioqueue = create_ioqueue();
	ioqueue_init(mouse.ioqueue);
	
	//初始化鼠标中断
	put_irq_handler(MOUSE_IRQ, mouse_handler);
	//打开从片
	enable_irq(CASCADE_IRQ);
	//打开鼠标中断
	enable_irq(MOUSE_IRQ);
	//激活鼠标前要设定键盘相关信息
	enable_mouse(&mouse);
	
	printk("< init mouse done.\n");
	
}

void thread_mouse(void *arg)
{
	//printk("running in mouse\n");
	thread_bus.mouse = 1;
	
	while(1){
		int data = ioqueue_get(mouse.ioqueue);
		if(mouse_read(&mouse, data) != 0) {
		
			//对原始数据操作
			mouse.x += mouse.x_increase;
			mouse.y += mouse.y_increase;
			
			if (mouse.x < 0) {
				mouse.x = 0;
			}
			if (mouse.y < 0) {
				mouse.y = 0;
			}

			if (mouse.x > video_info.width - 1) {
				mouse.x = video_info.width - 1;
			}
			if (mouse.y >= video_info.height - 1) {
				mouse.y = video_info.height - 1;
			}

			mouse.active = 0;

			if (mouse.x_increase || mouse.y_increase) {
				
				mouse.active = 1;
			}
			
			if((mouse.button & 0x01) != 0 ){
				mouse.key_left = MOUSE_DOWN;
					
			}else{
				if(mouse.key_left == MOUSE_DOWN){
					mouse.key_left = MOUSE_UP;
				}else{
					mouse.key_left = MOUSE_IDLE;
				}
			}
			
			if((mouse.button & 0x02) != 0){
				mouse.key_right = MOUSE_DOWN;
				
			}else{
				if(mouse.key_right == MOUSE_DOWN){
					mouse.key_right = MOUSE_UP;
				}else{
					mouse.key_right = MOUSE_IDLE;
				}
			}
			
			if((mouse.button & 0x04) != 0){
				mouse.key_middle = MOUSE_DOWN;
				
			}else{
				
				if(mouse.key_middle == MOUSE_DOWN){
					mouse.key_middle = MOUSE_UP;
				}else{
					mouse.key_middle = MOUSE_IDLE;
				}
			}

			//以上对原始数据进行了处理，接下来把原始数据应用到图形界面，发送消息过去
			#ifdef _CONFIG_GUI_BOGUI_
				/*
				鼠标左键按下后，需要去执行的操作
				*/
				if(mouse.key_left== MOUSE_DOWN){
					/*获取鼠标位置的容器
					没有选择容器的时候才能选择新的容器，如果选择了容器，只有等鼠标弹起后才能选择新的容器。
					*/
					//可以获取容器
					if (bogui_container_manager->can_hand_container) {
						//获取一个最高的容器，或者获取不到容器
						bogui_container_manager->hand_container = bogui_get_container_where_mouse_on(mouse.x, mouse.y);
						//如果获取到了容器
						//判断是什么类型的容器
						if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_BUTTOM) {
							//是最底层容器（桌面）
							//先把已有的聚焦窗口的标题栏变成未激活状态
							bogui_inactive_all_window();
							bogui_container_manager->current_container = buttom_container;
								
							/*当点击桌面的时候，没有窗口聚焦，所以要把窗口控制也设置一下*/
							bogui_taskbar_winctl_cancel_all();

							//鼠标监听在底层容器上发生的点击事件
							bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_DOWN);
								
						} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_WINDOW) {
							//是窗口
							//如果选中的和当前的不一样，才改变窗口聚焦
							if (bogui_container_manager->hand_container != bogui_container_manager->current_container) {
								
								/*先发送一个改变窗口聚焦的消息*/
								mouse_gmsg_out.id = BOGUI_MSG_WINDOW_FOUCS;
								mouse_gmsg_out.data.ptr[0] = bogui_container_manager->hand_container;

								bogui_msg_send(&mouse_gmsg_out);
							}
							//是在标题栏还是在活动区域？

							//获取鼠标简化位置
							bogui_container_manager->mouse_off_x = mouse.x - bogui_container_manager->hand_container->x;		//鼠标位置和图层位置的距离
							bogui_container_manager->mouse_off_y = mouse.y - bogui_container_manager->hand_container->y;		//鼠标位置和图层位置的距离
								
							//鼠标监听在窗口容器上发生的点击事件
							bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_DOWN);
							
							//窗口移动处理，如果没有点击窗口上的按钮
							if (bogui_container_manager->moveing_container == NULL && bogui_container_manager->can_hand_container) {

								if (0 <= bogui_container_manager->mouse_off_x && \
									bogui_container_manager->mouse_off_x < bogui_container_manager->hand_container->width && \
									0 <= bogui_container_manager->mouse_off_y && \
									bogui_container_manager->mouse_off_y < bogui_container_manager->hand_container->window->frame.y) {
										
									//记录窗口的偏移
									bogui_container_manager->win_off_x = bogui_container_manager->mouse_off_x;
									bogui_container_manager->win_off_y = bogui_container_manager->mouse_off_y;

									//记录上次鼠标的位置					
									bogui_container_manager->last_mouse_x = mouse.x;
									bogui_container_manager->last_mouse_y = mouse.y;
										
									//如果在标题栏之内，就可以移动, 设定移动的窗口
									bogui_container_manager->moveing_container = bogui_container_manager->hand_container;
								}
							}
							/*
							点击后就不能继续监测其它的点击，必须等待下一次点击
							用于避免几个窗口叠加在一起，最上层操作完后会监测下面一层，
							从而又会执行类似的操作
							*/
							bogui_container_manager->can_hand_container = 0;

							//点击一个窗口后，把它的窗口控制激活
							bogui_taskbar_winctl_focus(bogui_container_manager->hand_container->window->winctl);

						} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FIXED) {
							//固定的容器（任务栏）
							//先把已有的聚焦窗口的标题栏变成未激活状态
							//fixed是不接受键盘的，其它的都要接受键盘
							bogui_inactive_all_window();

							bogui_container_manager->current_container = bogui_taskbar.container;

							/*
							当有鼠标点击后，就监听
							如果发现没有点击到某个winctl，就会把已经激活的winctl取消，就达到了
							点击容器的其它地方使得已经激活的winctl取消激活
							*/
							bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_DOWN);
							
						} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FLOAT) {
							//悬浮窗（弹出菜单）

						}
						
					} else {
						
					}
				/*
				鼠标左键弹起后，需要去执行的操作
				*/
				} else if (mouse.key_left == MOUSE_UP) {

					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_UP);
					
					/*鼠标弹起后，如果有窗口处于移动中，发送移动窗口消息*/
					if (bogui_container_manager->moveing_container != NULL) {

						//如果位置改变了才移动窗口
						if (bogui_container_manager->last_mouse_x != mouse.x || \
							bogui_container_manager->last_mouse_y != mouse.y) 
						{
							mouse_gmsg_out.id = BOGUI_MSG_WINDOW_MOVE;

							//传输窗口的位置
							mouse_gmsg_out.data.i32[0] = mouse.x - bogui_container_manager->win_off_x;
							mouse_gmsg_out.data.i32[1] = mouse.y - bogui_container_manager->win_off_y;

							//移动的时候不能刷新图层
							bogui_container_manager->moveing_container->refresh_lock = 1;

							mouse_gmsg_out.data.ptr[0] = bogui_container_manager->moveing_container;

							bogui_msg_send(&mouse_gmsg_out);
						}
						
						bogui_container_manager->moveing_container = NULL;
						
					}
					bogui_container_manager->can_hand_container = 1;

				}

				/*----------------------------华丽的分割线-------------------*/

				/*
				鼠标右键按下后，需要去执行的操作
				*/
				if (mouse.key_right == MOUSE_DOWN) {

					//获取一个最高的容器，或者获取不到容器
					bogui_container_manager->hand_container = bogui_get_container_where_mouse_on(mouse.x, mouse.y);
					//如果获取到了容器
					//判断是什么类型的容器
					if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_BUTTOM) {
						//是最底层容器（桌面）
						//先把已有的聚焦窗口的标题栏变成未激活状态
						bogui_inactive_all_window();
						bogui_container_manager->current_container = buttom_container;
						
						/*当点击桌面的时候，没有窗口聚焦，所以要把窗口控制也设置一下*/
						bogui_taskbar_winctl_cancel_all();

						//鼠标监听在底层容器上发生的点击事件
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_DOWN);
						
						/*打开弹出窗口*/
					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_WINDOW) {
						//是窗口
						//如果选中的和当前的不一样，才改变窗口聚焦
						if (bogui_container_manager->hand_container != bogui_container_manager->current_container) {
							
							/*先发送一个改变窗口聚焦的消息*/
							mouse_gmsg_out.id = BOGUI_MSG_WINDOW_FOUCS;
							mouse_gmsg_out.data.ptr[0] = bogui_container_manager->hand_container;

							bogui_msg_send(&mouse_gmsg_out);
						}
						//是在标题栏还是在活动区域？

						//鼠标监听在窗口容器上发生的点击事件
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_DOWN);
						
					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FIXED) {
						//固定的容器（任务栏）
						//先把已有的聚焦窗口的标题栏变成未激活状态
						//fixed是不接受键盘的，其它的都要接受键盘
						bogui_inactive_all_window();

						bogui_container_manager->current_container = bogui_taskbar.container;

						/*
						当有鼠标点击后，就监听
						如果发现没有点击到某个winctl，就会把已经激活的winctl取消，就达到了
						点击容器的其它地方使得已经激活的winctl取消激活
						*/
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_DOWN);
						
					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FLOAT) {
						//悬浮窗（弹出菜单）

					}
				
				} else if (mouse.key_right == MOUSE_UP) {
			
					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_UP);
					
				}
				/*
				鼠标中键按下后，需要去执行的操作
				*/
				if (mouse.key_middle == MOUSE_DOWN) {
					//获取一个最高的容器，或者获取不到容器
					bogui_container_manager->hand_container = bogui_get_container_where_mouse_on(mouse.x, mouse.y);
					//如果获取到了容器
					//判断是什么类型的容器
					if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_BUTTOM) {
						//是最底层容器（桌面）
						//先把已有的聚焦窗口的标题栏变成未激活状态
						bogui_inactive_all_window();
						bogui_container_manager->current_container = buttom_container;
						
						/*当点击桌面的时候，没有窗口聚焦，所以要把窗口控制也设置一下*/
						bogui_taskbar_winctl_cancel_all();

						//鼠标监听在底层容器上发生的点击事件
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_DOWN);
						
						/*什么也不做*/

					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_WINDOW) {
						//是窗口
						//如果选中的和当前的不一样，才改变窗口聚焦
						if (bogui_container_manager->hand_container != bogui_container_manager->current_container) {
							
							/*先发送一个改变窗口聚焦的消息*/
							mouse_gmsg_out.id = BOGUI_MSG_WINDOW_FOUCS;
							mouse_gmsg_out.data.ptr[0] = bogui_container_manager->hand_container;

							bogui_msg_send(&mouse_gmsg_out);
						}
						//是在标题栏还是在活动区域？
						
						//鼠标监听在窗口容器上发生的点击事件
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_DOWN);
						
					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FIXED) {
						//固定的容器（任务栏）
						//先把已有的聚焦窗口的标题栏变成未激活状态
						//fixed是不接受键盘的，其它的都要接受键盘
						bogui_inactive_all_window();

						bogui_container_manager->current_container = bogui_taskbar.container;

						//bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_DOWN);
						
					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FLOAT) {
						//悬浮窗（弹出菜单）

					}
					
				} else if (mouse.key_middle == MOUSE_UP) {
			
					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_UP);

				}
				/*
				有鼠标位置改变消息，就要移动
				*/
				if (mouse.active) {
					//移动图层
					/*
					发送鼠标移动消息
					*/
					mouse_gmsg_out.id = BOGUI_MSG_MOUSE_MOVE;
					mouse_gmsg_out.data.i32[0]	= mouse.x;
					mouse_gmsg_out.data.i32[1]	= mouse.y;

					mouse_gmsg_out.data.ptr[0]	= top_container;

					bogui_msg_send(&mouse_gmsg_out);

					//如果没有抓取容器才检测
					if (bogui_container_manager->can_hand_container) {
						//执行移动监听事件
						bogui_mouse_move_listen(mouse.x, mouse.y);
					}
				}
			#endif //_CONFIG_GUI_BOGUI_	
		}
	}
}

void enable_mouse(struct mouse *mouse)
{
	/* 激活鼠标 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/*键盘控制器返回ACK(0xfa)*/
	mouse->phase = 0;
	
	return;
}

void mouse_handler(int32_t irq)
{
	uint8_t data;
	data = io_in8(PORT_KEYDAT);
	ioqueue_put(mouse.ioqueue, data);
}

/*
put button info in buf
buf[0]:left
buf[1]:middle
buf[2]:right
*/

void sys_get_mouse_button(char buf[])
{
	buf[0] = mouse.key_left;
	buf[1] = mouse.key_middle;
	buf[2] = mouse.key_right;
}

/*put x, y in buf*/
int sys_get_mouse_position(int *x, int *y)
{
	/*type:0 获取相对于屏幕的坐标*/
	*x = mouse.x;
	*y = mouse.y;
	return 1;
}

int32_t get_mouse_x()
{
	return mouse.x;
}
int32_t get_mouse_y()
{
	return mouse.y;
}

void mouse_analysis()
{

	
	//	}












		/*
		gui_msg_mouse.data.i32[4] = mouse.key_middle;
		
		gui_msg_mouse.id = BOGUI_MSG_MOUSE_EVEN;
		bogui_msg_send(&gui_msg_mouse);
*/
		//以上对原始数据进行了处理，接下来把原始数据应用到图形界面，发送消息过去
		#ifdef _CONFIG_GUI_BOGUI2_
			/*
			鼠标左键按下后，需要去执行的操作
			*/
			if(mouse.key_left== MOUSE_DOWN){
				/*获取鼠标位置的容器
				没有选择容器的时候才能选择新的容器，如果选择了容器，只有等鼠标弹起后才能选择新的容器。
				*/
				//可以获取容器
				if (bogui_container_manager->can_hand_container) {
					//获取一个最高的容器，或者获取不到容器
					bogui_container_manager->hand_container = bogui_get_container_where_mouse_on(mouse.x, mouse.y);
					//如果获取到了容器
					//判断是什么类型的容器
					if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_BUTTOM) {
						//是最底层容器（桌面）
						//先把已有的聚焦窗口的标题栏变成未激活状态
						bogui_inactive_all_window();
						bogui_container_manager->current_container = buttom_container;
							
						/*当点击桌面的时候，没有窗口聚焦，所以要把窗口控制也设置一下*/
						bogui_taskbar_winctl_cancel_all();

						//鼠标监听在底层容器上发生的点击事件
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_DOWN);
							
					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_WINDOW) {
						//是窗口
						//如果选中的和当前的不一样，才改变窗口聚焦
						if (bogui_container_manager->hand_container != bogui_container_manager->current_container) {
								
							/*先发送一个改变窗口聚焦的消息*/
							mouse_gmsg_out.id = BOGUI_MSG_WINDOW_FOUCS;
							mouse_gmsg_out.data.ptr[0] = bogui_container_manager->hand_container;

							bogui_msg_send(&mouse_gmsg_out);
						}
						//是在标题栏还是在活动区域？

						//获取鼠标简化位置
						bogui_container_manager->mouse_off_x = mouse.x - bogui_container_manager->hand_container->x;		//鼠标位置和图层位置的距离
						bogui_container_manager->mouse_off_y = mouse.y - bogui_container_manager->hand_container->y;		//鼠标位置和图层位置的距离
							
						//鼠标监听在窗口容器上发生的点击事件
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_DOWN);
						
						//窗口移动处理，如果没有点击窗口上的按钮
						if (bogui_container_manager->moveing_container == NULL && bogui_container_manager->can_hand_container) {

							if (0 <= bogui_container_manager->mouse_off_x && \
								bogui_container_manager->mouse_off_x < bogui_container_manager->hand_container->width && \
								0 <= bogui_container_manager->mouse_off_y && \
								bogui_container_manager->mouse_off_y < bogui_container_manager->hand_container->window->frame.y) {
									
								//记录窗口的偏移
								bogui_container_manager->win_off_x = bogui_container_manager->mouse_off_x;
								bogui_container_manager->win_off_y = bogui_container_manager->mouse_off_y;

								//记录上次鼠标的位置					
								bogui_container_manager->last_mouse_x = mouse.x;
								bogui_container_manager->last_mouse_y = mouse.y;
									
								//如果在标题栏之内，就可以移动, 设定移动的窗口
								bogui_container_manager->moveing_container = bogui_container_manager->hand_container;
							}
						}
						/*
						点击后就不能继续监测其它的点击，必须等待下一次点击
						用于避免几个窗口叠加在一起，最上层操作完后会监测下面一层，
						从而又会执行类似的操作
						*/
						bogui_container_manager->can_hand_container = 0;

						//点击一个窗口后，把它的窗口控制激活
						bogui_taskbar_winctl_focus(bogui_container_manager->hand_container->window->winctl);

					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FIXED) {
						//固定的容器（任务栏）
						//先把已有的聚焦窗口的标题栏变成未激活状态
						//fixed是不接受键盘的，其它的都要接受键盘
						bogui_inactive_all_window();

						bogui_container_manager->current_container = bogui_taskbar.container;

						/*
						当有鼠标点击后，就监听
						如果发现没有点击到某个winctl，就会把已经激活的winctl取消，就达到了
						点击容器的其它地方使得已经激活的winctl取消激活
						*/
						bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_DOWN);
						
					} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FLOAT) {
						//悬浮窗（弹出菜单）

					}
					
				} else {
					
				}
			/*
			鼠标左键弹起后，需要去执行的操作
			*/
			} else if (mouse.key_left == MOUSE_UP) {

				bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_LEFT, MOUSE_UP);
				
				/*鼠标弹起后，如果有窗口处于移动中，发送移动窗口消息*/
				if (bogui_container_manager->moveing_container != NULL) {

					//如果位置改变了才移动窗口
					if (bogui_container_manager->last_mouse_x != mouse.x || \
						bogui_container_manager->last_mouse_y != mouse.y) 
					{
						mouse_gmsg_out.id = BOGUI_MSG_WINDOW_MOVE;

						//传输窗口的位置
						mouse_gmsg_out.data.i32[0] = mouse.x - bogui_container_manager->win_off_x;
						mouse_gmsg_out.data.i32[1] = mouse.y - bogui_container_manager->win_off_y;

						//移动的时候不能刷新图层
						bogui_container_manager->moveing_container->refresh_lock = 1;

						mouse_gmsg_out.data.ptr[0] = bogui_container_manager->moveing_container;

						bogui_msg_send(&mouse_gmsg_out);
					}
					
					bogui_container_manager->moveing_container = NULL;
					
				}
				bogui_container_manager->can_hand_container = 1;

			}
			/*
			鼠标右键按下后，需要去执行的操作
			*/
			if (mouse.key_right == MOUSE_DOWN) {

				//获取一个最高的容器，或者获取不到容器
				bogui_container_manager->hand_container = bogui_get_container_where_mouse_on(mouse.x, mouse.y);
				//如果获取到了容器
				//判断是什么类型的容器
				if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_BUTTOM) {
					//是最底层容器（桌面）
					//先把已有的聚焦窗口的标题栏变成未激活状态
					bogui_inactive_all_window();
					bogui_container_manager->current_container = buttom_container;
					
					/*当点击桌面的时候，没有窗口聚焦，所以要把窗口控制也设置一下*/
					bogui_taskbar_winctl_cancel_all();

					//鼠标监听在底层容器上发生的点击事件
					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_DOWN);
					
					/*打开弹出窗口*/
				} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_WINDOW) {
					//是窗口
					//如果选中的和当前的不一样，才改变窗口聚焦
					if (bogui_container_manager->hand_container != bogui_container_manager->current_container) {
						
						/*先发送一个改变窗口聚焦的消息*/
						mouse_gmsg_out.id = BOGUI_MSG_WINDOW_FOUCS;
						mouse_gmsg_out.data.ptr[0] = bogui_container_manager->hand_container;

						bogui_msg_send(&mouse_gmsg_out);
					}
					//是在标题栏还是在活动区域？

					//鼠标监听在窗口容器上发生的点击事件
					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_DOWN);
					
				} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FIXED) {
					//固定的容器（任务栏）
					//先把已有的聚焦窗口的标题栏变成未激活状态
					//fixed是不接受键盘的，其它的都要接受键盘
					bogui_inactive_all_window();

					bogui_container_manager->current_container = bogui_taskbar.container;

					/*
					当有鼠标点击后，就监听
					如果发现没有点击到某个winctl，就会把已经激活的winctl取消，就达到了
					点击容器的其它地方使得已经激活的winctl取消激活
					*/
					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_DOWN);
					
				} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FLOAT) {
					//悬浮窗（弹出菜单）

				}
			
			} else if (mouse.key_right == MOUSE_UP) {
		
				bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_RIGHT, MOUSE_UP);
				
			}
			/*
			鼠标中键按下后，需要去执行的操作
			*/
			if (mouse.key_middle == MOUSE_DOWN) {
				execv("c:/bosh", NULL);
				//获取一个最高的容器，或者获取不到容器
				bogui_container_manager->hand_container = bogui_get_container_where_mouse_on(mouse.x, mouse.y);
				//如果获取到了容器
				//判断是什么类型的容器
				if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_BUTTOM) {
					//是最底层容器（桌面）
					//先把已有的聚焦窗口的标题栏变成未激活状态
					bogui_inactive_all_window();
					bogui_container_manager->current_container = buttom_container;
					
					/*当点击桌面的时候，没有窗口聚焦，所以要把窗口控制也设置一下*/
					bogui_taskbar_winctl_cancel_all();

					//鼠标监听在底层容器上发生的点击事件
					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_DOWN);
					
					/*什么也不做*/

				} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_WINDOW) {
					//是窗口
					//如果选中的和当前的不一样，才改变窗口聚焦
					if (bogui_container_manager->hand_container != bogui_container_manager->current_container) {
						
						/*先发送一个改变窗口聚焦的消息*/
						mouse_gmsg_out.id = BOGUI_MSG_WINDOW_FOUCS;
						mouse_gmsg_out.data.ptr[0] = bogui_container_manager->hand_container;

						bogui_msg_send(&mouse_gmsg_out);
					}
					//是在标题栏还是在活动区域？
					
					//鼠标监听在窗口容器上发生的点击事件
					bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_DOWN);
					
				} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FIXED) {
					//固定的容器（任务栏）
					//先把已有的聚焦窗口的标题栏变成未激活状态
					//fixed是不接受键盘的，其它的都要接受键盘
					bogui_inactive_all_window();

					bogui_container_manager->current_container = bogui_taskbar.container;

					//bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_DOWN);
					
				} else if (bogui_container_manager->hand_container->flags&BOGUI_CONTAINER_FLOAT) {
					//悬浮窗（弹出菜单）

				}
				
			} else if (mouse.key_middle == MOUSE_UP) {
		
				bogui_mouse_click_listen(mouse.x, mouse.y, MOUSE_MIDDLE, MOUSE_UP);

			}
			/*
			有鼠标位置改变消息，就要移动
			*/
			if (mouse.active) {
				//移动图层
				//bogui_container_slide(top_container, mouse.x, msg_in.data.i32[1]);
				/*
				发送鼠标移动消息
				*/
				mouse_gmsg_out.id = BOGUI_MSG_MOUSE_MOVE;
				mouse_gmsg_out.data.i32[0]	= mouse.x;
				mouse_gmsg_out.data.i32[1]	= mouse.y;

				mouse_gmsg_out.data.ptr[0]	= top_container;

				bogui_msg_send(&mouse_gmsg_out);
				//如果没有抓取容器才检测
				if (bogui_container_manager->can_hand_container) {
					//执行移动监听事件
					bogui_mouse_move_listen(mouse.x, mouse.y);
				}
			}
		#endif //_CONFIG_GUI_BOGUI_	
	//}
}

int mouse_read(struct mouse *mouse, unsigned char data)
{
	if (mouse->phase == 0) {
		if (data == 0xfa) {
			mouse->phase = 1;
		}
		return 0;
	}
	if (mouse->phase == 1) {
		if ((data & 0xc8) == 0x08) {
			mouse->read_buf[0] = data;
			mouse->phase = 2;
		}
		return 0;
	}
	if (mouse->phase == 2) {
		mouse->read_buf[1] = data;
		mouse->phase = 3;
		return 0;
	}
	if (mouse->phase == 3) {
		mouse->read_buf[2] = data;
		mouse->phase = 1;
		mouse->button = mouse->read_buf[0] & 0x07;
		mouse->x_increase = mouse->read_buf[1];
		mouse->y_increase = mouse->read_buf[2];
		if ((mouse->read_buf[0] & 0x10) != 0) {
			mouse->x_increase |= 0xffffff00;
		}
		if ((mouse->read_buf[0] & 0x20) != 0) {
			mouse->y_increase |= 0xffffff00;
		}
		mouse->y_increase = - mouse->y_increase;
		return 1;
	}
	return -1; 
}


#endif //_CONFIG_DEVIVE_MOUSE_


