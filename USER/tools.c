#include "sys.h"
#include "tools.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif

struct sTime now;
//ʱ��ģ�⣬time struct��ʼ������
void timeStructInit()
{
	now.year=2017;
	now.month=8;
	now.day=7;
	now.hour=17;
	now.min=6;
	now.sec=30;
	now.msec=0;
}
//ʱ��ģ�⣬��ȡ��ǰʱ�亯��
void getTimeNow()
{
	INT32U timeUsed=OSTimeGet();
	now.msec+=timeUsed%1000;
	if(now.msec>=1000)
	{
		now.sec=now.sec+now.msec/1000;
		now.msec=now.msec%1000;
	}
	if(now.sec>=60)
	{
		now.sec-=60;
		++now.min;
	}
	if(now.min>=60)
	{
		now.min-=60;
		++now.hour;
	}
	if(now.hour>=24)
	{
		now.hour-=24;
		++now.day;
	}
}



