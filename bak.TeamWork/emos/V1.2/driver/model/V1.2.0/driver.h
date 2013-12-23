/**
 * \file driver.c
 * \brief EMOS平台驱动接口
 */

/*! \addtogroup group1 EMOS平台驱动外部接口
 *  @{
 */

#ifndef _DRIVER_H	
#define _DRIVER_H

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"
		
#ifndef NULL
#define NULL 0
#endif

#define HANDLE 			u8					/*!< 设备句柄. */ 
#define HANDLENULL	0xFF				/*!< 无效设备句柄. */ 
	
/*!
	\enum  HVL_ERR_CODE
	驱动返回的错误代码.
*/
typedef enum
{
	HVL_NO_ERR = 0,         		/*!< 没有错误 */ 
	HVL_NO_CONTROL_BLOCK,   		/*!< 没有设备控制块可用 */ 
	HVL_INSTALL_FAIL,       		/*!< 设备安装失败 */ 
	HVL_NO_FIND_DEV,        		/*!< 没有找有指定设设备 */ 
	HVL_OPEN_DEV_FAIL,      		/*!< 打开设备失败*/ 
	HVL_CLOSE_DEV_FAIL,     		/*!< 关闭设备失败 */ 
	HVL_HANDLE_DEV_FAIL,     		/*!< 设备句柄无效*/ 
	HVL_PARAM_ERR           		/*!< 参数错误*/ 
}HVL_ERR_CODE; 							

/*!
	\enum  DEV_ID
	设备编号
*/
typedef enum 
{
	
	CHAR_USART1 = 0, 			/*!<串口1 */ 
	CHAR_USART2, 			/*!<串口2 */ 
	CHAR_USART3, 			/*!<串口3*/ 
	CHAR_UART4, 			/*!<串口4 */ 
	CHAR_UART5, 			/*!<串口5*/ 
	CHAR_USART6,			/*!<串口6*/ 
	CHAR_TIM1,				/*!<定时器1*/ 
	CHAR_TIM2,				/*!<定时器2*/ 
	CHAR_TIM3,				/*!<定时器3*/ 
	CHAR_TIM4,				/*!<定时器4*/ 
	CHAR_TIM5,				/*!<定时器5*/ 
	CHAR_TIM6,				/*!<定时器6*/ 
	CHAR_TIM7,				/*!<定时器7*/ 
	CHAR_TIM8,				/*!<定时器8*/ 
	CHAR_TIM9,				/*!<定时器9*/ 
	CHAR_TIM10,				/*!<定时器10*/ 
	CHAR_TIM11,				/*!<定时器11*/ 
	CHAR_TIM12,				/*!<定时器12*/ 
	CHAR_RTC,					/*!<实时时钟*/ 
	CHAR_DOG,					/*!<看门狗*/ 
	CHAR_BEEPJOB,			/*!<蜂鸣器*/ 
	CHAR_TTS,					/*!<TTS语音*/ 
	CHAR_KEY,					/*!<矩阵键盘*/ 
	CHAR_LCD,					/*!<LCD屏*/ 
	CHAR_AD,					/*!<AD模数转换*/ 
	CHAR_LIGHT1,			/*!<光幕1*/ 
	CHAR_LIGHT2,			/*!<光幕2*/ 
	CHAR_LAND_1,			/*!<地感1*/ 
	CHAR_LAND_2,			/*!<地感2*/ 
	CHAR_AXIS1,				/*!<轮轴识别器1*/ 
	CHAR_AXIS2,				/*!<轮轴识别器2*/ 
	CHAR_xINIO1,			/*!<IO输入1*/ 
	CHAR_xINIO2,			/*!<IO输入2*/ 
	CHAR_xINIO3,			/*!<IO输入3*/ 
	CHAR_xINIO4,			/*!<IO输入4*/ 
	CHAR_xOUTIO_1,		/*!<IO输出1*/ 
	CHAR_xOUTIO_2,		/*!<IO输出2*/ 
	CHAR_xOUTIO_3,		/*!<IO输出3*/ 
	CHAR_xOUTIO_4,		/*!<IO输出4*/ 
	CHAR_UNDEF1,			/*!<未定义ID，用于扩展调试*/ 
	CHAR_UNDEF2,			/*!<未定义ID，用于扩展调试*/ 
	CHAR_UNDEF3,		  /*!<未定义ID，用于扩展调试*/ 
	CHAR_UNDEF4,			/*!<未定义ID，用于扩展调试*/ 
	CHAR_UNDEF5,			/*!<未定义ID，用于扩展调试*/ 

	//=================================================
	CHAR_BLOCK_BOUNDARY,	/*!<字符和块设备边界*/ 
	//=================================================
	
	//块设备
	BLOCK_FRAMEEPROM,	/*!<参数存储器*/ 
	BLOCK_DATAFLASH,	/*!<数据存储器*/ 
	BLOCK_USB1,				/*!<USB1*/ 
	BLOCK_USB2,				/*!<USB2*/ 
	BLOCK_SD,					/*!<SD*/ 
	BLOCK_NORFLASH,		/*!<NORFLASH*/ 
	BLOCK_NANDFLASH,	/*!<NANDFLASH*/ 
	BLOCK_UNDEF1,			/*!<未定义ID，用于扩展调试*/ 
	BLOCK_UNDEF2,			/*!<未定义ID，用于扩展调试*/ 
	BLOCK_UNDEF3,			/*!<未定义ID，用于扩展调试*/ 
	BLOCK_UNDEF4,			/*!<未定义ID，用于扩展调试*/ 
	BLOCK_UNDEF5,			/*!<未定义ID，用于扩展调试*/ 
	//=================================================
	DEV_MAX						/*!<最大设备ID号*/ 
	//=================================================
}DEV_ID;						

