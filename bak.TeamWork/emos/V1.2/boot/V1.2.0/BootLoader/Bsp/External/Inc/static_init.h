#ifndef _STATIC_INIT_H_	
#define _STATIC_INIT_H_

#ifdef __cplusplus			//定义对CPP进行C处理
extern "C" {
#endif

#include "stm32f4xx.h"
	
//固件地址区
#define AppSpaceSize						262144										//固件区空间大小			256K	大小转换成空间需要-1 空间从0开始存放
#define AppStartAddress					0x8020000									//固件区开始地址			0x8020000
#define AppEndAddress						0x805FFFF									//固件区结束地址			0x805FFFF
	
//固件备份地址区
#define AppBakSpaceSize					262144										//固件备份区空间大小	256K	大小转换成空间需要-1 空间从0开始存放
#define AppStartAddressBak			0x8060000									//固件备份开始地址		0x8060000
#define AppEndAddressBak				0x809FFFF									//固件备份结束地址		0x809FFFF
#define AppCRCAddress						0x809FFFC									//CRC效验码地址				0x809FFFC - 0x809FFFF		4Byte
#define AppSizeAddress					0x809FFF8									//大小存放地址				0x809FFF8 - 0x809FFFB		4Byte
#define AppUpSrcAddress					0x809FFF4									//固件来源						0x809FFF4 - 0x809FFF7		4Byte	

//升级标志地址区		16K AppFlag [0x8008000 - 0x800BFFF]
#define AppFlagSpaceSize				16384											//升级标志区空间大小	16K
#define AppFlagAddress					0x8008000									//升级标志区 					0x8010000 - 0x801FFFF		64K
#define AppUPAddress						0x8008000									//是否更新地址				0x8008000 - 0x8008003		4Byte
#define AppERRAddress						0x8008004									//更新失败状态地址		0x8008004 - 0x8008007		4Byte

//辅助应用程序地址区
#define AuxiliaryAppSpaceSize			65535										//辅助程序区空间大小	64K
#define AuxiliaryAppStartAddress	0x8010000								//辅助程序开始地址区	0x8010000
#define AuxiliaryAppCRCAddress		0x801FFFC								//辅助程序CRC效验码		0x801FFFC
#define AuxiliaryAppSizeAddress		0x801FFF8								//辅助程序大小				0x801FFF8
#define AuxiliaryAppUpSrcAddress	0x801FFF4								//辅助程序来源				0x801FFF4
#define AuxiliaryAppEndAddress		0x801FFFF								//辅助程序结束地址区	0x801FFFF

//ASCII字库地址区		16K ASCIIFont [0x80A0000 - 0x80BFFFF]
#define ASCIIFontSpaceSize			16384											//ASCII字库区空间大小	16K
#define	ASCIIFontStartAddress		0x80A0000									//ASCII字库区开始地址	0x800C000	
#define	ASCIIFontEndAddress			0x80BFFFF									//ASCII字库区结束地址	0x800FFFF		16K
#define ASCIIFontCRCAddress			0x80BFFFC									//ASCII字库CRC效验码	0x80BFFFC - 0x80BFFFF		4Byte
#define ASCIIFontSizeAddress		0x80BFFF8									//ASCII字库大小				0x80BFFF8 - 0x80BFFFB		4Byte
#define ASCIIFontUpSrcAddress		0x80BFFF4									//ASCII字库大小				0x80BFFF4 - 0x80BFFF7		4Byte	


//GB字库地址区			225K GBFont [0x80C0000 - 0x80FFFFF]
#define GBFontSpaceSize					262144										//GB字库区空间大小		256K
#define	GBFontStartAddress			0x80C0000									//GB字库区开始地址		0x80A0000
#define	GBFontEndAddress				0x80FFFFF									//GB字库区结束地址		0x80DFFFF		256K
#define GBFontCRCAddress				0x80FFFFC									//GB字库CRC效验码			0x80FFFFC - 0x80FFFFF		4Byte
#define GBFontSizeAddress				0x80FFFF8									//GB字库大小					0x80FFFF8 - 0x80FFFFB		4Byte
#define GBFontUpSrcAddress			0x80FFFF4									//GB字库来源					0x80FFFF4 - 0x80FFFF7		4Byte
	
//标志区标志地址
//Application数据用标志
#define ReadAppCRCAddress()					(*((volatile u32*)(AppCRCAddress)))							//读取固件文件CRC校验值
#define ReadAppSizeAddress()				(*((volatile u32*)(AppSizeAddress)))						//读取固件文件Size大小
#define ReadAppUpSrcAddress()				(*((volatile u32*)(AppUpSrcAddress)))						//读取固件文件来源
#define ReadAppUPFlagAddress()			(*((volatile u32*)(AppUPAddress)))							//读取更新标志	PCBOOT NETBOOT
#define ReadAppErrFlagAddress()			(*((volatile u32*)(AppERRAddress)))							//读取更新状态标志	正常0	非0不正常查代码找原因

//辅助程序标志地址
//辅助程序数据用标志
#define ReadAuxiliaryAppCRCAddress()				(*((volatile u32*)(AuxiliaryAppCRCAddress)))		//读取辅助程序文件CRC校验值
#define ReadAuxiliaryAppSizeAddress()				(*((volatile u32*)(AuxiliaryAppSizeAddress)))		//读取辅助程序文件Size大小
#define ReadAuxiliaryAppUpSrcAddress()			(*((volatile u32*)(AuxiliaryAppUpSrcAddress)))	//读取辅助程序文件来源


//GB字库数据用标志
#define ReadGBCRCAddress()				(*((volatile u32*)(GBFontCRCAddress)))						//读取GB字库校验值
#define ReadGBSizeAddress()				(*((volatile u32*)(GBFontSizeAddress)))						//读取GB字库大小
#define ReadGBUpSrcAddress()			(*((volatile u32*)(GBFontUpSrcAddress)))					//读取GB字库来源

//ASCII字库数据用标志
#define ReadASCIICRCAddress()			(*((volatile u32*)(ASCIIFontCRCAddress)))					//读取ASCII字库校验值
#define ReadASCIISizeAddress()		(*((volatile u32*)(ASCIIFontSizeAddress)))				//读取ASCII字库大小
#define ReadASCIIUpSrcAddress()		(*((volatile u32*)(ASCIIFontUpSrcAddress)))				//读取ASCII字库来源

//标志区标志值
//数据来源标志
#define PCBOOT 								0x2012				//PC发固件升级标志
#define NETBOOT 							0x2013				//ZX发固件升级标志
//数据跟新标志
#define UPBOOTFlag 						0x1004				//升级标志		有此标志则有新固件包 没有则不需要升级

//标志区错误值
#define	Succeed		0					//升级成功
#define	CRCErr		1					//升级失败	CRC效验错误	
#define	SizeErr		2					//升级失败	文件大小错误
#define	CopyErr		3					//升级失败	备份区代码到应用区代码拷贝出错

//频率类型
#define SYSCLK		0					//SYSCLK频率
#define HCLK			1					//HCLK频率
#define PCLK1			2					//PCLK1频率
#define PCLK2			3					//PCLK2频率

void Delay(const u32 nCount);						//一个工作的频率延时
void Delay_MS(const u32 nCount);				//毫秒级延时
void Delay_S(const u32 nCount_temp);		//秒级延时
void GetCpuIdHex(u8* HCpuId);			//获取CPUID
u32 BeiNumber(const u32 num_temp,const u8 len_temp);		//倍数
u32 ASCIItoNumber(u8* data_temp,const u8 len_temp);	//ASCII to Number
u32 Get_SyS_CLK(const u8 temp);		//获取系统频率

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif
