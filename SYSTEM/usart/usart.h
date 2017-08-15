#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
#include "tools.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2011/6/14
//�汾��V1.4
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
#define EN_USART3_RX 			1
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8 USART_RX_BUF3[USART_REC_LEN];
extern u8 rec[11];
extern u16 USART_RX_STA;         		//����״̬���	
extern u16 USART_RX_STA3;
//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
void uart_init3(u32 bound);
#define buffersize 50
void CopeSerial3Data(unsigned char ucData);
struct sAcc//���ٶȽṹ��
{
	short a[3];
	//struct sTime time;
};
struct sGyro//���ٶȽṹ��
{
	short w[3];
	//struct sTime time;
};
struct sAngle//�ǶȽṹ��
{
	short g[3];
	//struct sTime time;
};

#endif