typedef struct 
{
	u32		ver;					/*!<驱动版本			自定义*/
	u8*		pDisc;				/*!<驱动描述地址	自定义*/
	u8    canShared;  	/*!<是否共享			0:不可共享使用, 1:可共享使用*/
	u8		OpenMax; 			/*!<对于共享设备最大打开次数*/
	u8		OpenCount; 		/*!<对于共享设备目前打开次数*/
	u8 		ReadMax; 			/*!<最大读次数*/
	u8		ReadConut;		/*!<目前读次数*/
	u8 		WriteMax; 		/*!<最大写次数*/
	u8		WriteConut;		/*!<目前写次数*/
}Dev_Info;						/*!<设备信息结构体*/


/*!
 \brief 根据设备ID号查询设备信息
 \param[in] ID 设备ID
 \return 设备信息结构体
*/
Dev_Info* QueryDev_Info(DEV_ID ID);			

/****************************************中断注册******************************************
说明：无
****************************************************************************************/
typedef void (*pIRQHandle)(void); //中断服务函数
typedef struct _SDEV_IRQ_REGS
{
	u8							EXTI_No;				//中断线号			1-16号
	u8							EXTI_Flag;			//中断线标志		0未注册		1注册
	pIRQHandle      IRQHandler;     //中断句柄
}SDEV_IRQ_REGS;
SDEV_IRQ_REGS* GetExti_list(void);			//中断线获取接口

/*!
	\brief 调用设备的打开函数		
	\return 设备Open返回为0表示成功
	*/
typedef		s8 		(*pDevOpen)		(u32 lParam);																												//设备打开
/*!
	\brief 调用设备的关闭函数	
	\return 设备Close返回为0标识成功
	*/	
typedef		s8 		(*pDevClose)	(void);																												
/*!
	\brief 字符设备读
	\retval >= 0 标识成功
	\retval <0 失败
	*/	
typedef		s32 	(*pCDevRead) 	(u8* buffer,u32 len);		
/*!
	\brief 字符设备写
	\retval >= 0 标识成功
	\retval <0 失败
	*/	
typedef		s32 	(*pCDevWrite)	(u8* buffer,u32 len);									
/*!
	\brief 块设备读
	\retval >= 0 标识成功
	\retval <0 失败
	*/				
typedef  	s32 	(*pBDevRead)	(u32 offset,u8* buffer,u32 len);	
/*!
	\brief 块设备写
	\retval >= 0 标识成功
	\retval <0 失败
	*/			
typedef  	s32 	(*pBDevWrite)	(u32 offset,u8* buffer,u32 len);	
/*!
	\brief 设备控制
	\retval >= 0 标识成功
	\retval <0 失败
	*/	
typedef		s32 	(*pDevIoctl)	(u32 cmd,u32 lParam);																								//

