#ifndef _CMOS_H
#define _CMOS_H

/**CMOS操作端口**/
#define cmos_index 0x70
#define cmos_data 0x71
/**CMOS中相关信息偏移*/
#define CMOS_CUR_SEC	0x0			//CMOS中当前秒?(BCD)
#define CMOS_ALA_SEC	0x1			//CMOS中?警秒?(BCD)
#define CMOS_CUR_MIN	0x2			//CMOS中当前分?(BCD)
#define CMOS_ALA_MIN	0x3			//CMOS中?警分?(BCD)
#define CMOS_CUR_HOUR	0x4			//CMOS中当前小?(BCD)
#define CMOS_ALA_HOUR	0x5			//CMOS中?警小?(BCD)
#define CMOS_WEEK_DAY	0x6			//CMOS中一周中当前天(BCD)
#define CMOS_MON_DAY	0x7			//CMOS中一月中当前日(BCD)
#define CMOS_CUR_MON	0x8			//CMOS中当前月?(BCD)
#define CMOS_CUR_YEAR	0x9			//CMOS中当前年?(BCD)
#define CMOS_DEV_TYPE	0x12		//CMOS中??器格式
#define CMOS_CUR_CEN	0x32		//CMOS中当前世?(BCD)

#define BCD_HEX(n)	((n >> 4) * 10) + (n & 0xf)  //BCD?十六?制

#define BCD_ASCII_first(n)	(((n<<4)>>4)+0x30)  //取BCD的个位并以字符?出,来自UdoOS
#define BCD_ASCII_S(n)	((n<<4)+0x30)  //取BCD的十位并以字符?出,来自UdoOS

/*
获得数据的端口
*/
unsigned int get_hour_hex();
unsigned int get_min_hex();
unsigned char get_min_hex8();
unsigned int get_sec_hex();
unsigned int get_day_of_month();
unsigned int get_day_of_week();
unsigned int get_mon_hex();
unsigned int get_year();



#endif

