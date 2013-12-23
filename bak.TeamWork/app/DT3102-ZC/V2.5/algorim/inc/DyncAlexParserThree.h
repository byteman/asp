#ifndef ALEX_THREE_PARSER_H
#define ALEX_THREE_PARSER_H

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef int (*AlexEventProc)(void* pADP);

typedef struct {

/////////////////////////////////////
	
	float  fWetUp;
	float  fWetDown;
	
	float  fDownAlexWetUp;
	float  fDownAlexWetDown;
	
	int	 mSpeedLen;
	int    iPowerOnIndex;      //  开机延时
	int    iPowerOnDelay;
	
/////////////////////////////////////	
	int	nAlexValid;
	int 	nPulseWidth;
	int 	nUpFlag;			//轴有效开始标志
	int 	nUpCnt;
	int	nUPkeepCnt;
	int	nDownkeepCnt;
	int	nDownCnt;
	float fWet;                 //  轴重
	float fWetReliab;	        //  计量可靠度 wet1/(wet1 + wet2)
////////////////////
	int	nDownAlexAlexValid;
	int 	nDownAlexPulseWidth;
	int 	nDownAlexUpFlag;			//轴有效开始标志
	int 	nDownAlexUpCnt;
	int	nDownAlexDownCnt;
	int	nDownAlexUPkeepCnt;
	int	nDownAlexDownkeepCnt;
	float	nDownAlexMaxWet;
////////////////////
	float mScaleWet ;
	float mUpScaleAlex1Wet ;
	float mUpScaleAlex2Wet ;
	float mDownScaleAlex1Wet ;
	
/////////////////输出结果//////////
	int	nAlexNo;			//表示是哪个轴识别器 1---前基础上的轴 2 ----上称端的轴 3 ----下称端的轴
	int   nAlexINorRemove;	// 1 ---加轴或0 --减轴
	int	nAlexWet;		//轴重
	int	nAlexPulseWith; //轴有效时间
	int	nAlexMaxWet;   //轴信号触发期间，采集的整秤最大值。
	float fSpeed;			//轴速度
	
	AlexEventProc pADP;
	  
}sAlexThreeDyncProc;

//取参数宏
#define SALEXTHREEDYNC(x)	(*((sAlexThreeDyncProc*)x))


/*
轴信号处理模块初始化
*/
/*
 * 功能:    动态处理
 */
char  AlexDyncProcThreeInit(void* pDecb);

/*
分析轴的方向，进轴还是倒轴
axisAd : 轴传感器AD
bigScalerKg: 大称实时重量
*/
float AlexDyncThreeProc(void* pDecb, float * pInArr, int nInCount);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif


