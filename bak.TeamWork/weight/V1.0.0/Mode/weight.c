#include "weight.h"
#include "drvproc.h"

#define _WEIGHT_DEBUG		1

#define	DTOS_MAJOR_VER	1
#define	DTOS_MAIN_VER		0
#define	DTOS_MINOR_VER	2
#define	DTOS_DEBUG_VER	2

/*! \brief 每个称台最多能支持的AD通道数 */
#define MAX_AD_CHANLE 6  /*!< 最大AD通道数 */

/*! \brief 最多能申请到的称台个数 */
#define MAX_SCALER 2 /*!< 最大称个数 */


/*!
\brief 
ADC采集通道，表示一个ADC转换通道
*/
typedef struct _sAdcProcChannel
{
	/*! \brief AD采集*/
	void*		pAdcGet_UserData;				/*!<AD采集用户参数 */
	pInitFunc	pAdcGetInit;					/*!<AD采集初始化 */
	pAdcGetFun	pAdcGet;						/*!<AD采集函数 */
	long		nOrgAd;							/*!<输入,当前原始AD值 */

	/*! \brief 故障检测*/
	void*				pAvc_UserData;			/*!<异常检测参数*/
	pInitFunc			pAdcValidCheckInit;	    /*!<异常检测初始化*/
	pAdcValidCheckFunc	pAdcValidCheck;	        /*!<异常检测函数*/
	int					nValidState;			/*!<故障检测后输出值*/

	/*! \brief 分频*/
	int					bDivEnable;			/*!<是否使能分频*/	
	unsigned short	nDivIndex;			/*!<分频索引*/	
	unsigned short	nDivMax;			/*!<分频系数*/	
	float				fDivSum;			/*!<分频累计和*/		
	float				fDivValue;			/*!<分频结果*/	

	/*! \brief 初级滤波*/
	void*		pFcb_UserData;					/*!<滤波参数*/
	pInitFunc	pFilterInit;					/*!<滤波初始化函数*/
	pFilterFunc pFilter;						/*!<滤波函数*/
	float		fFilterValue;					/*!<滤波输出*/

	/*! \brief 标准化*/
	void*				pScb_UserData;			/*!<标准化参数*/
	pInitFunc			pAdcStandlizeInit;		/*!<标准化初始化函数*/
	pAdcStandlizeFunc	pAdcStandlize;			/*!<标准化函数*/
	float				fStandValue;			/*!<标准化输出结果*/

	/*! \brief 归一化*/
	void*				pNcb_UserData;			/*!<归一化参数*/
	pInitFunc			pAdcNormalizeInit;		/*!<归一化初始化函数*/
	pAdcNormalizeFunc	pAdcNormalize;			/*!<归一化函数*/
	float				fNormalizeValue;		/*!<归一化输出，给静态秤用*/

	/*! \brief 动态处理中的零位处理 */
	void*			pZcb_UserData;				/*!<零位处理参数*/
	pInitFunc		pZeroTraceInit;				/*!<零位处理初始化函数*/
	pZeroTraceFunc	pZeroTrace;					/*!<零位处理函数*/
	pSetZeroFunc pSetZero;					/*!<置零处理函数*/
	float			fZerolizeValue;				/*!<零位处理输出，给动态称用*/
}sAdcProcChannel;


/*!
\brief 静态秤	
*/
typedef  struct _sStaticScalerImpl
{

	/*! \brief 输入，静态处理的输入*/
	float InArr[MAX_AD_CHANLE];
	int nInCount;		/*!<InArr中的数据个数*/

	/*! \brief 静态求值*/   
	void*			 pTcb_UserData;		/*!<需要注册的静态抽样函数*/
	pInitFunc		 pStaticTotalInit;		/*!<需要注册的静态处理函数*/
	pStaticTotalFunc pStaticTotal;			/*!<静态求值函数*/
	float			 fTotalValue;          /*!<静态求值结果*/

	/*! \brief 静态抽样*/
	void*			pDscb_UserData;		/*!<静态抽样参数*/
	pInitFunc		pSampleInit;		/*!<需要注册的静态抽样初始化函数*/
	pSampleFunc		pSample;			/*!<需要注册的静态抽样函数*/
	float			fSampleValue;       /*!<静态求值结果*/

	/*! \brief 静态滤波*/
	void*			pFcb_UserData;				/*!<滤波参数*/
	pInitFunc		pFilterInit;	            /*!<滤波初始化函数*/
	pFilterFunc		pFilter;			     /*!< 滤波函数*/
	float			fFilterValue;                 /*!< 滤波输出*/

	/*! \brief 零位跟踪*/

	void*			pZtcb_UserData;				/*!<零位跟踪参数*/
	pInitFunc		pZeroTraceInit;					/*!<零位跟踪初始化函数*/
	pZeroTraceFunc	pZeroTrace;		             /*!<零位跟踪函数*/
	pSetZeroFunc pSetZero;					/*!<置零处理函数*/		
	float			fZeroTraceValue;						 /*!<零位跟踪输出结果*/

	/*! \brief 状态判定*/

	void*			pSccb_UserData;								 /*!<状态判定参数*/
	pInitFunc		pStateCheckInit;					/*!<状态判定初始化函数*/
	pStateCheckFunc pStateCheck;		         /*!<状态判定函数*/
	int				nState;						             /*!<状态判定输出结果*/

	/*! \brief 分度值处理*/

	float			fStaticDrv;						    /*!<静态称重分度值*/
	float			fStaticDrvValue;						    /*!<静态分度值处理结果*/

} sStaticScalerImpl;

