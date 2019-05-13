#ifndef _GUI_BOGUI_MESSAGE_H
#define _GUI_BOGUI_MESSAGE_H

#include <sys/config.h>

#ifdef _CONFIG_GUI_BOGUI_

#include <gui/bogui/container.h>

enum BOGUI_MSG_ID
{
    BOGUI_MSG_UNKNOW,
    BOGUI_MSG_MOUSE,
    BOGUI_MSG_WINDOW_MOVE,
    BOGUI_MSG_WINDOW_FOUCS,
    BOGUI_MSG_WINDOW_CLOSE,
    BOGUI_MSG_WINDOW_ADD,
    BOGUI_MSG_WINDOW_HIDE,
    BOGUI_MSG_WINDOW_SHOW,
    BOGUI_MSG_MOUSE_MOVE,
    BOGUI_MSG_MOUSE_EVEN,
    BOGUI_MSG_WINDOW_TOPEST,
};

#define BOGUI_MAX_MESSAGE_NR 64

typedef struct bogui_message_data_s
{
    int flags[8];
    int i32[12];
    void *ptr[12];
} bogui_message_data_t;

typedef struct bogui_message
{
    enum BOGUI_MSG_ID id;    //消息的标识符
    /*bogui_container_t *receiver;    //消息的接受容器
    bogui_container_t *sender;    //消息的发送容器
    bogui_container_t *replyer;    //消息的回复容器
    uint32_t time;  //消息的产生时间
*/
    struct bogui_message_data_s data;  //私有数据

    struct bogui_message *next;
} bogui_message_t;

extern bogui_message_t *bogui_msg_header;


void bogui_message_init();

int bogui_msg_receive(bogui_message_t *msg);
int bogui_msg_send(bogui_message_t *msg);


#endif //_CONFIG_GUI_BOGUI_

#endif //_GUI_BOGUI_MESSAGE_H

