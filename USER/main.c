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

/////////////////////////UCOSII�����ջ����///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//���������ջ�ռ�	
OS_STK START_TASK_STK[START_STK_SIZE];
//�������ӿ�
void start_task(void *pdata);	
 			   
//GPRS�쳣����
//�����������ȼ�
#define GPRS_TASK_PRIO       			3 
//���������ջ��С
#define GPRS_STK_SIZE  		    		64
//���������ջ�ռ�	
OS_STK LED1_TASK_STK[GPRS_STK_SIZE];
//�������ӿ�
void gprs_task(void *pdata);


//��ʪ������
//�����������ȼ�
#define TEMPHUMI_TASK_PRIO        9
//���������ջ��С
#define TEMPHUMI_STK_SIZE         64
//���������ջ�ռ�
OS_STK TEMPHUMI_TASK_STK[TEMPHUMI_STK_SIZE];
//�������ӿ�
void temphumi_task(void *pdata);


//��ѹ����
#define AIRPRE_TASK_PRIO          5
//�����������ȼ�
#define AIRPRE_STK_SIZE           64
//���������ջ�ռ�
OS_STK AIRPRE_TASK_STK[AIRPRE_STK_SIZE];
//�������ӿ�
void airpre_task(void *pdata);


//��������
//�����������ȼ�
#define NOISE_TASK_PRIO           6
//���������ջ��С
#define NOISE_STK_SIZE            64
//���������ջ�ռ�
OS_STK NOISE_TASK_STK[NOISE_STK_SIZE];
//�������ӿ�
void noise_task(void *pdata);



//���������ݴ�ӡ����
//�����������ȼ�
#define PRINT_TASK_PRIO           7
//���������ջ��С
#define PRINT_STK_SIZE            512
//���������ջ�ռ�
OS_STK PRINT_TASK_STK[PRINT_STK_SIZE];
//�������ӿ�
void print_task(void *pdata);

//GPRS��ʱ��������
//�����������ȼ�                  
#define GPRSDELAY_TASK_PRIO        8
//���������ջ��С
#define GPRSDELAY_STK_SIZE         64
//���������ջ�ռ�
OS_STK  GPRSDELAY_TASK_STK[GPRSDELAY_STK_SIZE];
//�������ӿ�
void gprsdelay_task(void *pdata);


//����ֵ�ź�����
OS_FLAG_GRP * flags_value;
//�����ź���
OS_EVENT * sem_airpre;


int main(void)
{
	delay_init();	     //��ʱ��ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ�
	uart_init(9600);  //��ʼ������1
	uart_init3(115200); //��ʼ������3
	while(DHT11_Init())//��ʼ��DHT11
	{
		printf("Cann't init\r\n");
	}
	//DHT11_Read_Data(&temperature,&humidity);
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
}

 	  
//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	u8 err;
	pdata = pdata; 
	flags_value=OSFlagCreate(0,&err); //�����ź�����
	sem_airpre=OSSemCreate(0);//�����ź���
	//Ԥ��һ����ʪ�ȵĶ�ȡ������ȫ�ֱ���
	
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������
	timeStructInit();			//��ʼ��ģ��ʱ���
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
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
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

//GPRS����3
void gprs_task(void *pdata)
{
	u16 flags;	
	u8 err;	
	while(1)
	{
		flags=OSFlagPend(flags_value,0X0007,OS_FLAG_WAIT_SET_ANY,0,&err);//�ȴ��ź���
		if(flags==0)
			printf("gprs send \r\n");
		else
		{
			if(flags&0X0002)
			{
				//printf("gprs send shock\r\n");
				//�����񶯱�־+ȫ��/���������������
				OSFlagPost(flags_value,0X0002,OS_FLAG_CLR,&err);
			}
			if(flags&0X0004)
			{
				//printf("gprs send noise\r\n");
				OSFlagPost(flags_value,0X0004,OS_FLAG_CLR,&err);
				//��������+��ȡ����������
			}
			if(flags&0X0001)
			{
				printf("gprs send height \r\n");
				OSFlagPost(flags_value,0X0001,OS_FLAG_CLR,&err);
				//���͸߶�+��ȡ����������
			}
		}
		delay_ms(10);
	}
}


//TEMPHUMI����9
void temphumi_task(void *pdata)
{
	unsigned char temperature;
	unsigned char humidity;  
	while(1)
	{
		//��ȡ��ʪ������
		printf("Start read temperature and humidity\r\n");
		DHT11_Read_Data(&temperature,&humidity);
		printf("temperature is: %d,humidity is %d ",(int)temperature,(int)humidity);
		OSTimeDlyHMSM(0,0,2,0);
	}
}



//NOISE����6
void noise_task(void *pdata)
{
	u8 err;
	while(1)
	{
		//����������������ȡ����ֵ
		
		if(1)//�����������ĳһ��ֵ
		{
			//printf("noise happened \r\n");
			OSFlagPost(flags_value,1<<NOISE_FLAG,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
		}
		delay_ms(100);
	}
}

//AIRPRE����5
void airpre_task(void *pdata)
{
	u8 err1;
	u8 err2;
	while(1)
	{	//����SD�������ź���
		OSSemPend(sem_airpre,0,&err1);
		//���ô���ѹ��������ȡ�߶�ֵ
		printf("air pressure get \r\n");
		OSFlagPost(flags_value,1<<AIRPRE_FLAG,OS_FLAG_SET,&err2);//���ö�Ӧ���ź���Ϊ1
		delay_ms(10);
	}
}

//PRINT���񣨼���sd������7
void print_task(void *pdata)
{
	INT32U i;
	u8 err;
	int accSum;//�ѵ�ǰ�����Ĵ�С����accSum
	int variance;//��ʼ������
	float z;
	struct sAcc tempAcc[buffersize];//buffersize�ݶ�300���Ӵ���SD���ٶ��޸�
	int count=0;//count������������С����ֵ�Ĵ���������һ���������ж�Ϊͣ��
	while(1)
	{
		variance=0;
		memcpy(&tempAcc[0],&stcAcc[0],buffersize*sizeof(tempAcc[0]));
		accSum=accSize;//��accSize����accSum
		accSize=0;//����accSize
		printf("accSum is :%d\r\n",accSum);
		for(i=0;i<accSum;++i)//ѭ���ж��Ƿ������ݳ�����ֵ��������z�᷽��
		{
			if(0)//�ж�x y����ٶ��Ƿ񳬹���ֵ
			{
				//����ȫ�ֱ����������䣨δ��ɣ�
				
				OSFlagPost(flags_value,1<<AIRPRE_FLAG,OS_FLAG_SET,&err);//������ֵ�����ö�Ӧ���ź���Ϊ1
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
		if(variance<1000&&count<30)//�������С��ĳһ��ֵ˵���ھ�ֹ/����
		{
			printf("count is:%d\r\n",count);
			++count;
			OSSemPost(sem_airpre);//�����ź�����ȡ�߶�
			delay_ms(50);
		}
		else//����SD���洢
		{
			if(variance>1000)
				count=0;
			OSSchedLock();//��ֹ����
			//��ȡʱ���������������SD��
			getTimeNow();
			
			OSSchedUnlock();//��������
		}
		
		OSTaskStkChk(PRINT_TASK_PRIO,&StackBytes);
		printf("use:%d  free:%d\r\n",StackBytes.OSUsed,StackBytes.OSFree);
		delay_ms(50);
	}
}

//gprsdelay����8
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