/*!
\brief 
动态秤
*/
typedef  struct _sDyncScalerImpl
{
	//输入，动态处理的输入
	float InArr[MAX_AD_CHANLE];
	int nInCount;

	//动态求值
	void* pDecb;
	pDyncProcInitFunc pDyncProcInit;            /*!<动态处理初始化*/
	pDyncProcFunc pDyncProc;                    /*!<动态处理函数*/
	float fDyncValue;                           /*!<动态处理结果   */
	float fDyncDrv;								/*!<动态称重分度值*/
	float fDyncDrvValue;						/*!<动态分度值处理结果*/
} sDyncScalerImpl;


/*! \brief 称对象描述 */
typedef  struct _sScaler
{
	/*!< 是否使用 */
	short bUsed;

	/*!< 是否运行 */
	short bRun;	

	/*!< 输入，AD通道的输入*/
	sAdcProcChannel ArrAdcProcChannel[MAX_AD_CHANLE];
	int nAdcCount;

	/*!< 静态处理 */
	sStaticScalerImpl StaticScalerImpl;

	/*!< 动态处理 */
	sDyncScalerImpl DyncScalerImpl;
} sScaler;

//取秤台参数
#define SCALER(hs)	((sScaler*)(hs))


/*! \brief 测试句柄是否有效 */
#ifdef _WEIGHT_DEBUG
#define CHECK_HANDLE_VALID(hs) do \
{  \
	if(hs == HSCALER_INVALID) return ERR_INVALID_VAR; \
} while (0)
#else
#define CHECK_HANDLE_VALID(hs)
#endif

/*! \brief 测试通道和句柄是否有效 */
#ifdef _WEIGHT_DEBUG
#define CHECK_CHANLE_VALID(hs, ch) do \
{  \
	if(hs == HSCALER_INVALID) return ERR_INVALID_VAR; \
	if(ch >= SCALER(hs)->nAdcCount) return ERR_INVALID_VAR; \
} while (0)
#else
#define CHECK_CHANLE_VALID(hs, ch)
#endif


//内部函数声明
static void InitAdcChanle(sAdcProcChannel* pAdc);
static void InitStaticScalerParam(sStaticScalerImpl* ssi,int nChanle);
static void InitDyncScalerParam(sDyncScalerImpl* sdi,int nChanle);

//秤台数组
static  sScaler gScalerArray[MAX_SCALER];

static pLockFunc  pScaleLock = NULL;

static pUnlockFunc  pScaleUnlock = NULL;

void	ScalerLock(void)
{
	if(pScaleLock != NULL) pScaleLock();
}

void	ScalerUnlock(void)
{
	if(pScaleUnlock != NULL) pScaleUnlock();
}

//创建秤台
HScaler ScalerCreate(int nAdChanle, ERR_WEIGHT* err)
{
	int i = 0;
	HScaler hs = HSCALER_INVALID;

	*err = ERR_SUCCESS;

	ScalerLock();

	//检查通道数目
	if((nAdChanle <= 0) || (nAdChanle > MAX_AD_CHANLE)) 
	{
		*err = ERR_INVALID_VAR;

		ScalerUnlock();

		return HSCALER_INVALID;
	}

	//遍历数组选择未使用的秤台
	for(i = 0; i < MAX_SCALER; i++)
	{
		if(gScalerArray[i].bUsed == 0)
		{
			gScalerArray[i].bUsed = 1;
			gScalerArray[i].bRun   = 0;
			hs = (unsigned int)(&(gScalerArray[i]));
			break;
		}
	}

	//返回无效秤台
	if(hs == HSCALER_INVALID)
	{
		*err = ERR_NO_SCALER;

		ScalerUnlock();

		return HSCALER_INVALID;
	}

	//设置秤台通道数
	SCALER(hs)->nAdcCount = nAdChanle;

	//初始化AD采集通道
	for(i=0; i < nAdChanle; i++)
	{	  
		InitAdcChanle(&(SCALER(hs)->ArrAdcProcChannel[i]));
	}

	//初始化静态秤参数
	InitStaticScalerParam(&(SCALER(hs)->StaticScalerImpl), nAdChanle);

	//初始化动态秤参数
	InitDyncScalerParam(&(SCALER(hs)->DyncScalerImpl), nAdChanle);

	ScalerUnlock();

	return hs;   
}

//销毁秤台
void ScalerDestory(HScaler hs)
{
	int i = 0;

	ScalerLock();

	//遍历数组选择未使用的秤台
	for(i = 0; i < MAX_SCALER; i++)
	{
		if(hs == (unsigned int)(&(gScalerArray[i])))
		{
			gScalerArray[i].bUsed = 0;
			gScalerArray[i].bRun = 0;
		}
	}

	ScalerUnlock();

}

//注册临界区保护函数

int RegisterScalerLockProc( pLockFunc  pLock, pUnlockFunc  pUnlock)
{
	pScaleLock 	= pLock;
	pScaleUnlock = pUnlock;	

	return 0;
}

//注册ADC原始AD获取函数
int RegisterAdcGetProc(HScaler hs, int chanle, pInitFunc pAdcGetInit, pAdcGetFun pAdcGet, void* UserData)
{    
	CHECK_CHANLE_VALID(hs, chanle);

	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcGetInit		= pAdcGetInit;
	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcGet			= pAdcGet;
	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcGet_UserData	= UserData;

	return 0;
}

//注册ADC原始AD异常检查函数
int RegisterAdcCheckProc(HScaler hs, int chanle, pInitFunc pAdcValidCheckInit, pAdcValidCheckFunc pAdcValidCheck, void* UserData)
{
	CHECK_CHANLE_VALID(hs, chanle);

	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcValidCheckInit = pAdcValidCheckInit;
	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcValidCheck		= pAdcValidCheck;
	SCALER(hs)->ArrAdcProcChannel[chanle].pAvc_UserData		= UserData;

	return 0;
}

