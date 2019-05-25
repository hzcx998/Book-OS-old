#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include "boex.h"
#include "item.h"
#include "status_bar.h"

/*
item button mouse [btn]
*/
void item_btn_down_middle(int mx, int my);
void item_btn_down_left(int mx, int my);
void item_btn_down_right(int mx, int my);
void item_btn_down_trailer(int mx, int my);
void item_btn_down_home(int mx, int my);

void item_btn_up_left(int mx, int my);
void item_btn_up_right(int mx, int my);
void item_btn_up_middle(int mx, int my);
void item_btn_up_home(int mx, int my);
void item_btn_up_trailer(int mx, int my);

item_manager_t *cur_item_manager;

extern item_manager_t catalog_item_manager, favorite_item_manager;
extern status_bar_t status_bar;

int item_manager_init(item_manager_t *item_manager, int max_items, int x, int y, \
	int width, int height, int item_width)
{
	item_manager->item_talbe = malloc(sizeof(struct item_s)*max_items);
	if (item_manager->item_talbe == NULL) {
		return -1;
	}
	item_manager->x = x;
	item_manager->y = y;
	item_manager->width = width;
	item_manager->height = height;

	item_manager->page_idx = 0;
	item_manager->items = 0;
	item_manager->pages = 0;	//默认是1个页

	item_manager->cur_item = NULL;
	item_manager->cur_page = 0;
	item_manager->max_items = max_items;

	item_manager->page_items = height/ITEM_DEFAULT_HEIGHT-1;

	//printf("item manager: max items:%d page items:%d\n", item_manager->max_items, item_manager->page_items);

	item_manager_color_face(item_manager, BACK_COLOR, FRONT_COLOR);

	int i;
	item_t *item;
	for (i = 0; i < max_items; i++) {
		item = &item_manager->item_talbe[i];

		item_init(item, 0, 0, item_width, ITEM_DEFAULT_HEIGHT);
		item_color_status(item, ITEM_COLOR_IDLE, ITEM_COLOR_TOUCH, ITEM_COLOR_CLICK);
		item_color_face(item, BACK_COLOR, FRONT_COLOR);
		item_bind_even(item, NULL, NULL);
		item_set_text(item, " ");
	}

	button_t *button;
	int btn_x, btn_y;
	btn_y = item_manager->y + height-BUTTON_DEFAULT_HEIGHT;	//把按钮绘制到中下位置
	btn_x = item_manager->x + width/2 - (BUTTON_DEFAULT_WIDTH*2+BUTTON_DEFAULT_WIDTH/2);
	//btn_x = item_manager->x + BUTTON_DEFAULT_WIDTH;

	item_manager->button = malloc(sizeof(struct button_s)*ITEM_CTRL_BUTTON_NR);
	if (item_manager->button == NULL) {
		free(item_manager->item_talbe);
		return -1;
	}
	for (i = 0; i < ITEM_CTRL_BUTTON_NR; i++) {
		button = &item_manager->button[i];

		button_init(button, btn_x, btn_y, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);
		button_color_status(button, BACK_COLOR, MIDDLE_COLOR, BUTTON_COLOR_CLICK);
		button_color_face(button, BACK_COLOR, ARGB_BLACK);

		//就按钮的显示和事件不同
		if (i == 0) {
			button_set_text(button, "{");
			button_bind_even(button, item_btn_down_home, item_btn_up_home);
		} else if (i == 1) {
			button_set_text(button, "<");
			button_bind_even(button, item_btn_down_left, item_btn_up_left);
		} else if (i == 2) {
			button_set_text(button, "^");
			button_bind_even(button, item_btn_down_middle, item_btn_up_middle);
		} else if (i == 3) {
			button_set_text(button, ">");
			button_bind_even(button, item_btn_down_right, item_btn_up_right);
		} else if (i == 4) {
			button_set_text(button, "}");
			button_bind_even(button, item_btn_down_trailer, item_btn_up_trailer);
		} 

		button_draw(button);

		btn_x += BUTTON_DEFAULT_WIDTH;
	}
	return 0;
}

void item_manager_color_face(item_manager_t *item_manager, uint32_t bcolor, uint32_t fcolor)
{
	item_manager->bcolor = bcolor;
	item_manager->fcolor = fcolor;
}

item_t *item_manager_alloc(item_manager_t *item_manager)
{
	int i;
	item_t *item;
	for (i = 0; i < item_manager->max_items; i++) {
		item = &item_manager->item_talbe[i];
		if (item->status == ITEM_STATUS_UNUSED) {
			item->status = ITEM_STATUS_IDLE;

			item_set_pos(item, item_manager->pages, item_manager->x, \
				item_manager->y + item_manager->page_idx*item->height);
			//printf("item: x %d y %d page %d \n", item->x, item->y, item->page);
			item_manager->items++;	//添加一个项
			item_manager->page_idx++;	//当前页面的项增加

			if (item_manager->page_idx >= item_manager->page_items) {	//超过一个页
				item_manager->pages++;	//页面数增加
				item_manager->page_idx = 0;	//idx 归零
			}

			return item;
		}
	}
	return NULL;
}

