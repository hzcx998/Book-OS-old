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

#include <sys/gui.h>
#include <sys/mm.h>
#include <sys/core.h>
#include <sys/debug.h>
#include <string.h>

bogui_container_manager_t *bogui_container_managerr_init(uint8_t *vram, int width, int height)
{
	bogui_container_manager_t *container_manager;
	container_manager = (bogui_container_manager_t *) mm.malloc(sizeof(bogui_container_manager_t));
	if (container_manager == NULL) {
		panic("bogui: alloc mem for container manager failed!\n");
		
	}

	//map只需要1字节就够可
	container_manager->map = (uint8_t *)mm.malloc(width * height);
	if(container_manager->map == NULL){
		mm.free(container_manager);
		panic("bogui: alloc mem for container manager map failed!\n");

	}

	container_manager->vram = vram;
	container_manager->width = width;
	container_manager->height = height;
	container_manager->top = -1;		//no container

	int i;
	int buf_size;
	for(i = 0; i < BOGUI_MAX_CONTAINER_NR; i++){
		container_manager->container_table[i].id = i;

		//根据等级分配不同大小
		if (0 <= i && i < 10) {
			buf_size = BOGUI_CONTAINER_LEVEL_1;
		} else if (10 <= i && i < 20) {
			buf_size = BOGUI_CONTAINER_LEVEL_2;
		} else if (20 <= i && i < 30) {
			buf_size = BOGUI_CONTAINER_LEVEL_3;
		} else if (30 <= i && i < 40) {
			buf_size = BOGUI_CONTAINER_LEVEL_4;
		} else if (40 <= i && i < 50) {
			buf_size = BOGUI_CONTAINER_LEVEL_5;
		} 

		//初始化buf并且设定size
		container_manager->container_table[i].buffer_size = buf_size;
		container_manager->container_table[i].buffer = (uint8_t *)mm.malloc(buf_size);

		container_manager->container_table[i].flags = BOGUI_CONTAINER_UNUSED;

	}
	
	return container_manager;
}

bogui_container_t *bogui_container_alloc(int size)
{
	bogui_container_t *container;
	int i;
	//从前往后查找
	for(i = 0; i < BOGUI_MAX_CONTAINER_NR; i++){
		container = &bogui_container_manager->container_table[i];
		if(container->flags == BOGUI_CONTAINER_UNUSED){
			//如果分配的size比设定的小，就说明找到了
			if (size <= container->buffer_size) {
				
				container->flags |= BOGUI_CONTAINER_USING;
				return container;
			}
		}
	}
	return NULL;	//分配失败，所有容器都在使用
}

void bogui_container_free(bogui_container_t *container)
{
	int i;
	for(i = 0; i < BOGUI_MAX_CONTAINER_NR; i++){
		if(&bogui_container_manager->container_table[i] == container){
			if(container->axis >= 0){
				bogui_container_axis(container, -1);	//隐藏
			}
			container->flags = BOGUI_CONTAINER_UNUSED;
			
			break;
		}
	}
}

void bogui_container_init(bogui_container_t *container, char *name, int x, int y, int width, int height, int bit_width)
{
	/*container->buffer = (uint8_t *)mm.malloc(width*height*bit_width);
	if (container->buffer == NULL) {
		return;
	}*/
	container->x = x;
	container->y = y;
	container->width = width;
	container->height = height;
	container->bit_width = bit_width;
	container->axis = -1;			//容器为隐藏状态
	
	container->refresh_lock = 0;

	//鼠标监听为空
	container->mouse_move_listen = NULL;
	container->mouse_click_listen = NULL;
	container->keyboard_listen = NULL;
	
	container->thread = NULL;
	container->window = NULL;
	

	memset(container->name, 0, BOGUI_CONTAINER_NAME_LEN);
	strcpy(container->name, name);
}

void bogui_container_bind_listen(bogui_container_t *container, int type, void (*func))
{
	if (type == GOGUI_MOUSE_MOVE_LISTEN) {
		container->mouse_move_listen = func;
	} else if (type == GOGUI_MOUSE_CLICK_LISTEN) {
		container->mouse_click_listen = func;
	} else if (type == GOGUI_KEYBOARD_LISTEN) {
		container->keyboard_listen = func;
	}
}