//注册ADC通道滤波函数
int RegisterAdcFilterProc(HScaler hs, int chanle, pInitFunc pFilterInit, pFilterFunc pFilter, void* UserData)
{
	CHECK_CHANLE_VALID(hs, chanle);

	SCALER(hs)->ArrAdcProcChannel[chanle].pFilterInit	= pFilterInit;
	SCALER(hs)->ArrAdcProcChannel[chanle].pFilter		= pFilter;
	SCALER(hs)->ArrAdcProcChannel[chanle].pFcb_UserData	= UserData;

	return 0;
}

//注册ADC标准化函数
int RegisterAdcStandlizeProc(HScaler hs, int chanle, pInitFunc pAdcStandlizeInit, pAdcStandlizeFunc pAdcStandlize, void* UserData)
{
	CHECK_CHANLE_VALID(hs, chanle);

	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcStandlizeInit	= pAdcStandlizeInit;
	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcStandlize		= pAdcStandlize;
	SCALER(hs)->ArrAdcProcChannel[chanle].pScb_UserData		= UserData;

	return 0;
}

//注册ADC通道归一化函数
int RegisterAdcNormalizeProc(HScaler hs, int chanle, pInitFunc pAdcNormalizeInit, pAdcNormalizeFunc pAdcNormalize, void* UserData)
{
	CHECK_CHANLE_VALID(hs, chanle);

	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcNormalizeInit	= pAdcNormalizeInit;
	SCALER(hs)->ArrAdcProcChannel[chanle].pAdcNormalize		= pAdcNormalize;
	SCALER(hs)->ArrAdcProcChannel[chanle].pNcb_UserData		= UserData;

	return 0;
}

//注册ADC通道中动态零位跟踪函数
int RegisterAdcZeroTraceProc(HScaler hs, int chanle, pInitFunc pZeroTraceInit, pZeroTraceFunc pZeroTrace, pSetZeroFunc pSetZero, void* UserData)
{
	CHECK_CHANLE_VALID(hs, chanle);

	SCALER(hs)->ArrAdcProcChannel[chanle].pZeroTraceInit = pZeroTraceInit;
	SCALER(hs)->ArrAdcProcChannel[chanle].pZeroTrace = pZeroTrace;
	SCALER(hs)->ArrAdcProcChannel[chanle].pSetZero = pSetZero;
	SCALER(hs)->ArrAdcProcChannel[chanle].pZcb_UserData = UserData;

	return 0;
}

//注册静态处理函数
int RegisterScalerStaticTotalProc(HScaler hs, pInitFunc pStaticTotalInit, pStaticTotalFunc pStaticTotal, void* UserData)
{
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->StaticScalerImpl.pStaticTotalInit	= pStaticTotalInit;
	SCALER(hs)->StaticScalerImpl.pStaticTotal		= pStaticTotal;
	SCALER(hs)->StaticScalerImpl.pTcb_UserData	= UserData;

	return 0;
}

//组成静态处理采用函数
int RegisterScalerStaticSampleProc(HScaler hs, pInitFunc pSampleInit, pSampleFunc pSample, void* UserData)
{
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->StaticScalerImpl.pSampleInit = pSampleInit;
	SCALER(hs)->StaticScalerImpl.pSample	 = pSample;
	SCALER(hs)->StaticScalerImpl.pDscb_UserData = UserData;

	return 0;
}

//注册静态处理滤波函数
int RegisterScalerStaticFilterProc(HScaler hs, pInitFunc pFilterInit, pFilterFunc pFilter, void* UserData)
{
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->StaticScalerImpl.pFilterInit   =  pFilterInit;
	SCALER(hs)->StaticScalerImpl.pFilter		=  pFilter;
	SCALER(hs)->StaticScalerImpl.pFcb_UserData	=  UserData;

	return 0;
}

//注册静态处理零位跟踪函数
int RegisterScalerStaticZeroTraceProc(HScaler hs, pInitFunc pZeroTraceInit, pZeroTraceFunc pZeroTrace, pSetZeroFunc pSetZero, void* UserData)
{
	CHECK_HANDLE_VALID(hs);
	SCALER(hs)->StaticScalerImpl.pZeroTraceInit	= pZeroTraceInit;
	SCALER(hs)->StaticScalerImpl.pZeroTrace		= pZeroTrace;
	SCALER(hs)->StaticScalerImpl.pSetZero		= pSetZero;
	SCALER(hs)->StaticScalerImpl.pZtcb_UserData	= UserData;

	return 0;
}

//注册静态处理状态检查函数
int RegisterScalerStaticStateCheckProc(HScaler hs, pInitFunc pStateCheckInit, pStateCheckFunc pStateCheck, void* UserData)
{
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->StaticScalerImpl.pStateCheckInit	= pStateCheckInit;
	SCALER(hs)->StaticScalerImpl.pStateCheck		=  pStateCheck;
	SCALER(hs)->StaticScalerImpl.pSccb_UserData	= UserData;

	return 0;
}

//注册动态处理函数
int RegisterScalerDyncProc(HScaler hs, pDyncProcInitFunc pDyncProcInit, pDyncProcFunc pDyncProc, void* UserData)
{
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->DyncScalerImpl.pDyncProcInit	= pDyncProcInit;
	SCALER(hs)->DyncScalerImpl.pDyncProc		= pDyncProc;
	SCALER(hs)->DyncScalerImpl.pDecb			= UserData;

	return 0;
}

