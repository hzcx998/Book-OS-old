/*
File:		device/harddisk.c
Contains:	driver for harddisk
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_HARDDISK_

#include <sys/arch.h>
#include <sys/dev.h>
#include <sys/mm.h>
#include <sys/core.h>
#include <sys/debug.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


u8 channel_counts;
struct ide_channel channels[2];	/*we have 2 channels*/

struct hard_disk *current_hd;
static int harddisk_init_done;
void init_harddisk()
{
	printk("> init harddisk start.\n");

	u8 hard_disk_nr = *((u8 *)HARDDISK_NR_ADDR);
	
	if(hard_disk_nr == 0){
		panic("No hard disk on this computer! At least 1 hd!\n");
	}
	channel_counts = DIV_ROUND_UP(hard_disk_nr, 2);

	#ifdef _DEBUG_HD_
		printk("hard disk counts:%d\n", hard_disk_nr);
		printk("channel counts:%d\n", channel_counts);
	#endif	
	u8 channel_idx = 0;
	struct ide_channel *channel;
	
	harddisk_init_done = 0;
	
	while(channel_idx < channel_counts){
		/*init channel*/
		channel = &channels[channel_idx];
		switch(channel_idx){
			case 0:
				strcpy(channel->name, "Primary");
				channel->prot_base = PRIMARY_PROT_BASE;
				channel->irq = PRIMARY_IRQ;
				break;
			case 1:
				strcpy(channel->name, "Secondary");
				channel->prot_base = SECONDARY_PROT_BASE;
				channel->irq = SECONDARY_IRQ;
				break;
			default:
				panic("channel error!\n");
				break;
		}
		channel->wait_intr = 0;
		channel->intr_occur = 0;
		channel->lock = create_lock();
		/*channel lock*/
		lock_init(channel->lock);
		
		/*
		sema is 0, after disk request data, the process will sleep,
		untill intr occur, harddisk hander will wakeup process
		*/
		//sema_init(channel->sema_done, 0);

		channel_idx++;
	}
	
	channel_idx = 0;
	
	/*registe in idt*/
	while(channel_idx < channel_counts){
		channel = &channels[channel_idx];
		put_irq_handler(channel->irq, harddisk_handler);
		channel_idx++;
	}
	enable_irq(CASCADE_IRQ);
	/*if we only have 1 channel, we don't enable 2'd channel*/
	switch(channel_counts){
		case 2:
			enable_irq(SECONDARY_IRQ);
		case 1:	
			enable_irq(PRIMARY_IRQ);
			break;
		default:
			panic("channel error!\n");
			break;
	}
	
	channel_idx = 0;
	u8 dev_no = 0;
	u8 dev_nr;
	
	u8 hd_nr = hard_disk_nr;
	
	
	while(channel_idx < channel_counts){
		channel = &channels[channel_idx];
		
		/*we may has 1 or 3 hd, so we set dev_nr 1 or 2*/
		if(hd_nr == 1){
			dev_nr = 1;
		}else{
			dev_nr = 2;
		}
		
		/*init hard disk*/
		while(dev_no < dev_nr){
			struct hard_disk *hd = &channel->devices[dev_no];
			hd->channel = channel;
			hd->dev_no = dev_no;

			sprintf(hd->name, "hd-%d", dev_no);

			#ifdef _DEBUG_HD_
				printk("%s %s\n", hd->channel->name, hd->name);
			#endif

			/*init partition*/
			disk_identify(hd);
			dev_no++;
		}
		hd_nr -= 2;
		dev_no = 0;
		channel_idx++;
	}
	
	/*install harddisk device*/
	channel_idx = 0;
	dev_no = 0;
	hd_nr = hard_disk_nr;
	uint8 divice_slave_idx = 0;
	
	while(channel_idx < channel_counts){
		/*we may has 1 or 3 hd, so we set dev_nr 1 or 2*/
		if(hd_nr == 1){
			dev_nr = 1;
		}else{
			dev_nr = 2;
		}
		/*init hard disk*/
		while(dev_no < dev_nr){
			dev_no++;
			//printk("instal harddisk device slave %d.\n", divice_slave_idx);
			
			/*install harddsik to disk device table*/
			disk_device_install(DEVICE_HARDDISK, divice_slave_idx);
			divice_slave_idx++;
		}
		hd_nr -= 2;
		dev_no = 0;
		channel_idx++;
	}
	disk_device_uninstall(MAKE_DEVICE_ID(DEVICE_HARDDISK, 1));
	
	harddisk_init_done = 1;
	printk("< init harddisk done.\n");
}

