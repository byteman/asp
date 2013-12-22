#ifndef _DATAMANAGER_H_	
#define _DATAMANAGER_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
#include "Common.h"
	
	
	
#define sFLASH_MAXADDR            0x400000  //2012.11.22 lxj

typedef enum
{
	DM_WEIGHTDATASAVE,		   //记录
	DM_UPLOADACK			   //数据上传成功的应答
} DataManagerID;				//DataManager消息ID号

typedef struct _DataManagerMsgInfo				//DataManager消息信息
{
	u16	 DataManagerID;		//DataManager消息号
	u32  nParam;
} DataManagerInfo, *LPDataManagerMsgInfo;


#define  DATAHEADLEN            33
#define  DATAPACKMAXLEN         (5*AXLECOUNTMAX+2*AXLEGROUPCOUNTMAX-2)
#define  CalcDataPackLen(axle,axlegroup)   (5*axle+3*axlegroup-2)

typedef struct PACKED
{
    //数据头
    u8   ucDataType;                 //数据类型          0xaa 称重数据
    u8   ucSync;                     //同步标志          0xff 未上传 0x00 已上传
    u32  ulNo;                       //序号
    u16  usTimeYear;                 //称重时间  年
    u8   ucTimeMonth;                //称重时间  月
    u8   ucTimeDay;                  //称重时间  日
    u8   ucTimeHour;                 //称重时间  时
    u8   ucTimeMin;                  //称重时间  分
    u8   ucTimeSec;                  //称重时间  秒
    u32  ulWeitht;                   //总重
    u8   ucOverLoad;                 //超载
	u8   ucDir;                      //0 未知 1正向(A->B) 2反向(B->A)
    u16  usSpeed;                    //车速              权值: 0.1km/h
    s16  sAcce;                      //加速度            权值: 0.1 km/h2
    u8   ucAxleCount;                //轴数
    u8	 ucAxleGroupCount;           //轴组数
    u32  ulDataCrc;                  //数据区CRC
    u32  ulHeadCrc;                  //数据头CRC

    //数据区
	unsigned short	usAxleWet[AXLECOUNTMAX];			// 轴重
	unsigned char	ucAxleType[AXLECOUNTMAX];			// 轴型 0 : 未知  1: 单轮 2: 双轮
	unsigned short  usAxleDis[AXLECOUNTMAX-1];			// 轴距 m
	unsigned short	usAxleGroupWet[AXLEGROUPCOUNTMAX];		// 轴组重
	unsigned char	ucAxleGroupType[AXLEGROUPCOUNTMAX];	// 轴组类型
} SVehDataPack;



u8   SaveWetData(SVehWeight* wet);
void DataFlashCreat(u8 nPrio);
void DataManagerInit(void);		//初始化
void DataManagerProcess(void);	//业务数据管理队列消息挂起等待
SVehDataPack* GetWetDataByAddr(u32 addr);
SVehDataPack* GetWetDataByNo(u32 no);

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif
