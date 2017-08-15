#ifndef __TOOLS_H
#define __TOOLS_H
#include "stdio.h"	
#include "sys.h" 


struct sTime//时间结构体
{
	short year;
	short month;
	short day;
	short hour;
	short min;
	short sec;
	short msec;
};

void timeStructInit(void);
void getTimeNow(void);





#endif

