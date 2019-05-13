/*
File:		kernel/cmos.c
Contains:	read cmos data
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <sys/arch.h>

unsigned char read_cmos (unsigned char p)
{
	unsigned char data;
	io_out8(cmos_index, p);
	data = io_in8(cmos_data);
	io_out8(cmos_index, 0x80);
	return data;
}

unsigned int get_hour_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_HOUR));
}

unsigned int get_min_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_MIN));
}
unsigned char get_min_hex8()
{
	return BCD_HEX(read_cmos(CMOS_CUR_MIN));
}
unsigned int get_sec_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_SEC));
}
unsigned int get_day_of_month()
{
	return BCD_HEX(read_cmos(CMOS_MON_DAY));
}
unsigned int get_day_of_week()
{
	return BCD_HEX(read_cmos(CMOS_WEEK_DAY));
}
unsigned int get_mon_hex()
{
	return BCD_HEX(read_cmos(CMOS_CUR_MON));
}
unsigned int get_year()
{
	return (BCD_HEX(read_cmos(CMOS_CUR_CEN))*100)+BCD_HEX(read_cmos(CMOS_CUR_YEAR))+1980;
}

#endif //_CONFIG_ARCH_X86_

