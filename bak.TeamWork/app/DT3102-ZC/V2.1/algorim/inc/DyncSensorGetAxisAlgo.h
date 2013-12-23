#ifndef SENSOR_ALEX_PARSER_H
#define SENSOR_ALEX_PARSER_H

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef int (*AlexEventProc)(void* pADP);

#define	MAX_ALEX_NUM	10
typedef struct __TAlexArr 
{
	int num; 	//轴数
	float fSpeed; //速度
	int width[MAX_ALEX_NUM];  //平台的长度
	int avg[MAX_ALEX_NUM];  //平台长度内的平均值
	
}TAlexArr;

//3280
typedef struct {

	int		mFixAlexMode ; //补轴模式选择;
	int  		m_LimitK;		//删除轴的时候，采用的0.05倍的轴重系数3
	int		m_rate;			//每个平台的波动值的0.5倍系数  2
	int		m_SumLen;		//表示计算速度点数增加的0.1倍   8

	float  fWetUp;
	float  fWetDown;

	int 	 m_CmpSpeddPT;
	int    iPowerOnIndex;      //  开机延时
	int    iPowerOnDelay;

	///输出结果
	int	nAlexNum;			//表示轴数
	int   nAlexINorRemove;	// 1 ---加轴或0 --减轴
	int	nAlexWet;		//轴重
	int	nAlexPulseWith; //轴有效时间
	int	nAlexMaxWet;   //轴信号触发期间，采集的整秤最大值。
	float CarSpeed ;	//速度
	
	AlexEventProc pDecb;
	  
}sSoftAlexDyncProc;

//取参数宏
#define SSOFTALEXDYNC(x)	(*((sSoftAlexDyncProc*)x))


/*
获取轴信号处理结果
*/
void   SoftAlexDyncProcResult(TAlexArr* alexarr);

/*
需要在光幕断尾后调用。
结束轴信号处理
*/
void  SoftAlexDyncProcStop(void);


/*
 * 功能:    动态处理
 */
char  SoftAlexDyncProcInit(void* pDecb);

/*
分析轴的方向，进轴还是倒轴
axisAd : 轴传感器AD
bigScalerKg: 大称实时重量
*/
float SoftAlexDyncProc(void* pDecb, float * pInArr, int nInCount);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif

