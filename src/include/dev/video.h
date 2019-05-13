#ifndef _DEVICE_VIDEO_H_
#define _DEVICE_VIDEO_H_

#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_VIDEO_

#include <types.h>
#include <stdint.h>

#define VIDEO_INFO_ADDR 0x800 

/*
	video_info_INFO_ADDR = vodeo color 
	video_info_INFO_ADDR+2 = vodeo wide
	video_info_INFO_ADDR+4 = vodeo high
	video_info_INFO_ADDR+6 = vodeo ram
*/
#define VIDEO_VRAM_ADDR 0x400000

struct video_info {
	uint16_t bits_per_pixe;		//2字节
	uint32_t width, height;	//2字节+2字节
	uint8_t *vram;				//显存地址
	uint8_t *buffer;		//显存缓冲
	uint8 pixel_width;		//2字节
};

extern struct video_info video_info;

void init_video(void);

void video_clean_screen();
void sys_get_screen(int *width, int *height);

/*we transmit msg to video*/
#define VIDEO_MSG_WRITE_PIXEL	0 
#define VIDEO_MSG_READ_PIXEL 	1

typedef struct video_message_s
{
	uint8 m_type;					/*message type*/
	int32 m_x, m_y;					/*pixle x, y*/
	int32 m_width, m_height;		/*buffer width, height*/
	uint32 m_color;					/*pixel color*/

}video_message_t;	

void video_transmit(struct video_message_s *msg);
void video_transmit_to_buffer(struct video_message_s *msg);
void video_refresh_position(int x0, int y0, int x1, int y1);


#endif	/*_CONFIG_DEVIVE_VIDEO_*/

#endif	/*_DEVICE_VIDEO_H_*/