void item_manager_change_page(item_manager_t *item_manager, int page)
{
	if (page < 0) {
		page = 0;
	}

	if (page > item_manager->pages) {
		page = item_manager->pages;
	}
	//printf("cur pages:%d\n", item_manager->cur_page);
	//已经在本页，就不做切换
	if (item_manager->cur_page == page) {
		return;
	}
	item_manager->cur_page = page;

	//printf("new pages:%d\n", item_manager->cur_page);

	//清空屏幕
	gui_color(item_manager->bcolor);
	gui_rect(item_manager->x, item_manager->y, item_manager->width, item_manager->height-ITEM_DEFAULT_HEIGHT*2);
	
	status_bar_message(&status_bar, "cur page is %d", item_manager->cur_page);
	//把条目刷新出来
	int i;
	item_t *item;
	for (i = 0; i < item_manager->max_items; i++) {
		item = &item_manager->item_talbe[i];
		if (item->status != ITEM_STATUS_UNUSED) {
			item_draw(item_manager, item);
		}
	}

	gui_draw(item_manager->x, item_manager->y, item_manager->x + item_manager->width, \
		item_manager->y + (item_manager->height-ITEM_DEFAULT_HEIGHT*2));
	
}

void item_manager_free(item_manager_t *item_manager, item_t *item)
{
	int i;
	for (i = 0; i < item_manager->max_items; i++) {
		if (&item_manager->item_talbe[i] == item) {
			item->status = ITEM_STATUS_UNUSED;
			break;
		}
	}
	
}

void item_manager_update(item_manager_t *item_manager, int mousex, int mousey, int status)
{
	int i;
	item_t *item;
	//更新item
	for (i = 0; i < item_manager->max_items; i++) {
		item = &item_manager->item_talbe[i];
		if (item->page == item_manager->cur_page) {
			item_update(item_manager, item, mousex, mousey, status);	
		}
	}

	button_t *button;
	//更新按钮
	for (i = 0; i < ITEM_CTRL_BUTTON_NR; i++) {
		button = &item_manager->button[i];
		button_update(button, mousex, mousey, status);
	}
	
}

bool is_item_manager_mouse_on(item_manager_t *item_manager, int mx, int my)
{
	//判断是否在范围内
	if (item_manager->x <= mx && mx < item_manager->x + item_manager->width && \
		item_manager->y <= my && my < item_manager->y + item_manager->height) 
	{
		return true;
	}
	return false;
}


void item_init(item_t *item, int x, int y, int width, int height)
{
	item->x = x;
	item->y = y;
	item->width = width;
	item->height = height;
	item->status = ITEM_STATUS_UNUSED;
	item->mouse_down = NULL;
	item->mouse_up = NULL;
	item->page = 0;
}

void item_set_text(item_t *item, char *text)
{
	memset(item->text, 0, ITEM_TEXT_LEN);
	strcpy(item->text, text);
}

void item_color_status(item_t *item, uint32_t idle, uint32_t touch, uint32_t click)
{
	item->color_idle = idle;
	item->color_touch = touch;
	item->color_click = click;
}

void item_color_face(item_t *item, uint32_t bcolor, uint32_t fcolor)
{
	item->bcolor = bcolor;
	item->fcolor = fcolor;
}

void item_set_pos(item_t *item, int page, int x, int y)
{
	item->page = page;
	item->x = x;
	item->y = y;
}

void item_bind_even(item_t *item, void (*mouse_down)(), void (*mouse_up)())
{
	item->mouse_down = mouse_down;
	item->mouse_up = mouse_up;
}

void item_change_status(item_manager_t *item_manager, item_t *item, int status)
{
	item->status = status;
	item_draw(item_manager, item);
}

void item_draw(item_manager_t *item_manager, item_t *item)
{
	if (item->status == ITEM_STATUS_UNUSED) {
		return;
	}
	if (item->page != item_manager->cur_page) {
		return;
	}
	uint32_t color;

	if (item->status == ITEM_STATUS_IDLE) {
		color = item->color_idle;
	} else if (item->status == ITEM_STATUS_TOUCH) {
		color = item->color_touch;
	} else if (item->status == ITEM_STATUS_CLICK) {
		color = item->color_click;
	}

	//绘制底层
	gui_color(color);
	gui_rect(item->x, item->y, item->width, item->height);

	//绘制文字
	if (item->text[0] != 0) {
		gui_color(item->fcolor);
		int x, y;
		x = item->width/2 - strlen(item->text)*8/2;
		y = item->height/2 - 16/2;
		gui_text(item->x + x, item->y + y, item->text);
	}
	
	gui_draw(item->x, item->y, item->x + item->width, item->y + item->height);

}

