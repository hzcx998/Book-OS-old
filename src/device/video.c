/*
File:		device/video.c
Contains:	driver for video
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_VIDEO_

#include <sys/dev.h>
#include <sys/core.h>

#include <string.h>
#include <sys/mm.h>
#include <sys/gui.h>
#include <sys/debug.h>

struct video_info video_info;

void init_video()
{
	//直接从地址获取
	video_info.bits_per_pixe = *((uint16_t *)VIDEO_INFO_ADDR);
	video_info.width = (uint32_t )(*((uint16_t *)(VIDEO_INFO_ADDR+2)));
	video_info.height = (uint32_t )(*((uint16_t *)(VIDEO_INFO_ADDR+4)));
	
	video_info.pixel_width = video_info.bits_per_pixe/8;
	
	//先获取地址，在转换成指针
	//vram_addr = (uint32_t *)(VIDEO_INFO_ADDR+6);
	video_info.vram = (uint8_t *) VIDEO_VRAM_ADDR;
	
	video_info.buffer = (uint8_t *) mm.malloc(video_info.width*video_info.height*4);
	if (video_info.buffer == NULL) {
		panic("alloc buffer for video failed!\n");
	}
	display_start_logo();
/*
	video_message_t msg;
	msg.m_type = VIDEO_MSG_WRITE_PIXEL;

	msg.m_x = 5;
	msg.m_y = 5;
	msg.m_color = ARGB_BLUE;
	video_transmit_to_buffer(&msg);

	msg.m_type = VIDEO_MSG_READ_PIXEL;

	msg.m_x = 5;
	msg.m_y = 5;
	msg.m_color = 0;
	video_transmit_to_buffer(&msg);

	msg.m_type = VIDEO_MSG_WRITE_PIXEL;

	msg.m_x = 8;
	msg.m_y = 8;
	video_transmit_to_buffer(&msg);

	video_refresh_position(1, 1, 10, 10);
*/
}
/*
把颜色传输给显存
*/
void video_transmit(struct video_message_s *msg)
{
	//msg error, return
	if (msg == NULL) {
		return;
	}

	//get vram start addr
	uint8 *vram;

	//we do different operate by message type
	if (msg->m_type == VIDEO_MSG_WRITE_PIXEL) {
		
		//if out of screen
		if (msg->m_x < 0 || msg->m_y < 0 || msg->m_x >= video_info.width || \
			msg->m_y >= video_info.height) {
			return;
		}

		vram = (uint8 *)(video_info.vram + (msg->m_y*video_info.width+msg->m_x)*video_info.pixel_width);

		//we write vram through different model
		/*
		switch (video_info.pixel_width) {
			case 1:
				*vram = msg->m_color&0xff;
				break;
			case 2:
				*vram++ = msg->m_color&0xff;
				*vram = (msg->m_color>>8)&0xff;
				break;
			case 3:
				*vram++ = msg->m_color&0xff;
				*vram++ = (msg->m_color>>8)&0xff;
				*vram = (msg->m_color>>16)&0xff;
				break;
			case 4:
				*((uint32_t *)vram) = msg->m_color;
				break;
		}*/

		
		#ifdef _VIDEO_16_MODE_
			*vram++ = msg->m_color&0xff;
			*vram = (msg->m_color>>8)&0xff;
		#endif //_VIDEO_16_MODEL_

		#ifdef _VIDEO_24_MODE_
			*vram++ = msg->m_color&0xff;
			*vram++ = (msg->m_color>>8)&0xff;
			*vram = (msg->m_color>>16)&0xff;
		#endif //_VIDEO_24_MODEL_
		

	} else if (msg->m_type == VIDEO_MSG_READ_PIXEL) {
		//if out of screen
		if (msg->m_x < 0 || msg->m_y < 0 || msg->m_x >= video_info.width || \
			msg->m_y >= video_info.height) {
			return;
		}

		vram = (uint8 *)(video_info.vram + (msg->m_y*video_info.width+msg->m_x)*video_info.pixel_width);
/*
		switch (msg->m_bit_width) {
			case 1:
				msg->m_color = vram[0];
				break;
			case 2:
				msg->m_color = vram[0] | (vram[1]<<8);
				break;
			case 3:
				msg->m_color = vram[0] | (vram[1]<<8) | (vram[2]<<16);
				break;
			case 4:
				msg->m_color = *((uint32_t *)vram);
				break;
		}*/
		
		//we write vram through different model
		#ifdef _VIDEO_16_MODE_
			msg->m_color = vram[0] | (vram[1]<<8);
		#endif //_VIDEO_16_MODEL_

		#ifdef _VIDEO_24_MODE_
			msg->m_color = vram[0] | (vram[1]<<8) | (vram[2]<<16);
		#endif //_VIDEO_24_MODEL_
		
	}
}
/*
把颜色传输给buffer
*/
void video_transmit_to_buffer(struct video_message_s *msg)
{
	//msg error, return
	if (msg == NULL) {
		return;
	}

	//get buffer start addr
	uint8_t *buffer;

	//we do different operate by message type
	if (msg->m_type == VIDEO_MSG_WRITE_PIXEL) {
		
		//if out of screen
		if (msg->m_x < 0 || msg->m_y < 0 || msg->m_x >= video_info.width || \
			msg->m_y >= video_info.height) {
			return;
		}

		buffer = (uint8_t *)(video_info.buffer + (msg->m_y*video_info.width+msg->m_x)*4);

		*buffer++ = msg->m_color&0xff;
		*buffer++ = (msg->m_color>>8)&0xff;
		*buffer++ = (msg->m_color>>16)&0xff;
		*buffer = (msg->m_color>>24)&0xff;

	} else if (msg->m_type == VIDEO_MSG_READ_PIXEL) {
		//if out of screen
		if (msg->m_x < 0 || msg->m_y < 0 || msg->m_x >= video_info.width || \
			msg->m_y >= video_info.height) {
			return;
		}

		buffer = (uint8_t *)(video_info.buffer + (msg->m_y*video_info.width+msg->m_x)*4);

		msg->m_color = buffer[0] | (buffer[1]<<8) | (buffer[2]<<16)| (buffer[3]<<24);

	}
}

void video_clean_screen()
{
	int x, y;
	for(y = 0; y < video_info.height; y++){	//high*2才能写完全部，不然只有一半
		for(x = 0; x < video_info.width; x++){
			//vram_write_pixel(x, y, COLOR_BLACK);
		}
	}
}

void sys_get_screen(int *width, int *height)
{
	*width = video_info.width;
	*height = video_info.height;
}

/*
把buffer里面的数据传输到vram
*/
void video_refresh_position(int x0, int y0, int x1, int y1)
{
	uint8_t *buffer;
	int x, y;

	video_message_t msg;
	msg.m_type = VIDEO_MSG_WRITE_PIXEL;

	for (y = y0; y < y1; y++) {
		for (x = x0; x < x1; x++) {
			//从buffer获取颜色值
			//计算buffer地址
			buffer = video_info.buffer + (y*video_info.width + x)*4;

			if (buffer[3] != 0) {
				//颜色值转换成video模式对应的颜色
				msg.m_color = COLOR_RGB(buffer[2], buffer[1], buffer[0]);

				//颜色写入显存
				msg.m_x = x;
				msg.m_y = y;
				video_transmit(&msg);	

			}
			

		}

	}

}

#endif //_CONFIG_DEVIVE_VIDEO_
