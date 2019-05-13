/*************************
*bmp.c�ļ�
*************************/

#include <sys/config.h>

#ifdef _CONFIG_GUI_BASIC_

#include <sys/dev.h>
#include <sys/fs.h>
#include <sys/core.h>

#include <string.h>

#include <sys/gui.h>
#include <sys/mm.h>


int bmp_draw_picture(uint8 *buffer, const char *filename, int x, int y)
{
	
	printk("BMP: name:%s\n", filename);
	/*1.����ļ��Ƿ����*/
	int ack = fs.access(filename, F_OK);
	if(ack == -1){
		printk("BMP: file %s not exist!\n", filename);
		return -1;
	}
	/*2.��ʼ�ļ����ݵĻ�ȡ*/
	
	uint32 off_x = x, off_y = y;
	int line_x = 0, line_y = 0;
	uint32 color = 0;

	BitMapFileHeader FileHead;
	BitMapInfoHeader InfoHead;
	RgbQuad rgb;

	memset(&FileHead, 0, sizeof(BitMapFileHeader));
	memset(&InfoHead, 0, sizeof(BitMapInfoHeader));
	
	//��.bmp�ļ�
	int fd = fs.open(filename, O_RDONLY);
	if (fd == -1){
		printk("BMP: %s file open failed!", filename);
		return -1;
	}
	
	//��ȡ�ļ�ͷ����
	if (fs.read(fd, &FileHead, sizeof(BitMapFileHeader)) == -1){
		printk("BMP: read BitMapFileHeader error!\n");
		fs.close(fd);
		return -1;
	}
	
	if (memcmp(FileHead.bfType, "BM", 2) != 0){
		printk("BMP: %s not a BMP format file!\n", filename);
		fs.close(fd);
		return -1;
	}
	
	//��ȡ��Ϣͷ����
	if (fs.read(fd, (char *)&InfoHead, sizeof(BitMapInfoHeader)) == -1){
		printk("BMP: read BitMapInfoHeader error!\n");
		fs.close(fd);
		return -1;
	}
	//��fd posָ����������
	fs.lseek(fd, FileHead.bfOffBits, SEEK_SET);
	
	/*��ȡͼ������λ��*/
	uint8 bit_len = InfoHead.biBitCount / 8;
 
	uint8 *buf_image = (uint8 *)mm.malloc(InfoHead.biSizeImage);
	uint8 *buf = buf_image;
	if(buf_image == NULL){
		printk("BMP: alloc memory for buf failed!\n");
		fs.close(fd);
		return -1;
	}

	/*��ȡ���ݵ�buf����������������ݽ�����ʾ*/
	memset(buf, 0, InfoHead.biSizeImage);
	if (fs.read(fd,(uint8 *)buf, InfoHead.biSizeImage) != InfoHead.biSizeImage){
		printk("BMP: read data failed!\n");
		
		mm.free(buf_image);
		fs.close(fd);
		return -1;
	}
	
	int image_size = InfoHead.biSizeImage;

	rgb.Reserved = 0xFF;
	line_x = 0;
	line_y = InfoHead.biHeight-1;
	
	while(image_size > 0){
		color = 0;
		memcpy(&rgb, buf, bit_len);
		color = COLOR_RGB(rgb.Red, rgb.Green, rgb.Blue);
		
		/*д�뵽������*/
		graph_write_pixel(buffer, off_x+line_x, off_y+line_y, color);
		
		image_size-=bit_len;
		buf += bit_len;
		//������ʾ�������ļ��ṹ���⣩
		line_x++;
		if (line_x == InfoHead.biWidth){
			line_x = 0;
			line_y--;
			if(line_y < 0)break;
		}
	}
	/*��ʾ����Ļ��*/
	graph_refresh(buffer, off_x, off_y, off_x + InfoHead.biWidth,off_y + InfoHead.biHeight);
	return 0;
}

/*
int32_t layer_image_bmp(struct layer *layer, uint32_t x,uint32_t y, const char *bmpname)
{
	if(NULL == bmpname)return -1;
	
	uint32_t off_x = x, off_y = y;
	int line_x = 0, line_y = 0;
	uint16_t tmp = 0;

	BitMapFileHeader FileHead;
	BitMapInfoHeader InfoHead;
	RgbQuad rgb;

	memset(&FileHead, 0, sizeof(BitMapFileHeader));
	memset(&InfoHead, 0, sizeof(BitMapInfoHeader));
	
	//��.bmp�ļ�
	//gprintk("read %d#");
	int fd = sys_open(bmpname, O_RDWR);
	
	//gprintk("fd %d#",fd );
	if (fd == -1){
		gprintk("fopen bmp error#");
		return -1;
	}

	//���ļ���Ϣ
	if (0 == sys_read(fd, &FileHead, sizeof(BitMapFileHeader))){
		gprintk("read BitMapFileHeader error!#");
		sys_fclose(fd);
		return -1;
	}
	if (memcmp(FileHead.bfType, "BM", 2) != 0){
		gprintk("it's not a BMP file\n");
		sys_fclose(fd);
		return -1;
	}
	
	//gprintk("image head info: size:%d Off:%d#",FileHead.bfSize,FileHead.bfOffBits);
 
	//��λͼ��Ϣ
	if (0 == sys_read(fd, (char *)&InfoHead, sizeof(BitMapInfoHeader))){
		gprintk("read BitMapInfoHeader error!#");
		sys_fclose(fd);
		return -1;
	}
 //��ת��������
	sys_lseek(fd, FileHead.bfOffBits, SEEK_SET);
	//gprintk("image info: size:%d width:%d height:%d image size:%d bit:%d#",InfoHead.biSize,InfoHead.biWidth, InfoHead.biHeight, InfoHead.biSizeImage, InfoHead.biBitCount);

	int len = InfoHead.biBitCount / 8;
 
	uint8_t *buf_image = (uint8_t *)sys_malloc(InfoHead.biSizeImage);
	uint8_t *buf = buf_image;
	//gprintk("alloced ");
 
	memset(buf, 0x00, InfoHead.biSizeImage);
	if (0 == sys_read(fd,(uint8_t *)buf, InfoHead.biSizeImage)){
		sys_mfree(buf_image);
		return -1;
	}

	int image_size = InfoHead.biSizeImage;

	rgb.Reserved = 0xFF;
	line_x = 0;
	line_y = InfoHead.biHeight;
	//gprintk("b2 ");
	
	//gprintk("print#");
	while(image_size > 0){
		tmp = 0;
		memcpy(&rgb, buf, len);
		tmp = RGB16(rgb.Red, rgb.Green, rgb.Blue);
		draw_pix16(layer->buf,layer->wide, off_x+line_x, off_y+line_y-1, tmp);
		image_size-=len;
		buf += len;
		//������ʾ�������ļ��ṹ���⣩
		line_x++;
		if (line_x == InfoHead.biWidth){
			line_x = 0;
			line_y--;
			if(line_y == 0)break;
		}
	}
	
	layer_refresh(layer,off_x,off_y,off_x+InfoHead.biWidth+1, off_y+InfoHead.biHeight+1);
	
	//sys_mfree(buf_image);
	
	//sys_fclose(fd);
	return 0;
}
*/

#endif //_CONFIG_GUI_BASIC_
