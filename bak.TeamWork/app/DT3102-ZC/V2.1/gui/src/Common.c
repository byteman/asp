#include "Common.h"
//#include <stdio.h>
// #include "SysParam.h"
//#include "usart1_init.h"
#include "sys_param.h"


#define LBPERKG    ((float)2.20462262) // 2.20462262
#define KGPERLB    ((float)0.45359237) // 0.45359237

extern SSYSSET gSysSet;
extern char* gcWet_Unit[];

// struct __FILE 
// { 
// 	int handle; 
// }; 
// FILE __stdout;         
// _sys_exit(int x) 
// { 
// 	x = x; 
// } 

/*******************************************************************************
	函数名：fputc
	输  入:
	输  出:
	功能说明：
	重定义putc函数，这样可以使用printf函数从串口1打印输出
*/
//static unsigned char putbuf[100];
// int fputc(int ch, FILE *f)
// {
//     #if 0
// 	/* Loop until the end of transmission */
// 	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
// 	/* Place your implementation of fputc here */
// 	/* e.g. write a character to the USART */
// 	USART_SendData(USART1, (uint8_t) ch);
//     #endif
//     static unsigned int len=0;
//     putbuf[len]=ch;
//     len++;
//     if(len>80) {
//         USART1_DMASendString(putbuf,len);
//         len = 0;
//     } else if(len>2) {
//         if(((putbuf[len-1] == '\r')||(putbuf[len-1] == '\n'))&&((putbuf[len-2] == '\r')||(putbuf[len-1] == '\n'))) {
//             USART1_DMASendString(putbuf,len);
//             len = 0;
//         }
//     }
// 	return ch;
// }

/*******************************************************************************
	函数名：fputc
	输  入:
	输  出:
	功能说明：
	重定义getc函数，这样可以使用scanff函数从串口1输入数据
*/
// int fgetc(FILE *f)
// {
// 	/* 等待串口1输入数据 */
// 	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
// 	return (int)USART_ReceiveData(USART1);
// }

float kg2lg(float wet_kg)
{
    return(float)(wet_kg*LBPERKG);
}


float kg2t(float wet_kg)
{
    return(wet_kg/1000);
}


float t2kg(float wet_t)
{
    return(wet_t*1000);
}

float CurVal2Kg(float wet)
{
    if(gSysSet.unit) {
        return t2kg(wet);
    } else {
        return wet;
    }
}

float Kg2CurVal(float wet_kg)
{
    if(gSysSet.unit) {
        return kg2t(wet_kg);
    } else {
        return wet_kg;
    }
}

char *GetCurUnitStr(void)
{
    return gcWet_Unit[gSysSet.unit];  //lxj
}


/*
 *  return:  0:kg  1:lb
 */
unsigned char GetCurUnit(void)
{
    return gSysSet.unit;
}


void SwitchUnit(void)
{
    if(gSysSet.unit) {
	   gSysSet.unit = 0;
	} else {
		gSysSet.unit = 1;
	}
// 	SysSetParamSave();
}


// /****************************************************************************
// * 名	称：void GetCpuIdHex(void)
// * 功	能：获取CPU芯片ID值16进制   
// * 入口参数：无
// * 出口参数：无
// * 说	明：6个u16的值存放CPUID
// 			u16 x[6]={0};
// ****************************************************************************/
// void GetCpuIdHex(char* HCpuId)
// {
// 	long CpuId[3]={0};

// 	CpuId[0] = (*(long*)(0x1fff7a10));
// 	CpuId[1] = (*(long*)(0x1fff7a14));
// 	CpuId[2] = (*(long*)(0x1fff7a18));
// 	
// 	HCpuId[0] = (char)(CpuId[0]>>24);
// 	HCpuId[1] = (char)(CpuId[0]>>16); 
// 	HCpuId[2] = (char)(CpuId[0]>>8);
// 	HCpuId[3] = (char)(CpuId[0]);

// 	HCpuId[4] = (char)(CpuId[1]>>24);
// 	HCpuId[5] = (char)(CpuId[1]>>16); 
// 	HCpuId[6] = (char)(CpuId[1]>>8);
// 	HCpuId[7] = (char)(CpuId[1]);

// 	HCpuId[8] = (u8)(CpuId[2]>>24);
// 	HCpuId[9] = (u8)(CpuId[2]>>16); 
// 	HCpuId[10] = (u8)(CpuId[2]>>8);
// 	HCpuId[11] = (u8)(CpuId[2]);
// }




