#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	
#include "includes.h"
#include "tools.h"
#include "dht11.h"
extern struct sAcc stcAcc[buffersize];
extern struct sGyro stcGyro[buffersize];
extern struct sAngle stcAngle[buffersize];
extern unsigned int accSize;
extern struct sTime now;


OS_STK_DATA  StackBytes;
#define AIRPRE_FLAG 0
#define SHOCK_FLAG  1
#define NOISE_FLAG  2

/////////////////////////UCOSII任务堆栈设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//创建任务堆栈空间	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数接口
void start_task(void *pdata);	
 			   
//GPRS异常任务
//设置任务优先级
#define GPRS_TASK_PRIO       			3 
//设置任务堆栈大小
#define GPRS_STK_SIZE  		    		64
//创建任务堆栈空间	
OS_STK LED1_TASK_STK[GPRS_STK_SIZE];
//任务函数接口
void gprs_task(void *pdata);


//温湿度任务
//设置任务优先级
#define TEMPHUMI_TASK_PRIO        9
//设置任务堆栈大小
#define TEMPHUMI_STK_SIZE         64
//设置任务堆栈空间
OS_STK TEMPHUMI_TASK_STK[TEMPHUMI_STK_SIZE];
//任务函数接口
void temphumi_task(void *pdata);


//气压任务
#define AIRPRE_TASK_PRIO          5
//设置任务优先级
#define AIRPRE_STK_SIZE           64
//创建任务堆栈空间
OS_STK AIRPRE_TASK_STK[AIRPRE_STK_SIZE];
//任务函数接口
void airpre_task(void *pdata);


//噪声任务
//设置任务优先级
#define NOISE_TASK_PRIO           6
//设置任务堆栈大小
#define NOISE_STK_SIZE            64
//创建任务堆栈空间
OS_STK NOISE_TASK_STK[NOISE_STK_SIZE];
//任务函数接口
void noise_task(void *pdata);



//陀螺仪数据打印任务
//设置任务优先级
#define PRINT_TASK_PRIO           7
//设置任务堆栈大小
#define PRINT_STK_SIZE            512
//创建任务堆栈空间
OS_STK PRINT_TASK_STK[PRINT_STK_SIZE];
//任务函数接口
void print_task(void *pdata);

//GPRS延时发送任务
//设置任务优先级                  
#define GPRSDELAY_TASK_PRIO        8
//设置任务堆栈大小
#define GPRSDELAY_STK_SIZE         64
//创建任务堆栈空间
OS_STK  GPRSDELAY_TASK_STK[GPRSDELAY_STK_SIZE];
//任务函数接口
void gprsdelay_task(void *pdata);


//设置值信号量集
OS_FLAG_GRP * flags_value;
//设置信号量
OS_EVENT * sem_airpre;


int main(void)
{
	delay_init();	     //延时初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	LED_Init();		  	 //初始化与LED连接的硬件接口
	uart_init(9600);  //初始化串口1
	uart_init3(115200); //初始化串口3
	while(DHT11_Init())//初始化DHT11
	{
		printf("Cann't init\r\n");
	}
	//DHT11_Read_Data(&temperature,&humidity);
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
}

 	  
//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	u8 err;
	pdata = pdata; 
	flags_value=OSFlagCreate(0,&err); //创建信号量集
	sem_airpre=OSSemCreate(0);//创建信号量
	//预留一次温湿度的读取，存入全局变量
	
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右
	timeStructInit();			//初始化模拟时间戳
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 	OSTaskCreate(gprs_task,(void *)0,(OS_STK*)&LED1_TASK_STK[GPRS_STK_SIZE-1],GPRS_TASK_PRIO);//GPRS						   
	OSTaskCreateExt(print_task,(void *)0,(OS_STK*)&PRINT_TASK_STK[PRINT_STK_SIZE-1],PRINT_TASK_PRIO,PRINT_TASK_PRIO,
		(OS_STK*)&PRINT_TASK_STK[0],PRINT_STK_SIZE,(void*)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);//print
	OSTaskCreateExt(temphumi_task,(void *)0,(OS_STK*)&TEMPHUMI_TASK_STK[TEMPHUMI_STK_SIZE-1],TEMPHUMI_TASK_PRIO,TEMPHUMI_TASK_PRIO,
		(OS_STK*)&TEMPHUMI_TASK_STK[0],TEMPHUMI_STK_SIZE,(void*)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);//temphumi

	OSTaskCreateExt(noise_task,(void *)0,(OS_STK*)&NOISE_TASK_STK[NOISE_STK_SIZE-1],NOISE_TASK_PRIO,NOISE_TASK_PRIO,
		(OS_STK*)&NOISE_TASK_STK[0],NOISE_STK_SIZE,(void*)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);//noise
	OSTaskCreateExt(airpre_task,(void *)0,(OS_STK*)&AIRPRE_TASK_STK[AIRPRE_STK_SIZE-1],AIRPRE_TASK_PRIO,AIRPRE_TASK_PRIO,
		(OS_STK*)&AIRPRE_TASK_STK[0],AIRPRE_STK_SIZE,(void*)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);//airpre
	OSTaskCreateExt(gprsdelay_task,(void *)0,(OS_STK*)&GPRSDELAY_TASK_STK[GPRSDELAY_STK_SIZE-1],GPRSDELAY_TASK_PRIO,GPRSDELAY_TASK_PRIO,
		(OS_STK*)&GPRSDELAY_TASK_STK[0],GPRSDELAY_STK_SIZE,(void*)0,OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);//gprs_delay
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