//获取ADC通道原始AD值
int GetAdcChanleOrgAd(HScaler hs, int chanel) 
{
	sAdcProcChannel* pApc;
	CHECK_CHANLE_VALID(hs,chanel);

	pApc = &(SCALER(hs)->ArrAdcProcChannel[chanel]);

	return pApc->nOrgAd;
}

//获取ADC通道检测状态
int GetAdcChanleValidState(HScaler hs, int chanel) 
{
	sAdcProcChannel* pApc;
	CHECK_CHANLE_VALID(hs,chanel);

	pApc = &(SCALER(hs)->ArrAdcProcChannel[chanel]);

	return pApc->nValidState ;
}

//获取ADC通道滤波输出结果
float GetAdcChanleFilterValue(HScaler hs, int chanel) 
{
	sAdcProcChannel* pApc;
	CHECK_CHANLE_VALID(hs,chanel);

	pApc = &(SCALER(hs)->ArrAdcProcChannel[chanel]);

	return pApc->fFilterValue ;
}

//获取ADC通道标准化输出结果
float GetAdcChanleStandValue(HScaler hs, int chanel) 
{
	sAdcProcChannel* pApc;
	CHECK_CHANLE_VALID(hs,chanel);

	pApc = &(SCALER(hs)->ArrAdcProcChannel[chanel]);

	return pApc->fStandValue;
}

//获取ADC通道归一化处理输出结果
float GetAdcChanleNormalizeValue(HScaler hs, int chanel) 
{
	sAdcProcChannel* pApc;
	CHECK_CHANLE_VALID(hs,chanel);

	pApc = &(SCALER(hs)->ArrAdcProcChannel[chanel]);

	return pApc->fNormalizeValue;
}

//获取ADC通道零位处理结果
float GetAdcChanleZerolizeValue(HScaler hs, int chanel) 
{
	sAdcProcChannel* pApc;
	CHECK_CHANLE_VALID(hs,chanel);

	pApc = &(SCALER(hs)->ArrAdcProcChannel[chanel]);

	return pApc->fZerolizeValue ;
}

//获取静态求值结果
float GetScalerStaticTotalValue(HScaler hs)
{
	sStaticScalerImpl* ssi;
	CHECK_HANDLE_VALID(hs);

	ssi = &(SCALER(hs)->StaticScalerImpl);

	return ssi->fTotalValue;
}

//获取静态抽样结果
float GetScalerStaticSampleValue(HScaler hs)
{
	sStaticScalerImpl* ssi;
	CHECK_HANDLE_VALID(hs);

	ssi = &(SCALER(hs)->StaticScalerImpl);

	return ssi->fSampleValue;
}

//获取静态滤波结果
float GetScalerStaticFilterValue(HScaler hs)
{
	sStaticScalerImpl* ssi;
	CHECK_HANDLE_VALID(hs);

	ssi = &(SCALER(hs)->StaticScalerImpl);

	return ssi->fFilterValue;
}

//获取静态零位跟踪结果
float  GetScalerStaticZeroTraceValue(HScaler hs)
{
	sStaticScalerImpl* ssi;
	CHECK_HANDLE_VALID(hs);

	ssi = &(SCALER(hs)->StaticScalerImpl);

	return ssi->fZeroTraceValue;
}

//获取静态状态结果
int GetScalerStaticState(HScaler hs)
{
	sStaticScalerImpl* ssi;
	CHECK_HANDLE_VALID(hs);

	ssi = &(SCALER(hs)->StaticScalerImpl);

	return ssi->nState;
}

//获取静态设定分度值
float GetScalerStaticDrv(HScaler hs)
{
	sStaticScalerImpl* ssi;
	CHECK_HANDLE_VALID(hs);

	ssi = &(SCALER(hs)->StaticScalerImpl);

	return ssi->fStaticDrv;
}

//获取静态分度值处理后的结果
float GetScalerStaticDrvValue(HScaler hs)
{
	sStaticScalerImpl* ssi;
	CHECK_HANDLE_VALID(hs);

	ssi = &(SCALER(hs)->StaticScalerImpl);

	return ssi->fStaticDrvValue;
}

//获取动态处理值
float GetScalerDyncValue(HScaler hs )
{
	sDyncScalerImpl* dsi;
	CHECK_HANDLE_VALID(hs);
	dsi= &(SCALER(hs)->DyncScalerImpl);

	return dsi->fDyncValue;
}

//获取动态处理后的分度值
float GetScalerDyncDrvValue(HScaler hs )
{
	sDyncScalerImpl* dsi;
	CHECK_HANDLE_VALID(hs);

	dsi= &(SCALER(hs)->DyncScalerImpl);

	return dsi->fDyncDrvValue;
}

//获取动态设定分度值
float GetScalerDyncDrv(HScaler hs )
{
	sDyncScalerImpl* dsi;
	CHECK_HANDLE_VALID(hs);

	dsi= &(SCALER(hs)->DyncScalerImpl);
	return dsi->fDyncDrv;
}

#define STATE_ZERO      0x01
#define STATE_STEADY    0x10

//获取秤台稳定状态
static char GetSteadyState(int State)
{
	if(State & STATE_STEADY) 
	{
		return 1;
	}

	return 0;
}

//获取秤台零位状态
static char GetZeroState(int State)
{
	if(State & STATE_ZERO) 
	{
		return 1;
	}

	return 0;
}

//获取算法版本号
long	GetScalerVer(void)
{
	return ((DTOS_MAJOR_VER << 24) + (DTOS_MAIN_VER << 16 ) + ( DTOS_MINOR_VER<<8) + DTOS_DEBUG_VER);
}