void item_update(item_manager_t *item_manager, item_t *item, int mousex, int mousey, int status)
{
	if (item->status == ITEM_STATUS_UNUSED) {
		return;
	}
	//判断鼠标位置
	if (item->x <= mousex && mousex < item->x + item->width && \
		item->y <= mousey && mousey < item->y + item->height ) 
	{
		//在范围内
		
		//改变颜色
		if (item->status == ITEM_STATUS_IDLE) {
			item->status = ITEM_STATUS_TOUCH;
			item_draw(item_manager, item);
		}

		//如果鼠标点击
		if (status == MOUSE_DOWN) {
			//先做内部操作
			item_change_status(item_manager, item, ITEM_STATUS_CLICK);
			cur_item_manager = item_manager;

			if (item->mouse_down != NULL) {
				//执行鼠标其它事件
				item->mouse_down();
			}
		} else if (status == MOUSE_UP) {
			//先做内部操作
			item_change_status(item_manager, item, ITEM_STATUS_TOUCH);
			
			if (item->mouse_up != NULL) {
				//执行鼠标其它事件
				item->mouse_up();
			}
		}
	} else {
		//改变颜色
		if (item->status != ITEM_STATUS_IDLE) {
			item->status = ITEM_STATUS_IDLE;
			item_draw(item_manager, item);
		}
	}
}

/*
button down
*/

void item_btn_down_left(int mx, int my)
{
	status_bar_message(&status_bar, "button left mouse down");
	
	//判断鼠标是否在范围内
	if (is_item_manager_mouse_on(&catalog_item_manager, mx, my)) {
		//设定指针
		cur_item_manager = &catalog_item_manager;
	} else if (is_item_manager_mouse_on(&favorite_item_manager, mx, my)) {
		//设定指针
		cur_item_manager = &favorite_item_manager;
	} 

	
	
}

void item_btn_down_right(int mx, int my)
{
	status_bar_message(&status_bar, "button right mouse down");
	
	//判断鼠标是否在范围内
	if (is_item_manager_mouse_on(&catalog_item_manager, mx, my)) {

		//设定指针
		cur_item_manager = &catalog_item_manager;
	} else if (is_item_manager_mouse_on(&favorite_item_manager, mx, my)) {
		//设定指针
		cur_item_manager = &favorite_item_manager;
	} 
}

void item_btn_down_middle(int mx, int my)
{
	status_bar_message(&status_bar, "button middle mouse down");
	
	//判断鼠标是否在范围内
	if (is_item_manager_mouse_on(&catalog_item_manager, mx, my)) {

		//设定指针
		cur_item_manager = &catalog_item_manager;
	} else if (is_item_manager_mouse_on(&favorite_item_manager, mx, my)) {
		//设定指针
		cur_item_manager = &favorite_item_manager;
	} 
}

void item_btn_down_home(int mx, int my)
{
	status_bar_message(&status_bar, "button home mouse down");
	
	//判断鼠标是否在范围内
	if (is_item_manager_mouse_on(&catalog_item_manager, mx, my)) {

		//设定指针
		cur_item_manager = &catalog_item_manager;
	} else if (is_item_manager_mouse_on(&favorite_item_manager, mx, my)) {
		//设定指针
		cur_item_manager = &favorite_item_manager;
	} 
}

void item_btn_down_trailer(int mx, int my)
{
	status_bar_message(&status_bar, "button trailer mouse down");
	
	//判断鼠标是否在范围内
	if (is_item_manager_mouse_on(&catalog_item_manager, mx, my)) {

		//设定指针
		cur_item_manager = &catalog_item_manager;
	} else if (is_item_manager_mouse_on(&favorite_item_manager, mx, my)) {
		//设定指针
		cur_item_manager = &favorite_item_manager;
	} 
}

/*
button up
*/

void item_btn_up_left(int mx, int my)
{
	status_bar_message(&status_bar, "button left mouse up");
	
	//切换到上一个页面
	item_manager_change_page(cur_item_manager, cur_item_manager->cur_page-1);

}

void item_btn_up_right(int mx, int my)
{
	status_bar_message(&status_bar, "button right mouse up");
	
	//切换到下一个页面
	item_manager_change_page(cur_item_manager, cur_item_manager->cur_page+1);

}

void item_btn_up_middle(int mx, int my)
{
	status_bar_message(&status_bar, "button middle mouse down");
	
	item_manager_change_page(cur_item_manager, cur_item_manager->pages/2);
	
}

void item_btn_up_home(int mx, int my)
{
	status_bar_message(&status_bar, "button home mouse down");
	
	item_manager_change_page(cur_item_manager, 0);

}

void item_btn_up_trailer(int mx, int my)
{
	status_bar_message(&status_bar, "button trailer mouse down");
	
	item_manager_change_page(cur_item_manager, cur_item_manager->pages);

}
