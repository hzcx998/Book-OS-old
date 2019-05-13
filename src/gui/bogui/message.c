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


bogui_message_t bogui_msg_table[BOGUI_MAX_MESSAGE_NR];

//消息队列头
bogui_message_t *bogui_msg_header;
//当前分配的消息位置
bogui_message_t *bogui_current_msg;

void bogui_message_init()
{
	bogui_msg_header = NULL;
	bogui_current_msg = bogui_msg_table;





}
/*
从消息表中获取一个空闲的消息
*/
static bogui_message_t *bogui_msg_get()
{
	bogui_message_t *msg;
	msg = bogui_current_msg;
	bogui_current_msg++;
	if (bogui_current_msg >= bogui_msg_table + BOGUI_MAX_MESSAGE_NR) {
		bogui_current_msg = bogui_msg_table;
	}
	return msg;
}
/*
发送消息
*/
int bogui_msg_send(bogui_message_t *msg)
{
	if (msg == NULL) {
		return -1;
	}

	bogui_message_t *p = bogui_msg_header;

	bogui_message_t *local_msg;

	/*是空消息*/
	if (bogui_msg_header == NULL) {
		local_msg = bogui_msg_get();
		//memcpy(local_msg, msg, sizeof(bogui_message_t));
		*local_msg = *msg;
		bogui_msg_header = local_msg;
		bogui_msg_header->next = NULL;
		//printk("send head ");
		return 0;
	}

	while(p->next != NULL){
		p = p->next;
	}

	local_msg = bogui_msg_get();
	//memcpy(local_msg, msg, sizeof(bogui_message_t));
	*local_msg = *msg;

	p->next = local_msg;
	local_msg->next = NULL;

	//printk("send tail ");
	return 0;
}

/*
获取一个消息
*/
int bogui_msg_receive(bogui_message_t *msg)
{
	if (bogui_msg_header == NULL) {
		return -1;
	}

	bogui_message_t *P = bogui_msg_header;

	bogui_msg_header = bogui_msg_header->next;
	//printk("R ");
	
	*msg = *P;
	return 0;
}


#endif //_CONFIG_GUI_BOGUI_