void bogui_container_bind_thread(bogui_container_t *container, struct thread *thread)
{
	container->thread = thread;
}
/*
获取一个axis大于0的容器
如果不相等就返回空
*/
bogui_container_t *bogui_axis_get_container(int axis)
{
	int i;
	for(i = 0; i < BOGUI_MAX_CONTAINER_NR; i++){
		if(bogui_container_manager->container_table[i].axis == axis){
			
			return &bogui_container_manager->container_table[i];
		}
	}
	return NULL;
}

/*
带透明度的窗口管理系统
不需要bogui_container_refresh_map，在里面直接返回
*/
void bogui_container_refresh_buffer2(int x0, int y0, int x1, int y1, int h0, int h1)
{
	int h, bx, by, x, y, bx0, by0, bx1, by1;	//x,y是容器在屏幕中的位置

	bogui_container_t *container;

	rgba_t src_color;
	rgba_t *dst_color;
	uint8_t alpha;

	if(x0 < 0){x0 = 0;}
	if(y0 < 0){y0 = 0;}
	if(x1 > bogui_container_manager->width){x1 = bogui_container_manager->width;}
	if(y1 > bogui_container_manager->height){y1 = bogui_container_manager->height;}
	
	/*像素读写得信息*/
	//把颜色写入buffer
	video_message_t wrmsg;
	wrmsg.m_type = VIDEO_MSG_WRITE_PIXEL;

	//从buffer里面读取颜色
	video_message_t rdmsg;
	rdmsg.m_type = VIDEO_MSG_READ_PIXEL;
	
	for(h = 0; h <= bogui_container_manager->top; h++){
		//获取容器
		container = bogui_container_manager->container_addr[h];

		bx0 = x0 - container->x;
		by0 = y0 - container->y;
		bx1 = x1 - container->x;
		by1 = y1 - container->y;
		if(bx0 < 0){bx0 = 0;}
		if(by0 < 0){by0 = 0;}
		if(bx1 > container->width){bx1 = container->width;}
		if(by1 > container->height){by1 = container->height;}

		//循环写入数据
		for(by = by0; by < by1; by++){	//height*2才能写完全部，不然只有一半
			y = container->y + by;

			for(bx = bx0; bx < bx1; bx++){
				x = container->x + bx;
				//照着map中的关系进行刷新				
				//if(map[(y * bogui_container_manager->width + x)] == container_id){
					//获取源颜色
					bogui_container_read_pixel(container, bx, by, (uint32_t *)&src_color);

					//获取buf颜色
					rdmsg.m_x = x;
					rdmsg.m_y = y;
					//传输给buffer，获取颜色
					video_transmit_to_buffer(&rdmsg);

					//获取目标颜色
					dst_color = (rgba_t *)&rdmsg.m_color;

					alpha = src_color.alpha;

					dst_color->red = src_color.red*alpha/0xff + dst_color->red*(1-alpha/0xFF * alpha/0xFF);
					dst_color->green = src_color.green*alpha/0xff + dst_color->green*(1-alpha/0xFF * alpha/0xFF);
					dst_color->blue = src_color.blue*alpha/0xff + dst_color->blue*(1-alpha/0xFF * alpha/0xFF);
					dst_color->alpha = src_color.alpha*alpha/0xff + dst_color->alpha*(1-alpha/0xFF * alpha/0xFF);
					
					wrmsg.m_color = rdmsg.m_color;

					//写像素
					wrmsg.m_x = x;
					wrmsg.m_y = y;

					//传输给buffer
					video_transmit_to_buffer(&wrmsg);
				//}
			}
		}
	}
	video_refresh_position(x0, y0, x1, y1);
}