/*
判断指定秤台是否稳定
*/
int IsScalerStaticSteady(HScaler hs)
{
	CHECK_HANDLE_VALID(hs);

	if(GetSteadyState(SCALER(hs)->StaticScalerImpl.nState))
		return 1;
	else
		return 0;
}

/*
判断秤台是否处于零点
*/
int IsScalerStaticAtZero(HScaler hs)
{
	CHECK_HANDLE_VALID(hs);

	if(GetZeroState(SCALER(hs)->StaticScalerImpl.nState))
		return 1;
	else
		return 0;
}

/*
判断秤台某一路AD是否错误
*/
int IsAdcError(HScaler hs, int channel)
{
	CHECK_CHANLE_VALID(hs,channel);

	return SCALER(hs)->ArrAdcProcChannel[channel].nValidState?1:0;

}
/*! 
*	错误类型定义
*
*/
static char* errStrings[] = 
{
	"成功",			/*!< 无效参数. */
	"无效参数",
	"无可用秤台",
	"无效的ADC通道编号处理函数",	
	"没有定义AD采集函数",	 
	"没有定义AD有效性检查函数",	
	"没有定义AD初级滤波函数",	
	"没有定义AD标准化函数",	
	"没有定义AD归一化函数",	
	"没有定义AD零点跟踪函数",	
	"没有定义静态处理初始化函数",	
	"没有定义静态处理函数",	
	"没有定义静态抽样处理函数",	 
	"没有定义静态滤波函数",	 
	"没有定义静态零点跟踪处理函数",	
	"没有定义静态状态处理函数",	
	"没有定义动态处理初始化函数",	
	"没有定义动态处理函数",	 
};
//获取错误类型提示
const char* GetError(ERR_WEIGHT err)
{
	if(err >= ERR_MAX) return NULL;

	return errStrings[err];
}

//设置静态分度值
int SetScalerStaticDrv(HScaler hs, float nDrv)
{
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->StaticScalerImpl.fStaticDrv = nDrv;

	return 0;
}

//设置动态分度值
int SetScalerDyncDrv(HScaler hs, float nDrv)
{
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->DyncScalerImpl.fDyncDrv = nDrv;

	return 0;
}

//设置秤台分频
void EnableScalerFreqDiv(HScaler hs, int bEnable, unsigned short nDiv)
{
	int chanle = 0;
	sAdcProcChannel *pAdcChanle = NULL;

	for(chanle = 0 ; chanle < SCALER(hs)->nAdcCount; chanle++)
	{
		pAdcChanle = &(SCALER(hs)->ArrAdcProcChannel[chanle]);

		pAdcChanle->bDivEnable = bEnable;
		pAdcChanle->nDivIndex = 0;
		pAdcChanle->nDivMax = nDiv;
		pAdcChanle->fDivSum = 0.0f;		
	}				
}

//设置ADC分频
void EnableAdcFreqDiv(HScaler hs, int nChannel, int bEnable, unsigned short nDiv)
{
	sAdcProcChannel *pAdcChanle = NULL;

	if(hs == HSCALER_INVALID) return;
	if(nChannel < 0) return;
	if(nChannel >= SCALER(hs)->nAdcCount) return;

	pAdcChanle = &(SCALER(hs)->ArrAdcProcChannel[nChannel]);

	pAdcChanle->bDivEnable = bEnable;
	pAdcChanle->nDivIndex = 0;
	pAdcChanle->nDivMax = nDiv;
	pAdcChanle->fDivSum = 0.0f;		
}

//AD通道初始化
static void InitAdcChanle(sAdcProcChannel* pAdc)
{
	if(pAdc == NULL) return;

	//获取原始AD值
	pAdc->pAdcGetInit	= NULL;
	pAdc->pAdcGet		= NULL;
	pAdc->nOrgAd		= 0;

	//AD码有效性检查
	pAdc->pAvc_UserData			= NULL;
	pAdc->pAdcValidCheckInit	= NULL;
	pAdc->pAdcValidCheck		= NULL;
	pAdc->nValidState			= 0;

	//分频
	pAdc->bDivEnable = 0;
	pAdc->nDivIndex = 0;
	pAdc->nDivMax = 1;
	pAdc->fDivSum = 0.0f;
	pAdc->fDivValue = 0.0f;

	//滤波
	pAdc->pFcb_UserData			= NULL;
	pAdc->pFilterInit			= NULL;
	pAdc->pFilter				= NULL;
	pAdc->fFilterValue			= 0.0f;

	//标准化
	pAdc->pScb_UserData			= NULL;
	pAdc->pAdcStandlizeInit		= NULL;
	pAdc->pAdcStandlize			= NULL;
	pAdc->fStandValue			= 0.0f;

	//归一化
	pAdc->pNcb_UserData			= NULL;
	pAdc->pAdcNormalizeInit		= NULL;
	pAdc->pAdcNormalize			= NULL;
	pAdc->fNormalizeValue		= 0.0f;

	//动态零位跟踪
	pAdc->pZcb_UserData			= 0;
	pAdc->pZeroTraceInit		= NULL;
	pAdc->pZeroTrace			= NULL;
	pAdc->fZerolizeValue		= 0.0f;
}

