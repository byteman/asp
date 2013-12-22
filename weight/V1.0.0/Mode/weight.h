
/**
 * \file weight.c
 * \brief 称重模块接口
 */

/*! \addtogroup group1 称重模块外部接口
 *  @{
 */

#ifndef __WEIGHT_H
#define __WEIGHT_H

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif


#ifndef NULL
#define NULL			0
#endif

typedef unsigned char  BOOLEAN;

#ifndef BOOL
#define	BOOL			BOOLEAN
#endif


#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif






//出错定义================================================================================================
/*!
	\brief 称重模块错误返回类型
	\enum  称重模块运行时返回的错误
*/
typedef enum{
	ERR_SUCCESS=0,					/*!<无错误，成功. */ 
	ERR_INVALID_VAR,				/*!<无效参数 */ 
	ERR_NO_SCALER,					/*!<无可用秤台 */
	ERR_NO_ADC_CHANLE,				/*!<无效的ADC通道编号处理函数 */
	ERR_NO_ADC_PROC,				/*!<没有定义AD采集函数*/ 
	ERR_NO_ADC_CHECK_PROC,			/*!<没有定义AD有效性检查函数*/ 
	ERR_NO_ADC_FILTER_PROC,			/*!<没有定义AD初级滤波函数*/ 
	ERR_NO_ADC_STAND_PROC,			/*!<没有定义AD标准化函数*/ 
	ERR_NO_ADC_NORMALIZE_PROC,		/*!<没有定义AD归一化函数*/ 
	ERR_NO_ADC_ZEROTRACE_PROC,		/*!<没有定义AD零点跟踪函数*/ 
	ERR_NO_SS_STATIC_INIT_PROC,		/*!<没有定义静态处理初始化函数*/ 
	ERR_NO_SS_STATIC_PROC,			/*!<没有定义静态处理函数*/ 
	ERR_NO_SS_DIGSAMPLE_PROC,		/*!<没有定义静态抽样处理函数*/ 
	ERR_NO_SS_FILTER_PROC,			/*!<没有定义静态滤波函数*/ 
	ERR_NO_SS_ZEROTRACE_PROC,		/*!<没有定义静态零点跟踪处理函数*/ 
	ERR_NO_SS_STATECHECK_PROC,		/*!<没有定义静态状态处理函数*/ 
	ERR_NO_DS_DYNC_INIT_PROC,		/*!<没有定义动态处理初始化函数*/ 
	ERR_NO_DS_DYNC_PROC,			/*!<没有定义动态处理函数*/ 
	ERR_MAX,						/*!<最大错误号，无用*/ 
}ERR_WEIGHT;




//回调函数定义================================================================================================

//临界区保护接口函数原型定义
typedef void (*pLockFunc)(void );

typedef void (*pUnlockFunc)(void );

/*! 
\brief 初级滤波初始化处理函数原型定义
\param pUserParam 用户传入的自定义值
\return 返回初始化是否成功
\retval 1 成功
\retval 0 失败
*/
typedef char (*pInitFunc)(void* pUserParam);


/*! 
\brief 注册的AD采集函数原型定义. 
\param[in]  chanle 通道编号
\param[out] value  采集到的AD值	
\return 返回采集是否成功
\retval 1 成功
\retval 0 失败
*/
typedef long (*pAdcGetFun)(int chanle);

/*! 
\brief AD错误检测函数原型定义
\param nAD 需要检查有效性的AD值
\return 返回AD值是否有效
\retval 1 有效
\retval 0 无效

*/
typedef int (*pAdcValidCheckFunc)(long nAd);


/*! 
\brief 初级滤波处理函数原型定义
\param pFCB 用户传入的自定义值
\param nIn  需要滤波的AD值
\return 返回滤波后的值
*/
typedef float (*pFilterFunc)(void* pFCB, float nIn);

/*! 
\brief 标准化函数原型定义
\param pSCB 用户传入的自定义值
\param nIn  需要标准化的AD值
\return		返回标准化后的值
*/
typedef float (*pAdcStandlizeFunc)(void* pSCB, float fIn);

