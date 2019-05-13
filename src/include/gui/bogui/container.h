#ifndef _GUI_BOGUI_CONTAINER_H
#define _GUI_BOGUI_CONTAINER_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <sys/core.h>
#include <gui/bogui/window.h>

#define BOGUI_CONTAINER_NAME_LEN 16
#define BOGUI_CONTAINER_BIT_WIDHT 4

/*容器的flags*/
#define BOGUI_CONTAINER_UNUSED  0X00
#define BOGUI_CONTAINER_USING   0X80000000

#define BOGUI_CONTAINER_BUTTOM   0X01     /*位于底层的容器*/
#define BOGUI_CONTAINER_TOP      0X02     /*位于底层的容器*/
#define BOGUI_CONTAINER_WINDOW   0X04     /*是一个窗口的容器*/
#define BOGUI_CONTAINER_FLOAT    0X08     /*是一个悬浮的容器*/
#define BOGUI_CONTAINER_FIXED    0X10     /*是一个固定的容器*/
#define BOGUI_CONTAINER_HIDE     0X20     /*是一个固定的容器*/

#define GOGUI_MOUSE_MOVE_LISTEN     0X01    
#define GOGUI_MOUSE_CLICK_LISTEN    0X02  
#define GOGUI_KEYBOARD_LISTEN       0X03 


/*
容器
容器里面的颜色要是argb格式的
*/
typedef struct bogui_container_s
{
    int x, y;   //容器在屏幕上的位置

    int axis;      //容器的Z轴
	int width, height; //容器在屏幕上的及宽高
    int bit_width;      //容器中每个像素占用几个字节
    
    int id;
    uint8_t refresh_lock;   //控制容器是否可以刷新

    int buffer_size;
	uint8_t *buffer;    //容器显存数据的buffer



	uint32_t flags; //容器的标志
    char name[BOGUI_CONTAINER_NAME_LEN];    //容器的名字
    struct bogui_window_s *window;

    /*
    容器上的事件处理
    */
    void (*mouse_move_listen)(struct bogui_container_s *, int , int);  //鼠标移动监听
    void (*mouse_click_listen)(struct bogui_container_s *,int , int, int, int);  //鼠标点击监听
    void (*keyboard_listen)(struct bogui_container_s *,int );  //键盘按下监听

    struct thread *thread;
    
}bogui_container_t;

/*约定最大最大只能是 sizeof(char)*/

/*
设定几种窗口等级：

等级1   10   (0x1000     = 1kb)
等级2   10   (0x100000   = 1mb) 
等级3   10   (0x200000   = 2mb) 
等级4   10   (0x300000   = 3mb) 
等级5   10   (0x400000   = 4mb) 

窗口缓冲区大小  <= 100MB 

开始地址为16MB
至少需要固定空间 128

*/

#define BOGUI_CONTAINER_LEVEL_1     0x1000     
#define BOGUI_CONTAINER_LEVEL_2     0x100000     
#define BOGUI_CONTAINER_LEVEL_3     0x200000
#define BOGUI_CONTAINER_LEVEL_4     0x300000
#define BOGUI_CONTAINER_LEVEL_5     0x400000

#define BOGUI_MAX_CONTAINER_NR 50   //系统中一共有多少个容器

#define BOGUI_CONTAINER_SIZE 0X400000

/*
容器管理器
*/
typedef struct bogui_container_manager_s
{
    uint8_t *vram, *map;	//map用来记录图层的id信息
	int width, height, top;	//top用来记录最高层的图层	

    //容器的数组
    struct bogui_container_s container_table[BOGUI_MAX_CONTAINER_NR];
    //容器的地址指针数组
    struct bogui_container_s *container_addr[BOGUI_MAX_CONTAINER_NR];

    struct bogui_container_s *current_container;    //当前是在哪个container
    struct bogui_container_s *hand_container;    //抓取了哪个容器
    struct bogui_container_s *focus_container;    //聚焦了哪个容器
    struct bogui_container_s *moveing_container;    //要移动哪个容器

    uint8_t can_hand_container;

    uint8_t is_container_moving;

    int mouse_off_x, mouse_off_y;   //鼠标在当前窗口中的偏移
    int win_off_x, win_off_y;       //窗口与鼠标的偏移

    int last_mouse_x, last_mouse_y;       //窗口与鼠标的偏移
    
}bogui_container_manager_t;

extern bogui_container_manager_t *bogui_container_manager;

/*
容器自身的管理
*/
bogui_container_manager_t *bogui_container_managerr_init(uint8_t *vram, int width, int height);
bogui_container_t *bogui_container_alloc(int size);
void bogui_container_free(bogui_container_t *container);
void bogui_container_init(bogui_container_t *container, char *name, int x, int y, int width, int height, int bit_width);
void bogui_container_refresh(bogui_container_t *container, int x0, int y0, int x1, int y1);
void bogui_container_axis(bogui_container_t *container, int z);
void bogui_container_slide(bogui_container_t *container, int x, int y);

bogui_container_t *bogui_axis_get_container(int axis);

void bogui_container_add_attribute(bogui_container_t *container, uint32_t attr);
void bogui_container_sub_attribute(bogui_container_t *container, uint32_t attr);

/*容器与窗口的交互*/
void bogui_container_bind_window(bogui_container_t *container, bogui_window_t *window);
void bogui_container_bind_listen(bogui_container_t *container, int type, void (*func));

void bogui_container_bind_thread(bogui_container_t *container, struct thread *thread);

/*容器的其它操作*/
bogui_container_t *bogui_get_container_where_mouse_on(int mousex, int mousey);

/*
容器上面的图形绘制
*/
void bogui_container_read_pixel(bogui_container_t *container, int x, int y, uint32_t *color);
void bogui_container_write_pixel(bogui_container_t *container, int x, int y, uint32_t color);

void bogui_container_draw_rect(bogui_container_t *container, int x, int y, int width, int height, uint32_t color);
void bogui_container_draw_line(bogui_container_t *container, int x0, int y0, int x1, int y1, uint32_t color);

void bogui_container_draw_char(bogui_container_t *container, int x,int y,uint32 color, char ch);
void bogui_container_draw_string(bogui_container_t *container, int x,int y, char *s, uint32 color);


#endif //_CONFIG_GUI_BOGUI_

#endif //_GUI_BOGUI_CONTAINER_H