//静态处理初始化
static void InitStaticScalerParam(sStaticScalerImpl* ssi,int nChanle)
{
	int i;

	if(ssi == NULL) return;

	ssi->nInCount			= nChanle;
	if(ssi->nInCount < 0)	ssi->nInCount = 0;
	if(ssi->nInCount > MAX_AD_CHANLE) ssi->nInCount = MAX_AD_CHANLE;

	for(i = 0; i < ssi->nInCount; i++)
	{
		ssi->InArr[i] = 0.0f;
	}

	ssi->pStaticTotalInit	= NULL;
	ssi->pStaticTotal		= NULL;
	ssi->fTotalValue		= 0.0f;


	ssi->pSampleInit		= NULL;
	ssi->pSample			= NULL;
	ssi->fSampleValue		= 0.0f;

	ssi->pFilterInit		= NULL;
	ssi->pFilter			= NULL;
	ssi->fFilterValue		= 0.0f;

	ssi->pZeroTraceInit		= NULL;
	ssi->pZeroTrace			= NULL;
	ssi->fZeroTraceValue	= 0.0f;


	ssi->pStateCheckInit	= NULL;
	ssi->pStateCheck		= NULL;
	ssi->nState				= 0;

	ssi->fStaticDrv			= 1.0f;
	ssi->fStaticDrvValue	= 0.0f;
}

//动态处理初始化
static void InitDyncScalerParam(sDyncScalerImpl* sdi,int nChanle)
{
	int i;

	if(sdi == NULL) return;

	sdi->nInCount			= nChanle;
	if(sdi->nInCount < 0)	sdi->nInCount = 0;
	if(sdi->nInCount > MAX_AD_CHANLE) sdi->nInCount = MAX_AD_CHANLE;

	for(i = 0; i < sdi->nInCount; i++)
	{
		sdi->InArr[i] = 0.0f;
	}

	sdi->pDyncProcInit	= NULL;
	sdi->pDyncProc		= NULL;
	sdi->fDyncDrv		= 1.0f;
	sdi->fDyncValue		= 0.0f;
	sdi->fDyncDrvValue	= 0.0f;
}

//ADC通道处理函数
static ERR_WEIGHT AdcChanleProc(HScaler hs)
{
	int chanle = 0;
	sAdcProcChannel *pAdcChanle = NULL;

	for(chanle = 0 ; chanle < SCALER(hs)->nAdcCount; chanle++)
	{
		pAdcChanle = &(SCALER(hs)->ArrAdcProcChannel[chanle]);

		//获取原始AD值
		pAdcChanle->nOrgAd = pAdcChanle->pAdcGet(chanle);

		//调用故障检测函数来更新故障状态
		pAdcChanle->nValidState = pAdcChanle->pAdcValidCheck(pAdcChanle->nOrgAd);
		if(pAdcChanle->nValidState != 0)
		{
			continue;
		}

		//分频处理
		if(pAdcChanle->bDivEnable > 0)
		{
			pAdcChanle->fDivSum += (float)pAdcChanle->nOrgAd;
			pAdcChanle->nDivIndex++;
			if(pAdcChanle->nDivIndex < pAdcChanle->nDivMax) continue;

			pAdcChanle->fDivValue = pAdcChanle->fDivSum / pAdcChanle->nDivIndex;
			pAdcChanle->fDivSum = 0.0f;
			pAdcChanle->nDivIndex = 0;
		}
		else
		{
			pAdcChanle->fDivValue = (float)pAdcChanle->nOrgAd;
		}

		//调用初级滤波函数进行滤波，返回滤波后的AD值
		pAdcChanle->fFilterValue = pAdcChanle->pFilter(pAdcChanle->pFcb_UserData, pAdcChanle->fDivValue);

		//标准化
		pAdcChanle->fStandValue       = pAdcChanle->pAdcStandlize(pAdcChanle->pScb_UserData, pAdcChanle->fFilterValue);

		//归一化
		pAdcChanle->fNormalizeValue   = pAdcChanle->pAdcNormalize(pAdcChanle->pNcb_UserData, pAdcChanle->fStandValue);


		//动态零位
		pAdcChanle->fZerolizeValue    = pAdcChanle->pZeroTrace(pAdcChanle->pZcb_UserData, pAdcChanle->fNormalizeValue);

		//静态秤参数赋值, 该值用于静态重量计算
		SCALER(hs)->StaticScalerImpl.InArr[chanle] = pAdcChanle->fNormalizeValue;

		//动态秤参数赋值，改值用于零点跟踪
		SCALER(hs)->DyncScalerImpl.InArr[chanle] = pAdcChanle->fZerolizeValue;
	}

	return ERR_SUCCESS;
}


//静态处理
ERR_WEIGHT StaticScalerProcs(sStaticScalerImpl *pSSI)
{
	char ret=0;

	if(pSSI == NULL) return ERR_INVALID_VAR;

	//静态求值
	pSSI->fTotalValue      = pSSI->pStaticTotal(pSSI->pTcb_UserData, pSSI->InArr, pSSI->nInCount);

	//静态抽样
	ret                     = pSSI->pSample(pSSI->pDscb_UserData, pSSI->fTotalValue, &(pSSI->fSampleValue));
	if(!ret) return  ERR_SUCCESS;

	//静态秤滤波
	pSSI->fFilterValue      = pSSI->pFilter(pSSI->pFcb_UserData, pSSI->fSampleValue);

	//零位跟踪
	pSSI->fZeroTraceValue   = pSSI->pZeroTrace(pSSI->pZtcb_UserData, pSSI->fFilterValue);

	//状态判断
	pSSI->nState            = pSSI->pStateCheck(pSSI->pSccb_UserData, pSSI->fZeroTraceValue);

	//静态分度值处理
	pSSI->fStaticDrvValue   = WetDrvProc(pSSI->fZeroTraceValue, pSSI->fStaticDrv);

	return ERR_SUCCESS;
}