/*! 
\brief 归一化函数原型定义
\param pSCB 用户传入的自定义值
\param nIn  需要归一化的值
\return		返回归一化后的值
*/
typedef float (*pAdcNormalizeFunc)(void *pNCB, float fIn);

/*! 
\brief 零位跟踪函数原型定义

*/
typedef float (*pZeroTraceFunc)(void *pZTCB, float fIn);

/*! 
\brief  置零函数原型定义

*/
typedef void (*pSetZeroFunc)(void *pZTCB);


/*! \brief 状态判定函数原型定义*/
typedef int (*pStateCheckFunc)(void *pSCCB, float fIn);


/*! \brief 分度值处理函数原型定义*/
typedef float (*pDrvFunc)(void *pDCB, float fIn);


/*!
\brief	静态求值函数原型
*/
typedef float (*pStaticTotalFunc)(void* pSecb, float * pInArr, int nInCount);


/*!
\brief	静态抽样函数原型
*/
typedef int (*pSampleFunc)(void *pDscb, float in, float* pOut);


/*! \brief 动态处理初始化函数类型*/
typedef char (*pDyncProcInitFunc)(void* pDecb);


/*! \brief 动态处理函数类型*/
typedef float (*pDyncProcFunc)(void* pDecb, float * pInArr, int nInCount);

/**@}*/






//秤台定义==================================================================================================
/*! \brief 称台对象句柄*/
typedef unsigned int HScaler; /*!> 称台对象句柄*/


/*! \brief 无效的称台对象句柄 */
#define HSCALER_INVALID  0  /*!< 无效的称台对象句柄*/


/*!
\brief 创建一个秤台对象.
\param nAdcChanle 该秤台拥有的AD采集通道数.
\return 返回分配成功的秤台句柄 .
\retval HSCALER_INVALID 秤台分配失败
\retval 大于0			成功
*/
HScaler ScalerCreate(int nAdcChanle, ERR_WEIGHT* err);



/*!
\brief 销毁一个秤台对象.
\param HScaler 要销毁的秤台.
*/
void ScalerDestory(HScaler hs);




//ADC通道处理注册===========================================================================================
/*!
\brief 
    注册AD采集函数，由外部模块实现AD采集函数，内部称重模块负责调用
    为某个指定秤台的某个AD采集通道， 指定AD采集函数
\param hs		
	秤台句柄
\param init_proc
	注册AD采集初始化函数
\param adc_proc 
	注册的AD采集函数
\return 
	返回注册的结果
\retval  0  成功
\retval -1  失败
\remark 该接口为指定秤台内的所有AD采集通道注册同一个AD采集函数
*/
int RegisterAdcGetProc(HScaler hs, int chanle, pInitFunc pAdcGetInit, pAdcGetFun pAdcGet, void* UserData);


/*!
\brief
        为秤台指定AD采集通道注册AD有效性检测函数,检测AD采集值是否正确
\param hs			秤台句柄
\param chanle		指定采集通道
\param check_proc   注册的AD转化结果检测函数
\see ::RegisterAdcCheckProc
\return 返回注册的结果
        
\retval 0  成功
\retval -1 失败
*/
int RegisterAdcCheckProc(HScaler hs, int chanle, pInitFunc pAdcValidCheckInit, pAdcValidCheckFunc pAdcValidCheck, void* UserData);


/*!
\brief
        为称台指定ADC通道注册AD初级滤波初始化函数
\param hs									秤台句柄
\param chanle							通道编号
\param init_filter_proc   注册的ADC初级滤波初始化函数
\param filter_proc				注册的ADC初级滤波函数
\param userData						用户自定义数据
\return 返回注册的结果
        
\retval 0  成功
\retval -1 失败
*/		
int RegisterAdcFilterProc(HScaler hs, int chanle, pInitFunc pFilterInit, pFilterFunc pFilter, void* UserData);


