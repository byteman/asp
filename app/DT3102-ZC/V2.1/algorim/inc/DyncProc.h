#ifndef __DYNCPROC_H_
#define __DYNCPROC_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct {
    /*
     *  输入
     */
    int    iPowerOnDelay;
    float  fWetUp;
    float  fWetDown;
    
    /*
     *  输出
     */
	unsigned char ucAxelValid;  //  秤台上有轴  0: 秤台空       1:秤台有轴
	unsigned int  uiWaitCnt;    //  等待计数(时间)
	unsigned char ucUpDir;      //  轴上称位置    0: 未知         1:AD1      2:AD2
	unsigned char ucDownDir;    //  轴下称位置    0: 未知         1:AD1      2:AD2
	unsigned int  uiRunCnt;     //  过轴时间(时间)
	float fWet;                 //  轴重
	float fWetReliab;	        //  计量可靠度 wet1/(wet1 + wet2)
    
    /*
     *  私有
     */
    int     iPowerOnIndex;      //  开机延时
    int     iTimetick;          //  过车周期计时
    int     iScaleTime_On;      //  上秤时间
    int     iScaleTime_Down;    //  下秤时间
    char    cScaleTimeLock_On;  //  上称时间锁定标志
    char    cScaleTimeLock_Down;//  下秤时间锁定标志
    float   fMaxWet_AD1;        //  第一路最大重量
    float   fMaxWet_AD2;        //  第二路最大重量
    int     iMaxWetTime_AD1;    //  第一路最大重量时间
    int     iMaxWetTime_AD2;    //  第二路最大重量时间
    float   fWet_AD1Top;        //  第一路峰值时总重
    float   fWet_AD2Top;        //  第二路峰值时总重
    float   fWetBuf[800];       //  称重缓存
    int     iSteadyWetIndex;
    float   fSteadyWetBuf[600]; //  稳定的称重数据缓存
    int     iSteadyCount;
    float   fSteadyWet;         //  稳定的重量
    
    float   fAD1Buf[20];        //  第一路AD上称时间
    float   fAD2Buf[20];        //  第一路AD上称时间
}sDyncProc;

//取参数宏
#define SDYNC(x)	(*((sDyncProc*)x))

/*
 * 功能:    动态处理
 */
char  DyncProcInit(void* pDecb);
float DyncProc(void* pDecb, float * pInArr, int nInCount);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