//动态处理
ERR_WEIGHT DyncScalerProcs(sDyncScalerImpl *pDSI)
{
	if(pDSI == NULL) return ERR_INVALID_VAR;

	//动态求值
	pDSI->fDyncValue      = pDSI->pDyncProc(pDSI->pDecb, pDSI->InArr, pDSI->nInCount);

	//动态分度值处理
	pDSI->fDyncDrvValue   = WetDrvProc(pDSI->fDyncValue, pDSI->fDyncDrv);

	return ERR_SUCCESS;
}


/*!
\brief 检测秤台是否有效
*/
static ERR_WEIGHT ScalerCheckVaild(sScaler *pScaler)
{
	int i;
	sStaticScalerImpl*	pSSI = NULL;
	sDyncScalerImpl*	pDSI = NULL;
	sAdcProcChannel		*pAdcChanle = NULL;

	//检查秤台参数
	if(pScaler == NULL) return ERR_INVALID_VAR;

	//遍历数组核对有效称台
	for(i = 0; i < MAX_SCALER; i++)
	{
		if(pScaler == &(gScalerArray[i]))
		{
			break;
		}
	}
	if(i == MAX_SCALER) return ERR_INVALID_VAR;

	if((pScaler->nAdcCount < 0) || (pScaler->nAdcCount > MAX_AD_CHANLE)) return ERR_INVALID_VAR;

	//检查每个通道的参数
	for(i = 0; i < pScaler->nAdcCount; i++)
	{
		pAdcChanle = &(pScaler->ArrAdcProcChannel[i]);

		if(pAdcChanle->pAdcGet == NULL)
		{
			///该AD采集通道没有AD采集函数，失败返回
			return ERR_NO_ADC_PROC;
		}
		if(pAdcChanle->pAdcValidCheck == NULL)
		{
			///AD检测函数不存在 ,可以先跳过，因为有可能不需要检查AD
			return ERR_NO_ADC_CHECK_PROC;
		}
		if(pAdcChanle->pFilter == NULL)
		{
			///AD滤波函数不存在
			return ERR_NO_ADC_FILTER_PROC;
		}
		if(pAdcChanle->pAdcStandlize == NULL)
		{
			///标准化函数不存在
			return ERR_NO_ADC_STAND_PROC;
		}
		if(pAdcChanle->pAdcNormalize == NULL)
		{
			///归一化函数不存在
			return ERR_NO_ADC_NORMALIZE_PROC;
		}

		if(pAdcChanle->pZeroTrace == NULL)
		{
			///零点跟踪函数不存在
			return ERR_NO_ADC_ZEROTRACE_PROC;
		}

		if(pAdcChanle->pSetZero== NULL)
		{
			///零点跟踪函数不存在
			return ERR_NO_ADC_ZEROTRACE_PROC;
		}
	}

	//静态处理参数检查
	pSSI = &(pScaler->StaticScalerImpl);	
	if(pSSI->pStaticTotal == NULL)		return ERR_NO_SS_STATIC_PROC;
	if(pSSI->pSample == NULL)		return ERR_NO_SS_DIGSAMPLE_PROC;
	if(pSSI->pFilter == NULL)			return ERR_NO_SS_FILTER_PROC;
	if(pSSI->pZeroTrace == NULL)		return ERR_NO_SS_ZEROTRACE_PROC;
	if(pSSI->pSetZero == NULL)		return ERR_NO_SS_ZEROTRACE_PROC;
	if(pSSI->pStateCheck == NULL)		return ERR_NO_SS_STATECHECK_PROC;

	//动态处理参数检查
	pDSI = &(pScaler->DyncScalerImpl);
	if(pDSI->pDyncProc == NULL)			return ERR_NO_DS_DYNC_PROC;

	return ERR_SUCCESS;
}

