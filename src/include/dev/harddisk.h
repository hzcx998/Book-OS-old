#ifndef _DEVICE_HARDDISK_H_
#define _DEVICE_HARDDISK_H_

#include <sys/core.h>
#include <stdint.h>

#define HARDDISK_NR_ADDR 0x0000475

#define PRIMARY_IRQ 14
#define SECONDARY_IRQ 15

#define SECTOR_SIZE		512

#define PRIMARY_PROT_BASE		0X1F0
#define SECONDARY_PROT_BASE		0X170



/*8 bit main status registers*/
#define	STATUS_BUSY		0x80	//Disk busy
#define	STATUS_READY	0x40	//Disk ready for 
#define	STATUS_WREER	0x20	//Disk error
#define	STATUS_SEEK		0x10	//Seek end
#define	STATUS_DRQ		0x08	//Request data
#define	STATUS_ECC		0x04	//ECC check error
#define	STATUS_INDEX	0x02	//Receive index
#define	STATUS_ERR		0x01	//Command error
/*AT disk controller command*/
#define	CMD_RESTORE		0x10	//driver retsore
#define	CMD_READ		0x20	//Read sector
#define	CMD_WRITE		0x30	//Write sector
#define	CMD_VERIFY		0x40	//Sector check
#define	CMD_FORMAT		0x50	//Format track
#define	CMD_INIT		0x60	//Controller init
#define	CMD_SEEK		0x70	//Seek operate
#define	CMD_DIAGNOSE	0x90	//Driver diagnosis command
#define	CMD_SPECIFY		0x91	//Setting up driver parameters
#define CMD_IDENTIFY	0xEC	//Get disk information

/*device register*/
#define DEV_MBS		0xA0	//bit 7 and 5 are 1
#define DEV_LBA		0x40	//chose Lba way
#define DEV_DEV		0x10	//dmaster or slaver

/*hardisk register*/
#define REG_DATA(channel)		(channel->prot_base + 0)
#define REG_ERROR(channel)		(channel->prot_base + 1)
#define REG_SECT_CNT(channel)	(channel->prot_base + 2)
#define REG_LBA_L(channel)		(channel->prot_base + 3)
#define REG_LBA_M(channel)		(channel->prot_base + 4)
#define REG_LBA_H(channel)		(channel->prot_base + 5)
#define REG_DEV(channel)		(channel->prot_base + 6)
#define REG_STATUS(channel)		(channel->prot_base + 7)
#define REG_CMD(channel)		REG_STATUS(channel)
#define REG_ALT_STATUS(channel)	(channel->prot_base + 0X206)
#define REG_CTL(channel)		REG_ALT_STATUS(channel)

#define MAX_SECTORS ((10*1024*1024)/SECTOR_SIZE)
struct partition 
{
	u32 start_lba;
	u32 all_sectors;
	char name[8];
};

struct hard_disk
{
	char name[8];
	struct ide_channel * channel;
	u8 dev_no;
	struct partition partition;	/*only one partition*/
};

struct ide_channel
{
	char name[12];
	u16 prot_base;
	u8 irq;
	int wait_intr;
	int intr_occur;
	
	struct lock *lock;	/*channel lock, a time channel can use for one time*/
	struct semaphore *sema_done;	/*sleep or wakeup harddisk service*/
	struct hard_disk devices[2];	/*2 disk here*/
};

extern struct hard_disk *current_hd;

void init_harddisk();
void harddisk_service();
void IRQ_primary_channel();
void IRQ_secondary_channel();

void harddisk_handler(int irq);

void set_cmd_out(struct ide_channel *channel, u8 cmd);
void set_sector(struct hard_disk *hd, u32 lba, u8 counts);
void set_disk(struct hard_disk *hd);

void read_buf_from_disk(struct hard_disk *hd, void *buf, u8 counts);
void write_buf_to_disk(struct hard_disk *hd, void *buf, u8 counts);
int harddisk_busy_wait(struct hard_disk *hd);
void hd_read_sector(struct hard_disk *hd, u32 lba, void *buf, u32 counts);
void hd_write_sector(struct hard_disk *hd, u32 lba, void *buf, u32 counts);

void hd_read_sectors(u32 lba, void *buf, u32 counts);
void hd_write_sectors(u32 lba, void *buf, u32 counts);

void disk_identify(struct hard_disk *hd);
void analysis_identify_info(struct hard_disk *hd, u16* hdinfo);
void harddisk_interrupt_wait(struct ide_channel *channel);

int disk_result(struct ide_channel *channel);

void bad_hd_rw(struct ide_channel *channel);

int set_current_hd(char *hd_name);

#endif /*_HARDDISK_H_*/