/*!
	\enum DEV_REG
	设备注册信息结构体
*/
typedef struct
{
	DEV_ID				ID;							/*!<设备ID */
	u8 						canShared;  		/*!<是否共享			0:不可共享使用, 1:可共享使用*/
	u8 						OpenMax; 				/*!<对于共享设备最大打开次数*/
	u8 						ReadMax; 				/*!<最大读次数*/
	u8 						WriteMax; 			/*!<最大写次数*/
	u8* 					pDisc;					/*!<驱动描述			自定义*/
	u32 					ver;						/*!<驱动版本			自定义*/
	u32*					devOpen;    		/*!<设备打开函数指针*/
	u32* 					devClose;   		/*!<设备关闭函数指针*/
	u32* 					devRead;				/*!<设备读函数映射指针*/
	u32* 					devWrite;				/*!<设备写函数映射指针*/
	u32* 					devIoctl;				/*!<设备控制函数映射指针*/
}DEV_REG;											

/****************************************设备驱动注册函数******************************************
说明：需要添加到设备驱动初始化函数中。XXX_Init()
**************************************************************************************************/
/*!
	\brief 设备注册
	\param dev 注册的设备
	\return 返回设备注册结果
*/
HVL_ERR_CODE DeviceInstall(DEV_REG *dev);			
/*!
	\brief 中断注册函数
	\param irq 注册的中断信息
	\return 返回中断注册的结果
*/
s8 EXTIIRQHandlerInstall(SDEV_IRQ_REGS* irq);		
/*!
	\brief 设备管理单元初始化
*/
void DevList_init(void);																

/****************************************应用层统一访问接口函数***********************************
说明：对各设备的访问接口统一，不同的设备用不同的设备ID号进行区分
**************************************************************************************************/
//设备应用层接口

/*! 
	\brief 打开设备
	\param[in]  		ID					设备ID号
	\param[in]  		lParam			参数
	\param[in,out]  err				  错误类型
	
	\return 返回平台内部分配的设备句柄,如果是非共享设备，有过一次打开后就不能再打开了	。
	\retval HANDLE 设备句柄
	\retval HANDLENULL 无效设备句柄
		
*/

HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err);

/*!

	\brief 			关闭设备
  \param[in] IDH					需要关闭设备的句柄
	\retval err		错误类型 
	\return 返回是否成功关闭
*/
HVL_ERR_CODE DeviceClose(HANDLE IDH);

/*!
  \brief 字符设备读.
	\param[in]  IDH					设备句柄
	\param[in]  Buffer					读取存放缓冲区
	\param[in]  len							希望读取的数据个数
	\return								是否成功		
	\retval >=0 成功	
	\retval <0  失败
*/
s32 CharDeviceRead(HANDLE IDH,u8* Buffer,u32 len);

/*!

	\brief 字符设备写
	\param[in] IDH			设备句柄
	\param[in] Buffer			设备写数据源缓冲区
	\param[in] len					希望写入的长度
	\param[in] Reallen		实际写入的数据个数
	\param[in] lParam			参数
	\return  是否成功	
	\retval >=0 实际写入的字节数
	\retval <0  写入失败
*/
s32 CharDeviceWrite(HANDLE IDH,u8* Buffer,u32 len);

/*!

	\brief 块设备读
	\param[in] IDH					设备句柄
	\param[in] offset					开始地址
	\param[in] Buffer					读取存放缓冲区
	\param[in] len							希望读取的数据个数
	\return  是否成功	
	\retval >=0 实际读取的字节数
	\retval <0  读取失败
*/
s32 BlockDeviceRead(HANDLE IDH,u32 offset,u8* Buffer,u32 len);

/*!

	\brief 块设备写
	\param[in] IDH					设备句柄
	\param[in] offset					开始地址
	\param[in] Buffer				 写入数据存放的缓冲区
	\param[in] len							希望写入的数据个数
	\return  是否成功	
	\retval >=0 实际写入的字节数
	\retval <0  写入失败
*/
s32 BlockDeviceWrite(HANDLE IDH,u32 offset,u8* Buffer,u32 len);

/*!
\brief 设备操作
\param[in] IDH					设备句柄
\param[in] cmd							命令
\param[in] lParam					参数		根据命令的不同参数值不同
\return 根据命令的不同返回值不同	
*/
s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam);

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif


/*! @} */ // end of group1