/*
不带透明度的窗口管理系统
bogui_container_refresh_map
*/
static void bogui_container_refresh_buffer(int x0, int y0, int x1, int y1, int h0, int h1)
{
	int h, bx, by, x, y, bx0, by0, bx1, by1;	//x,y是容器在屏幕中的位置

	uint8_t *map = bogui_container_manager->map;

	bogui_container_t *container;
	
	uint8_t container_id;

	rgba_t color;

	if(x0 < 0){x0 = 0;}
	if(y0 < 0){y0 = 0;}
	if(x1 > bogui_container_manager->width){x1 = bogui_container_manager->width;}
	if(y1 > bogui_container_manager->height){y1 = bogui_container_manager->height;}
	
	/*像素读写得信息*/
	video_message_t wrmsg;

	wrmsg.m_type = VIDEO_MSG_WRITE_PIXEL;

	for(h = h0; h <= h1; h++){
		//获取容器
		container = bogui_container_manager->container_addr[h];

		container_id = container - bogui_container_manager->container_table;

		bx0 = x0 - container->x;
		by0 = y0 - container->y;
		bx1 = x1 - container->x;
		by1 = y1 - container->y;
		if(bx0 < 0){bx0 = 0;}
		if(by0 < 0){by0 = 0;}
		if(bx1 > container->width){bx1 = container->width;}
		if(by1 > container->height){by1 = container->height;}

		//循环写入数据
		for(by = by0; by < by1; by++){	//height*2才能写完全部，不然只有一半
			y = container->y + by;

			for(bx = bx0; bx < bx1; bx++){
				x = container->x + bx;
				//照着map中的关系进行刷新				
				if(map[(y * bogui_container_manager->width + x)] == container_id){
					//从自己的buf中读取
					bogui_container_read_pixel(container, bx, by, (uint32_t *)&color);

					//写像素
					wrmsg.m_x = x;
					wrmsg.m_y = y;

					/*窗口的透明度在这个地方进行写入*/
					wrmsg.m_color = COLOR_RGB(color.red, color.green, color.blue); 

					video_transmit(&wrmsg);
				}
			}
		}
	}
}

static void bogui_container_refresh_map(int x0, int y0, int x1, int y1, int h0)
{
	//return;
	int h, bx, by, x, y, bx0, by0, bx1, by1;	//x,y是图层在屏幕中的位置
	uint8_t *map = bogui_container_manager->map;

	rgba_t color;
	bogui_container_t *container;
	uint8_t container_id;

	if(x0 < 0){x0 = 0;}
	if(y0 < 0){y0 = 0;}
	if(x1 > bogui_container_manager->width){x1 = bogui_container_manager->width;}
	if(y1 > bogui_container_manager->height){y1 = bogui_container_manager->height;}
	
	for(h = h0; h <= bogui_container_manager->top; h++){
		//获取图层
		container = bogui_container_manager->container_addr[h];

		container_id = container - bogui_container_manager->container_table;
		bx0 = x0 - container->x;
		by0 = y0 - container->y;
		bx1 = x1 - container->x;
		by1 = y1 - container->y;
		if(bx0 < 0){bx0 = 0;}
		if(by0 < 0){by0 = 0;}
		if(bx1 > container->width){bx1 = container->width;}
		if(by1 > container->height){by1 = container->height;}
		//循环写入数据
		for(by = by0; by < by1; by++){	//height*2才能写完全部，不然只有一半
			y = container->y + by;
			for(bx = bx0; bx < bx1; bx++){
				x = container->x + bx;		

				bogui_container_read_pixel(container, bx, by, (uint32_t *)&color);

				//如果像素不是透明的，就把它的容器id写进map
				if(color.alpha != 0){
					map[(y * bogui_container_manager->width + x)] = container_id;	//只在第一个位置存放
				}

			}
		}
	}			
}

void bogui_container_refresh(bogui_container_t *container, int x0, int y0, int x1, int y1)
{
	if(container->axis >= 0){
		bogui_container_refresh_buffer(container->x +  x0, container->y +  y0, container->x +  x1, container->y +  y1, container->axis, container->axis);
	}
}