//GPRS任务3
void gprs_task(void *pdata)
{
	u16 flags;	
	u8 err;	
	while(1)
	{
		flags=OSFlagPend(flags_value,0X0007,OS_FLAG_WAIT_SET_ANY,0,&err);//等待信号量
		if(flags==0)
			printf("gprs send \r\n");
		else
		{
			if(flags&0X0002)
			{
				//printf("gprs send shock\r\n");
				//发送振动标志+全局/邮箱的陀螺仪数据
				OSFlagPost(flags_value,0X0002,OS_FLAG_CLR,&err);
			}
			if(flags&0X0004)
			{
				//printf("gprs send noise\r\n");
				OSFlagPost(flags_value,0X0004,OS_FLAG_CLR,&err);
				//发送噪声+读取陀螺仪数据
			}
			if(flags&0X0001)
			{
				printf("gprs send height \r\n");
				OSFlagPost(flags_value,0X0001,OS_FLAG_CLR,&err);
				//发送高度+读取陀螺仪数据
			}
		}
		delay_ms(10);
	}
}


//TEMPHUMI任务9
void temphumi_task(void *pdata)
{
	unsigned char temperature;
	unsigned char humidity;  
	while(1)
	{
		//读取温湿度数据
		printf("Start read temperature and humidity\r\n");
		DHT11_Read_Data(&temperature,&humidity);
		printf("temperature is: %d,humidity is %d ",(int)temperature,(int)humidity);
		OSTimeDlyHMSM(0,0,2,0);
	}
}



//NOISE任务6
void noise_task(void *pdata)
{
	u8 err;
	while(1)
	{
		//调用噪声驱动，获取噪声值
		
		if(1)//如果噪声超过某一阈值
		{
			//printf("noise happened \r\n");
			OSFlagPost(flags_value,1<<NOISE_FLAG,OS_FLAG_SET,&err);//设置对应的信号量为1
		}
		delay_ms(100);
	}
}

//AIRPRE任务5
void airpre_task(void *pdata)
{
	u8 err1;
	u8 err2;
	while(1)
	{	//请求SD卡发送信号量
		OSSemPend(sem_airpre,0,&err1);
		//调用大气压驱动，获取高度值
		printf("air pressure get \r\n");
		OSFlagPost(flags_value,1<<AIRPRE_FLAG,OS_FLAG_SET,&err2);//设置对应的信号量为1
		delay_ms(10);
	}
}

//PRINT任务（即存sd卡任务）7
void print_task(void *pdata)
{
	INT32U i;
	u8 err;
	int accSum;//把当前读到的大小赋给accSum
	int variance;//初始化方差
	float z;
	struct sAcc tempAcc[buffersize];//buffersize暂定300，视存入SD卡速度修改
	int count=0;//count用来计数方差小于阈值的次数，超过一定次数则判断为停靠
	while(1)
	{
		variance=0;
		memcpy(&tempAcc[0],&stcAcc[0],buffersize*sizeof(tempAcc[0]));
		accSum=accSize;//把accSize赋给accSum
		accSize=0;//清零accSize
		printf("accSum is :%d\r\n",accSum);
		for(i=0;i<accSum;++i)//循环判断是否有数据超过阈值，并计算z轴方差
		{
			if(0)//判断x y轴加速度是否超过阈值
			{
				//设置全局变量或者邮箱（未完成）
				
				OSFlagPost(flags_value,1<<AIRPRE_FLAG,OS_FLAG_SET,&err);//超过阈值，设置对应的信号量为1
				continue;
			}
			z=(float)tempAcc[i].a[2]/32768*16;
			printf("z is:%.4f \r\n",z);
			if(z!=0)
				variance+=(int)((z-0.99)*1000)*(int)((z-0.99)*1000);
		}
		if(accSum!=0)
			variance=variance/accSum;
		printf("variance is :%d\r\n",variance);
		if(variance<1000&&count<30)//如果方差小于某一阈值说明在静止/匀速
		{
			printf("count is:%d\r\n",count);
			++count;
			OSSemPost(sem_airpre);//发送信号量读取高度
			delay_ms(50);
		}
		else//进行SD卡存储
		{
			if(variance>1000)
				count=0;
			OSSchedLock();//禁止调度
			//获取时间戳，将参数存入SD卡
			getTimeNow();
			
			OSSchedUnlock();//开启调度
		}
		
		OSTaskStkChk(PRINT_TASK_PRIO,&StackBytes);
		printf("use:%d  free:%d\r\n",StackBytes.OSUsed,StackBytes.OSFree);
		delay_ms(50);
	}
}

//gprsdelay任务8
void gprsdelay_task(void *pdata)
{
	while(1)
	{
		OSTimeDlyHMSM(0,30,0,0);
		while(1)
		{

		}
	}
}