void harddisk_service()
{
	init_harddisk();
	
	/*char *argv[4];
	argv[0] = "abc";
	argv[1] = "123";
	argv[2] = "def";
	argv[3] = NULL;
	
	task_exec(WRITE_NAME,argv);
	*/
}

void harddisk_handler(int irq)
{
	u8 channel_idx = irq-14;
	struct ide_channel *channel = &channels[channel_idx];
	
	if(channel->wait_intr){
		//printk("intr occur\n");
	
		channel->wait_intr = 0;
		/*if(harddisk_init_done){
			sema_up(channel->sema_done);
		}*/
		channel->intr_occur = 1;
		if (disk_result(channel) != 0) {
			panic("harddisk r/w error!\n");
		}
	}
}

int set_current_hd(char *hd_name)
{
	if(!strcmp(hd_name, "hda")){
		current_hd = &channels[0].devices[0];
		return 0;
	}else if(!strcmp(hd_name, "hdb")){
		current_hd = &channels[0].devices[1];
		return 0;
	}else if(!strcmp(hd_name, "hdc")){
		current_hd = &channels[1].devices[0];
		return 0;
	}else if(!strcmp(hd_name, "hdd")){
		current_hd = &channels[1].devices[1];
		return 0;
	}else{
		return -1;
	}
}

int disk_result(struct ide_channel *channel)
{
	int i = io_in8(REG_STATUS(channel));

	/*not busy, no disk error, no command error*/
	if ((i & (STATUS_BUSY | STATUS_READY|STATUS_WREER| STATUS_ERR)) == STATUS_READY){
		/*no problem*/
		return 0; 
	}
	if(i&STATUS_ERR){
		/*get error*/
		i = io_in8(REG_ERROR(channel));
		printk("hard disk intr error %x\n",i);
	}
	return -1;
}

void set_disk(struct hard_disk *hd)
{
	u8 reg_device = DEV_MBS|DEV_LBA;
	if(hd->dev_no == 1){
		reg_device |= DEV_DEV;
	}
	io_out8(REG_DEV(hd->channel), reg_device);
}

void set_sector(struct hard_disk *hd, u32 lba, u8 counts)
{
	struct ide_channel *channel = hd->channel;
	/*set counts*/
	io_out8(REG_SECT_CNT(channel), counts);
	
	/*set lba*/
	io_out8(REG_LBA_L(channel), lba);
	io_out8(REG_LBA_M(channel), lba>>8);
	io_out8(REG_LBA_H(channel), lba>>16);
	io_out8(REG_DEV(channel), DEV_MBS|DEV_LBA|(hd->dev_no == 1 ? DEV_DEV:0)|lba>>24);
}

void set_cmd_out(struct ide_channel *channel, u8 cmd)
{
	channel->wait_intr = 1;
	channel->intr_occur = 0;
	io_out8(REG_CMD(channel), cmd);
}
/*read data from port */
void read_buf_from_disk(struct hard_disk *hd, void *buf, u8 counts)
{
	u32 size_of_byte;
	if(counts == 0){
		size_of_byte = 256*SECTOR_SIZE;
	}else{
		size_of_byte = counts*SECTOR_SIZE;
	}
	port_read(REG_DATA(hd->channel), buf, size_of_byte);
}
/*write data into port */
void write_buf_to_disk(struct hard_disk *hd, void *buf, u8 counts)
{
	u32 size_of_byte;
	if(counts == 0){
		size_of_byte = 256*SECTOR_SIZE;
	}else{
		size_of_byte = counts*SECTOR_SIZE;
	}
	port_write(REG_DATA(hd->channel), buf, size_of_byte);
}

int harddisk_busy_wait(struct hard_disk *hd)
{
	struct ide_channel *channel = hd->channel;
	u16 timeout = 30*1000;	/*30 second*/
	while(timeout >= 0){
		/*no busy*/
		if(!(io_in8(REG_STATUS(channel))&STATUS_BUSY)){
			/*Request data */
			return (io_in8(REG_STATUS(channel))&STATUS_DRQ);
		}else{
			/*sleep*/
			msec_sleep(10);
		}
		timeout -= 10;
	}
	printk("harddisk_interrupt_wait timeout!\n");
	return 0;
}