//改变图层高度
void bogui_container_axis(bogui_container_t *container, int axis)
{
	int h, old = container->axis;
	//修正高度
	if(axis > bogui_container_manager->top + 1) axis = bogui_container_manager->top + 1;
	if(axis < -1) axis = -1;
	
	container->axis = axis;		//设定高度
	
	//再container_addr中重新排列
	if(old > axis){	//设置的高度比以前自己高度低
		if(axis >= 0){	//如果新的高度为显示（>=0）
			//把 old  和 new 之间的图层提高一层， 把old覆盖，然后new那一层添加为new图层
			//中间图层升高
			for(h = old; h > axis; h--){
				bogui_container_manager->container_addr[h] = bogui_container_manager->container_addr[h - 1];	//把图层提高
				bogui_container_manager->container_addr[h]->axis = h;				//设置图层高度信息				
			}
			bogui_container_manager->container_addr[axis] = container;		//写入图层
			bogui_container_refresh_map(container->x, container->y, container->x + container->width, container->y + container->height, container->axis + 1);
			bogui_container_refresh_buffer(container->x, container->y, container->x + container->width, container->y + container->height, container->axis + 1, old);
		}else{	/*设置成了隐藏*/
			if(bogui_container_manager->top > old){
				//把原来图层和最高层之间的图层往低端降低
				for(h = old; h < bogui_container_manager->top; h++){
					bogui_container_manager->container_addr[h] = bogui_container_manager->container_addr[h + 1];	//把图层提高
					bogui_container_manager->container_addr[h]->axis = h;				//设置图层高度信息				
				}
			}
			bogui_container_manager->top--;		//隐藏了一个图层，最高层--
			bogui_container_refresh_map(container->x, container->y, container->x + container->width, container->y + container->height, 0);
			bogui_container_refresh_buffer(container->x, container->y, container->x + container->width, container->y + container->height, 0, old - 1);
		}
	} else if(old < axis){ //设置的高度比以前自己高度高
		if(old >= 0){	//以前处于显示中
			//把old 和new height中间的图层下降，然后填写新的相关信息
			for(h = old; h < axis; h++){
				bogui_container_manager->container_addr[h] = bogui_container_manager->container_addr[h + 1];	//把图层降低
				bogui_container_manager->container_addr[h]->axis = h;				//设置图层高度信息				
			}
			bogui_container_manager->container_addr[axis] = container;		//写入图层
		}else{	//原来处于隐藏状态
			//操作最高层和height之间的图层，将它们提高一层
			for(h = bogui_container_manager->top; h >= axis; h--){
				bogui_container_manager->container_addr[h + 1] = bogui_container_manager->container_addr[h];	//把图层提高
				bogui_container_manager->container_addr[h + 1]->axis = h + 1;			//设置图层高度信息				
			}
			bogui_container_manager->container_addr[axis] = container;		//写入图层
			bogui_container_manager->top++;		//增加显示了一个图层，最高层++
			
		}
		bogui_container_refresh_map(container->x, container->y, container->x + container->width, container->y + container->height, axis);
		bogui_container_refresh_buffer(container->x, container->y, container->x + container->width, container->y + container->height, axis, axis);
	}
}

void bogui_container_slide(bogui_container_t *container, int x, int y)
{
	int old_x = container->x, old_y = container->y;
	container->x = x;
	container->y = y;
	if(container->axis >= 0){	//如果改变位置后图层为显示状态
		bogui_container_refresh_map(old_x, old_y, old_x + container->width, old_y + container->height, 0);
		bogui_container_refresh_map(x, y, x + container->width, y + container->height, container->axis);
		
		bogui_container_refresh_buffer(old_x, old_y, old_x + container->width, old_y + container->height, 0, container->axis - 1);
		bogui_container_refresh_buffer(x, y, x + container->width, y + container->height, container->axis, container->axis);
	}
}

void bogui_container_bind_window(bogui_container_t *container, bogui_window_t *window)
{
	container->window = window;
}

void bogui_container_add_attribute(bogui_container_t *container, uint32_t attr)
{
	container->flags |= attr;
}

void bogui_container_sub_attribute(bogui_container_t *container, uint32_t attr)
{
	container->flags &= (uint32_t)(~attr);
}

/*
获取一个鼠标在上面的容器，从上网下进行搜索。
理论上说是不会有NULL容器，因为最底层是桌面，至少都可以获取到。
从最底层到最顶层的下一层
*/
bogui_container_t *bogui_get_container_where_mouse_on(int mousex, int mousey)
{
	int j;
	int x, y;
	bogui_container_t *container;
	for(j = bogui_container_manager->top - 1; j >= 0; j--){
		container = bogui_container_manager->container_addr[j];

		x = mousex - container->x;		//鼠标位置和图层位置的距离
		y = mousey - container->y;
		if(0 <= x && x < container->width && 0 <= y && y < container->height){	
			return container;
		}
	}
	return NULL;
}