/*!
\brief
        为秤台指定AD采集通道注册标准化处理函数
\param hs			秤台句柄
\param chanle		指定采集通道
\param stand_proc   注册的AD转化结果检测函数
\param userData						用户自定义数据
\return 返回注册的结果
        
\retval 0  成功
\retval -1 失败
*/
int RegisterAdcStandlizeProc(HScaler hs, int chanle, pInitFunc pAdcStandlizeInit, pAdcStandlizeFunc pAdcStandlize, void* UserData);


/*!
\brief
        为称台指定通道注册AD归一化处理函数
\param hs									秤台句柄
\param chanle							通道编号
\param normalize_proc   	注册的归一化处理函数
\param userData						用户自定义数据
\return 返回注册的结果
        
\retval 0  成功
\retval -1 失败
*/								
int RegisterAdcNormalizeProc(HScaler hs, int chanle, pInitFunc pAdcNormalizeInit, pAdcNormalizeFunc pAdcNormalize, void* UserData);


/*!
\brief
        为称台注册统一的ADC零点跟踪处理函数
\param hs									秤台句柄
\param chanel							通道编号
\param zerotrack_proc   	注册的归一化处理函数
\param userData						用户自定义数据
\return 返回注册的结果
        
\retval 0  成功
\retval -1 失败
*/								
int RegisterAdcZeroTraceProc(HScaler hs, int chanle, pInitFunc pZeroTraceInit, pZeroTraceFunc pZeroTrace, pSetZeroFunc pSetZero, void* UserData);





//静态处理注册=====================================================================================================
/*!
\brief	注册静态处理函数，包括初始化和动态处理函数

  \param	hs		秤台对象
  \param	init_proc	注册静态处理初始化函数
  \param	dync_proc	注册静态处理函数 
  \param	userData	静态处理的附加数据 
  \return			返回成功与否
  \retval	0		成功
  \retval  -1		失败
*/
int RegisterScalerStaticTotalProc(HScaler hs, pInitFunc pStaticTotalInit, pStaticTotalFunc pStaticTotal, void* UserData);

/*!
\brief	为秤台注册静态秤抽样函数

  \param	hs		秤台对象
  \param	sample_proc	注册抽样初始化函数
  \param	sample_proc	注册抽样函数
  \param	userData	静态秤滤波处理的附加数据 
  \return			返回成功与否
  \retval	0		成功
  \retval  -1		失败
*/
int RegisterScalerStaticSampleProc(HScaler hs, pInitFunc pSampleInit, pSampleFunc pSample, void* UserData);


/*!
\brief	为秤台注册静态秤滤波函数，包括初始化和动态处理函数

  \param	hs		秤台对象
  \param	init_proc	注册静态秤滤波初始化函数
  \param	dync_proc	注册静态秤滤波处理函数 
  \param	userData	静态秤滤波处理的附加数据 
  \return			返回成功与否
  \retval	0		成功
  \retval  -1		失败
*/
int RegisterScalerStaticFilterProc(HScaler hs, pInitFunc pFilterInit, pFilterFunc pFilter, void* UserData);


/*!
\brief	为秤台注册静态秤零点跟踪函数

  \param	hs		秤台对象
  \param	zt_proc	注册抽样函数
  \param	userData	静态秤滤波处理的附加数据 
  \return			返回成功与否
  \retval	0		成功
  \retval  -1		失败
*/
int RegisterScalerStaticZeroTraceProc(HScaler hs, pInitFunc pZeroTraceInit, pZeroTraceFunc pZeroTrace, pSetZeroFunc pSetZero, void* UserData);



/*!
\brief	为秤台注册静态秤状态检测函数

  \param	hs		秤台对象
  \param	sta_check_proc	注册抽样函数
  \param	userData	静态秤滤波处理的附加数据 
  \return			返回成功与否
  \retval	0		成功
  \retval  -1		失败
*/
int RegisterScalerStaticStateCheckProc(HScaler hs, pInitFunc pStateCheckInit, pStateCheckFunc pStateCheck, void* UserData);


//设置静态分度值
int SetScalerStaticDrv(HScaler hs, float nDrv);