void harddisk_interrupt_wait(struct ide_channel *channel)
{
	u16 timeout = 30*1000;	/*30 second*/
	while(timeout >= 0){
		
		if(channel->intr_occur){
			channel->intr_occur = 0;	//reset
			return;
		}else{
			/*sleep*/
			msec_sleep(10);
		}
		timeout -= 10;
	}
	printk("harddisk_interrupt_wait timeout!\n");
}
/*
I don't known why, when counts>1, there are some problem with ti!
So I read one by one.
*/
void hd_read_sector(struct hard_disk *hd, u32 lba, void *buf, u32 counts)
{
	/*lock channel*/
	lock_acquire(hd->channel->lock);
	
	set_disk(hd);
	u32 operate_sectors;
	u32 sectors_finished = 0;
	while(sectors_finished < counts){
		if((sectors_finished + 256) <= counts){
			operate_sectors = 256;
		}else{
			operate_sectors = counts - sectors_finished;      
		}
		set_sector(hd, lba + sectors_finished, operate_sectors);
		set_cmd_out(hd->channel, CMD_READ);
		//sema_down(hd->channel->sema_done);
		harddisk_interrupt_wait(hd->channel);
	
		if(!harddisk_busy_wait(hd)){	/*faild*/
			panic("read sector faild!\n");
		}
		read_buf_from_disk(hd, (void *)((u32)buf + sectors_finished*512), operate_sectors);
		sectors_finished += operate_sectors;
	}
	/*off lock channel*/
	lock_release(hd->channel->lock);
}
/*
I don't known why, when counts>1, there are some problem with ti!
So I write one by one.
*/

void hd_write_sector(struct hard_disk *hd, u32 lba, void *buf, u32 counts)
{
	/*lock channel*/
	lock_acquire(hd->channel->lock);
	
	set_disk(hd);
	
	u32 operate_sectors;
	u32 sectors_finished = 0;
	while(sectors_finished < counts){
		if((sectors_finished + 256) <= counts){
			operate_sectors = 256;
		}else{
			operate_sectors = counts - sectors_finished;      
		}
		set_sector(hd, lba + sectors_finished, operate_sectors);
		set_cmd_out(hd->channel, CMD_WRITE);
		
		if(!harddisk_busy_wait(hd)){	/*faild*/
			panic("read sector faild!\n");
		}
		
		write_buf_to_disk(hd, (void *)((u32)buf + sectors_finished*512), operate_sectors);
		//sema_down(hd->channel->sema_done);
		harddisk_interrupt_wait(hd->channel);
	
		sectors_finished += operate_sectors;
	}
	/*off lock channel*/
	lock_release(hd->channel->lock);
}

void hd_read_sectors(u32 lba, void *buf, u32 counts)
{
	int i = 0;
	while(i < counts){
		hd_read_sector(current_hd, lba + i, buf + i*SECTOR_SIZE, 1);
		i++;
	}
}

void hd_write_sectors(u32 lba, void *buf, u32 counts)
{
	int i = 0;
	while(i < counts){
		hd_write_sector(current_hd, lba + i, buf + i*SECTOR_SIZE, 1);
		i++;
	}
}


void disk_identify(struct hard_disk *hd)
{
	char info[SECTOR_SIZE];
	set_disk(hd);
	set_cmd_out(hd->channel, CMD_IDENTIFY);
	/*wait*/
	//sema_down(hd->channel->sema_done);
	
	harddisk_interrupt_wait(hd->channel);
	
	if(!harddisk_busy_wait(hd)){	/*faild*/
		panic("disk identify faild!\n");
	}
	
	read_buf_from_disk(hd, info, 1);
	
	analysis_identify_info(hd, (u16 *)info);
}

void analysis_identify_info(struct hard_disk *hd, u16* hdinfo)
{
	//Show information
	/*int capabilities = hdinfo[49];
	int cmd_set_supported = hdinfo[83];*/
	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	
	hd->partition.start_lba = 0;
	
	if (sectors > MAX_SECTORS) {
		sectors = MAX_SECTORS;
	}
	
	
	hd->partition.all_sectors = sectors;
	strcpy(hd->partition.name, "main");

	#ifdef _DEBUG_HD_
		printk("LBA supported:%s ",(capabilities & 0x0200) ? "Yes" : "No");
		printk("LBA48 supported:%s ",((cmd_set_supported & 0x0400) ? "Yes" : "No"));
		printk("sectors:%d size: %d MB\n",hd->partition.all_sectors ,hd->partition.all_sectors * SECTOR_SIZE / (1024*1024));
	#endif	
}

#endif //_CONFIG_DEVIVE_HARDDISK_