void bogui_container_read_pixel(bogui_container_t *container, int x, int y, uint32_t *color)
{
	if (x < 0 || x < 0 || x >= container->width || \
		y >= container->height) {
		return;
	}
	uint8_t *buf = (uint8_t *)(container->buffer + (y * container->width + x)* container->bit_width);
	
	switch (container->bit_width) {
		case 1:
			*color = buf[0];
			break;
		case 2:
			*color = buf[0] | (buf[1]<<8);
			break;
		case 3:
			*color = buf[0] | (buf[1]<<8) | (buf[2]<<16);
			break;
		case 4:
			*color = *((uint32_t *)buf);
			break;
	}
}

void bogui_container_write_pixel(bogui_container_t *container, int x, int y, uint32_t color)
{
	if (x < 0 || x < 0 || x >= container->width || \
		y >= container->height) {
		return;
	}
	uint8_t *buf = (uint8_t *)(container->buffer + (y * container->width + x)* container->bit_width);
	switch (container->bit_width) {
		case 1:
			*buf = color&0xff;
			break;
		case 2:
			*buf++ = color&0xff;
			*buf = (color>>8)&0xff;
			break;
		case 3:
			*buf++ = color&0xff;
			*buf++ = (color>>8)&0xff;
			*buf = (color>>16)&0xff;
			break;
		case 4:
			*((uint32_t *)buf) = color;
			break;
	}
}

void bogui_container_draw_rect(bogui_container_t *container, int x, int y, int width, int height, uint32_t color)
{
	int i, j;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			bogui_container_write_pixel(container, x + i, y + j, color);
		}
	}
}

static void bogui_container_draw_char_bit(bogui_container_t *container, int x, int y , uint32 color, uint8 *ascii)
{
	int i;
	char d /* data */;
	for (i = 0; i < 16; i++) {
		d = ascii[i];
		if ((d & 0x80) != 0) { bogui_container_write_pixel(container, x + 0, y + i, color); }
		if ((d & 0x40) != 0) { bogui_container_write_pixel(container, x + 1, y + i, color); }
		if ((d & 0x20) != 0) { bogui_container_write_pixel(container, x + 2, y + i, color); }
		if ((d & 0x10) != 0) { bogui_container_write_pixel(container, x + 3, y + i, color); }
		if ((d & 0x08) != 0) { bogui_container_write_pixel(container, x + 4, y + i, color); }
		if ((d & 0x04) != 0) { bogui_container_write_pixel(container, x + 5, y + i, color); }
		if ((d & 0x02) != 0) { bogui_container_write_pixel(container, x + 6, y + i, color); }
		if ((d & 0x01) != 0) { bogui_container_write_pixel(container, x + 7, y + i, color); }
	}
	
}

void bogui_container_draw_char(bogui_container_t *container, int x,int y,uint32 color, char ch)
{
	bogui_container_draw_char_bit(container, x, y, color, current_font->addr + ch * current_font->height);
}

void bogui_container_draw_string(bogui_container_t *container, int x,int y, char *s, uint32 color)
{
	for (; *s != 0x00; s++) {
		bogui_container_draw_char(container, x, y, color, *s);
		x += current_font->width;
	}
}

void bogui_container_draw_line(bogui_container_t *container, int x0, int y0, int x1, int y1, uint32_t color)
{
	int i, x, y, len, dx, dy;
	dx = x1 - x0;
	dy = y1 - y0;
	
	x = x0 << 10;
	y = y0 << 10;
	
	if(dx < 0){
		dx = -dx;
	}
	if(dy < 0){
		dy = -dy;
	}
	if(dx >= dy ){
		len = dx + 1;
		if(x0 > x1){
			dx = -1024;
		} else {
			dx = 1024;
			
		}
		if(y0 <= y1){
			dy = ((y1 - y0 + 1) << 10)/len;
		} else {
			dy = ((y1 - y0 - 1) << 10)/len;
		}
		
		
	}else{
		len = dy + 1;
		if(y0 > y1){
			dy = -1024;
		} else {
			dy = 1024;
			
		}
		if(x0 <= x1){
			dx = ((x1 - x0 + 1) << 10)/len;
		} else {
			dx = ((x1 - x0 - 1) << 10)/len;
		}	
	}
	for(i = 0; i < len; i++){
		//buf[((y >> 10)*wide + (x >> 10))*3] = color;
		//graph_write_pixel(buffer,(x >> 10), (y >> 10), color);

		bogui_container_write_pixel(container, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}

}

#endif //_CONFIG_GUI_BOGUI_

