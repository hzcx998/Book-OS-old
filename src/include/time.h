#ifndef _TIME_H_
#define _TIME_H_

struct time
{
	int32_t second;
	int32_t minute;
	int32_t hour;
	int32_t day;
	int32_t month;
	int32_t year;
	int32_t week_day;
	int32_t year_day;
	int32_t is_dst;
};

struct time *gettime(struct time *tm);

#endif  /*_TIME_H_*/