//启动称重处理
ERR_WEIGHT ScalerStart(HScaler hs)
{
	int i = 0;
	sScaler *pScaler = NULL;
	ERR_WEIGHT retCode = ERR_SUCCESS;

	ScalerLock();

	//参数检查
	CHECK_HANDLE_VALID(hs);

	pScaler = SCALER(hs);	
	if(pScaler->bUsed == 0)
	{
		ScalerUnlock();
		return ERR_INVALID_VAR;
	}	

	if((pScaler->nAdcCount > MAX_AD_CHANLE )  || (pScaler->nAdcCount <= 0) )
	{
		ScalerUnlock();
		return ERR_INVALID_VAR;
	}

	//参数检查
	retCode = ScalerCheckVaild(pScaler);	
	if(ERR_SUCCESS != retCode)
	{
		ScalerUnlock();
		return retCode;
	}	

	//调用通道初始化函数
	for (i = 0; i < pScaler->nAdcCount; i++)
	{	
		//AD采集通道初始化
		if(pScaler->ArrAdcProcChannel[i].pAdcGetInit != NULL)
		{
			pScaler->ArrAdcProcChannel[i].pAdcGetInit(pScaler->ArrAdcProcChannel[i].pAdcGet_UserData);
		}

		//AD通道异常处理初始化
		if(pScaler->ArrAdcProcChannel[i].pAdcValidCheckInit != NULL)
		{
			pScaler->ArrAdcProcChannel[i].pAdcValidCheckInit(pScaler->ArrAdcProcChannel[i].pAvc_UserData);
		}

		//AD通道初级滤波初始化
		if(pScaler->ArrAdcProcChannel[i].pFilterInit != NULL)
		{
			pScaler->ArrAdcProcChannel[i].pFilterInit(pScaler->ArrAdcProcChannel[i].pFcb_UserData);
		}

		//AD通道标准化初始化
		if(pScaler->ArrAdcProcChannel[i].pAdcStandlizeInit != NULL)
		{
			pScaler->ArrAdcProcChannel[i].pAdcStandlizeInit(pScaler->ArrAdcProcChannel[i].pScb_UserData);
		}

		//AD通道归一化初始化
		if(pScaler->ArrAdcProcChannel[i].pAdcNormalizeInit != NULL)
		{
			pScaler->ArrAdcProcChannel[i].pAdcNormalizeInit(pScaler->ArrAdcProcChannel[i].pNcb_UserData);
		}

		//AD通道零点跟踪初始化
		if(pScaler->ArrAdcProcChannel[i].pZeroTraceInit != NULL)
		{
			pScaler->ArrAdcProcChannel[i].pZeroTraceInit(pScaler->ArrAdcProcChannel[i].pZcb_UserData);
		}	
	}

	//静态秤处理初始化
	if(pScaler->StaticScalerImpl.pStaticTotalInit != NULL)
	{
		pScaler->StaticScalerImpl.pStaticTotalInit(pScaler->StaticScalerImpl.pTcb_UserData);
	}

	//抽样函数初始化
	if(pScaler->StaticScalerImpl.pSampleInit != NULL)
	{
		pScaler->StaticScalerImpl.pSampleInit(pScaler->StaticScalerImpl.pDscb_UserData);
	}

	//静态滤波初始化
	if(pScaler->StaticScalerImpl.pFilterInit != NULL)
	{
		pScaler->StaticScalerImpl.pFilterInit(pScaler->StaticScalerImpl.pFcb_UserData);
	}

	//零点跟踪初始化 
	if(pScaler->StaticScalerImpl.pZeroTraceInit != NULL)
	{
		pScaler->StaticScalerImpl.pZeroTraceInit(pScaler->StaticScalerImpl.pZtcb_UserData);
	}

	//状态处理初始化 
	if(pScaler->StaticScalerImpl.pStateCheckInit != NULL)
	{
		pScaler->StaticScalerImpl.pStateCheckInit(pScaler->StaticScalerImpl.pSccb_UserData);
	}

	//动态初始化
	if(pScaler->DyncScalerImpl.pDyncProcInit != NULL)
	{
		pScaler->DyncScalerImpl.pDyncProcInit(pScaler->DyncScalerImpl.pDecb);
	}

	//启动运行
	pScaler->bRun = 1;

	ScalerUnlock();

	return ERR_SUCCESS;
}

//运行称重处理
ERR_WEIGHT ScalerRun(HScaler hs)
{
	ERR_WEIGHT nRet = ERR_SUCCESS;

	if(SCALER(hs)->bUsed == 0)  return ERR_NO_SCALER;	
	if(SCALER(hs)->bRun   == 0)  return ERR_INVALID_VAR;

	//AD采集通道处理
	nRet = AdcChanleProc(hs);	
	if (ERR_SUCCESS != nRet)
	{
		return nRet;
	}

	//静态秤处理
	nRet = StaticScalerProcs(&(SCALER(hs)->StaticScalerImpl));
	if (ERR_SUCCESS != nRet)
	{
		return nRet;
	}


	//动态处理
	nRet = DyncScalerProcs(&SCALER(hs)->DyncScalerImpl);
	if (ERR_SUCCESS != nRet)
	{
		return nRet;
	}

	return nRet;
}

//停止称重处理
ERR_WEIGHT ScalerStop(HScaler hs)
{
	ScalerLock();

	//参数检查
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->bRun = 0;

	ScalerUnlock();

	return ERR_SUCCESS;
}

//恢复称重处理
ERR_WEIGHT ScalerResume(HScaler hs)
{
	ScalerLock();

	//参数检查
	CHECK_HANDLE_VALID(hs);

	SCALER(hs)->bRun = 1;

	ScalerUnlock();

	return ERR_SUCCESS;
}

//静态置零处理
ERR_WEIGHT ScalerStaticSetZero(HScaler hs, signed char bSet)
{
	ScalerLock();

	//参数检查
	CHECK_HANDLE_VALID(hs);

	if(SCALER(hs)->bRun == 0) 
	{
		ScalerUnlock();
		return ERR_INVALID_VAR;
	}
	if(SCALER(hs)->StaticScalerImpl.pSetZero == NULL) 
	{
		ScalerUnlock();
		return ERR_INVALID_VAR;
	}	

	SCALER(hs)->StaticScalerImpl.pSetZero(SCALER(hs)->StaticScalerImpl.pZtcb_UserData, bSet);

	ScalerUnlock();

	return ERR_SUCCESS;
}

//动态置零处理
ERR_WEIGHT ScalerDyncSetZero(HScaler hs, signed char bSet)
{
	int i = 0;
	sAdcProcChannel		*pAdcChanle = NULL;

	//参数检查
	CHECK_HANDLE_VALID(hs);

	if(SCALER(hs)->bRun == 0) 
	{
		return ERR_INVALID_VAR;
	}

	ScalerLock();

	//处理每个通道的置零函数
	for(i = 0; i < SCALER(hs)->nAdcCount; i++)
	{
		pAdcChanle = &(SCALER(hs)->ArrAdcProcChannel[i]);

		if(pAdcChanle->pSetZero== NULL)
		{
			///该AD采集通道没有AD采集函数，失败返回
			ScalerUnlock();
			return ERR_NO_SS_ZEROTRACE_PROC;
		}
		pAdcChanle->pSetZero(pAdcChanle->pZcb_UserData, bSet);

	}	
	ScalerUnlock();
	return ERR_SUCCESS;
}