//动态处理注册========================================================================================================
/*!
\brief	注册动态处理函数，包括初始化和动态处理函数

  \param	hs		秤台对象
  \param	init_proc	注册动态处理初始化函数
  \param	dync_proc	注册动态处理函数 
  \param	userData	动态处理的附加数据 
  \return			返回成功与否
  \retval	0		成功
  \retval  -1		失败
*/
int RegisterScalerDyncProc(HScaler hs, pDyncProcInitFunc pDyncProcInit, pDyncProcFunc pDyncProc, void* UserData);


//设置动态分度值
int SetScalerDyncDrv(HScaler hs, float nDrv);









//ADC通道处理结果===========================================================================
//获取ADC通道原始AD值
int GetAdcChanleOrgAd(HScaler hs,int chanel);

//获取ADC通道检测状态
int GetAdcChanleValidState(HScaler hs,int chanel) ;

//获取ADC通道滤波输出结果
float GetAdcChanleFilterValue(HScaler hs,int chanel);

//获取ADC通道标准化输出结果
float GetAdcChanleStandValue(HScaler hs,int chanel);

//获取ADC通道归一化处理输出结果
float GetAdcChanleNormalizeValue(HScaler hs,int chanel);

//获取ADC通道零位处理结果
float GetAdcChanleZerolizeValue(HScaler hs,int chanel);

//判断秤台某一路AD是否错误
//0 --- 正常
// != 0 异常
int IsAdcError(HScaler hs,int channel);








//静态处理结果================================================================================
//获取静态求值结果
float GetScalerStaticTotalValue(HScaler hs);

//获取静态抽样结果
float GetScalerStaticSampleValue(HScaler hs);

//获取静态滤波结果
float GetScalerStaticFilterValue(HScaler hs);

//获取静态零位跟踪结果
float  GetScalerStaticZeroTraceValue(HScaler hs);

//获取静态状态结果
int GetScalerStaticState(HScaler hs);

//获取静态设定分度值
float GetScalerStaticDrv(HScaler hs);

//获取静态分度值处理后的结果
float GetScalerStaticDrvValue(HScaler hs);

//判断指定秤台是否静态稳定
//0 不稳定 !=0 稳定 
int IsScalerStaticSteady(HScaler hs);

//判断秤台是否处于静态零点
//0 不稳定 !=0 稳定 
int IsScalerStaticAtZero(HScaler hs); 








//动态处理结果==================================================================================
//获取动态处理值
float GetScalerDyncValue(HScaler hs );


//获取动态处理后的分度值
float GetScalerDyncDrvValue(HScaler hs );


//获取动态设定分度值
float GetScalerDyncDrv(HScaler hs );



//秤台调用=====================================================================================
/*!
\brief 称重启动函数，需要再调用了注册函数后被调用。初始指定的称重对象.
\param hs 称台对象
\return 返回启动的结果
\retval ERR_SUCCESS  成功
\retval 其他 失败
*/
ERR_WEIGHT ScalerStart(HScaler hs);

/*!
\brief 称重模块框架调度运行函数，该函数需要在注册和调用启动函数后定时调用。
\param   hs： 针对那个秤台对象运行称重处理
\return  运行的结果   
\retval  ERR_SUCCESS  成功
\retval  其他 				失败
*/
ERR_WEIGHT ScalerRun(HScaler hs);


//停止称重处理
ERR_WEIGHT ScalerStop(HScaler hs);

//恢复称重处理
ERR_WEIGHT ScalerResume(HScaler hs);

/*!
\brief 返回错误代码代表的含义
\param[in]  err  错误码
\return		 返回错误码字符串
*/

//动态置零处理
ERR_WEIGHT ScalerDyncSetZero(HScaler hs);

//静态置零处理
ERR_WEIGHT ScalerStaticSetZero(HScaler hs);

//获取称重模型版本 
//4字节表示，如果版本号为：1.2.2.3 返回值为：0x01020203
long	GetScalerVer(void);

const char* GetError(ERR_WEIGHT err);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif


#endif
 
/*! @} */ // end of group


