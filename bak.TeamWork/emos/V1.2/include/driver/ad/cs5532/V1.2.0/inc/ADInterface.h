/************************************************************************************
**																			 
**																			 
**																			 
**--------文 件 信 息----------------------------------------------------------------
**文   件   名：AD&DAInterface.h														 
**创   建   人：														 
**创 建  时 间：													 
**最后修改时间：															      
**描        述：
************************************************************************************/
#ifndef _AD_DA_INTERFACE_H_
#define _AD_DA_INTERFACE_H_


#define ADCheck		0
#define ADRUNINIT	1
#define ADRUN		2


#define	OS_EVENT	SYS_EVENT
#define	OSTimeDly	SysTimeDly

//偏移，增益，通道寄存器	分全部或单个设置	
//配置寄存器				只有单个设置
#define ReadALLRegister		0x48	//读所有寄存器	
#define WriteALLRegister	0x40	//写所有寄存器
#define ReadRegister		0x08	//读单个寄存器
#define WriteRegister		0x00	//写单个寄存器

#define SYNC1	0xFF		//命令关键字
#define SYNC0	0xFE		//命令关键字

//命令寄存器	发送发送15个字节的SYNC1(0xFFH)+1字节的SYNC0(0xFEH)后进入命令寄存器
/******************************************
D07	D06	D05	D04	|	D03	D02	 D01	D00
C	ARA	CS1	CS0		R/W	RSB2 RSB1	RSB0
*******************************************/
// C			命令寄存器是否有效	0有效	1无效
// ARA			以阵列方式访问寄存器	0忽略	1阵列方式访问各寄存器(偏移、增益和通道设置寄存器)，
// 				寄存器选择位(RSB)决定了索要访问的特殊寄存器，访问寄存器时高位(MSB)在前，并以物理通道0，1，2等依次进行
// CS1-CS0		通道选择位
// 				00 CS1-CS0提供一个物理通道地址，也可用于访问与各物理通道相关的校准寄存器，当读数据寄存器时，各位失效
// R/W			读写所选寄存器	0写	1读
// RSB2-RSB0	寄存器选择位
// 				001	1	偏移寄存器
// 				010	2	增益寄存器
// 				011	3	配置寄存器
// 				101	5	通道设置寄存器
// 				其它	无

//对单个偏移寄存器设置为 ExcursionRegister + Excursion_1
#define ExcursionRegister	0x01	//偏移寄存器
#define Excursion_1	0x00			//偏移寄存器1
#define Excursion_2	0x10			//偏移寄存器2
#define Excursion_3	0x20			//偏移寄存器3(cs5533/cs5534)
#define Excursion_4	0x30			//偏移寄存器4(cs5533/cs5534)

//对单个增益寄存器设置为 GainRegister + GainSet_1
#define GainRegister		0x02	//增益寄存器访问命令
#define GainSet_1	0x00			//增益寄存器1
#define GainSet_2	0x10			//增益寄存器2
#define GainSet_3	0x20			//增益寄存器3(cs5533/cs5534)
#define GainSet_4	0x30			//增益寄存器4(cs5533/cs5534)

//配置寄存器	0x03	单个配置
/**** 0 ******************* 0 *************** 0 ***************** 0 ******************* 0 ************** 0 ****************** 0 **************** 0 ********
D31	D30	D29	D28	|	D27	D26	D25	D24	|	D23	D22	D21	D20	|	D19	D18	D17	D16	|	D15	D14	D13	D12	|	D11	D10	D09	D08	|	D07	D06	D05	D04	|	D03	D02	D01	D00
PSS	PDW	RS	RV	|	IS	GB	VRS	A1	|	A0	OLS	N	OGS	|	FRS	N	N	N	|	N	N	N	N	|	N	N	N	N	|	N	N	N	N	|	N	N	N	N
**********************************************************************************************************************************************************/
#define ConfigRegister	0x03	//配置寄存器

#define PSS		//节电模式选择	
//0		待机模式(振荡器工作，允许快速上电)	
//1		休眠模式(振荡器不工作)
#define PDW		//节电模式		
//0		常规模式	
//1		节电模式
#define RS1		0x20000000		//系统复位	1		激活一次复位(复位后自动变0)
#define RS0		0x00000000		//系统复位	0		正常操作
#define RV		//复位有效		
//0		正常操作	
//1		系统已复位。只读，读配置寄存器后被清0
#define IS		//输入短路		
//0		正常输入	
//1		各通道的输入都在内部短路
#define GB		//保护信号位	
//0		将A0设置为正常的输出锁存引脚	
//1		A0被设置为输出仪表放大器的共模输出电压(典型值2.5V)，当保护缓冲器被激活时，输出锁存选择位被忽略
#define VRS0	0x00000000		//电压参考选择	0	2.5V < Vref <= VA+
#define VRS1	0x02000000		//电压参考选择	1	1V <= Vref <= 2.5V

#define A1_A0	//输出锁存位 ( 00 A0=0,A1=0 | 01 A0=0,A1=1 | 10 A0=1,A1=0 | 11 A0=1,A1=1 )
//当配置寄存器中的输出锁存选择位OLS为1时，输出锁存位(A0-A1)将根据所执行的命令字被设置为相应的逻辑状态，注意输出逻辑电压由VA+和VA-供电
#define OLS		//输出锁存选择
//0		低电平时，通道设置寄存器作为A0和A1的源
//1		高电平时，配置寄存器作为A0和A1的源
#define OGS		//偏移与增益选择
//0		按所用的设置单元(Setup)中的CS1-CS0位选择校准寄存器
//1		按所用的设置单元(Setup)中的OG1-OG0位选择校准寄存器
#define FRS0	0x00000000		//60HZ	滤波速率选择	0	使用缺省输出字速率 快
#define FRS1	0x00080000		//50HZ	滤波速率选择	1	输出字速率及相应的滤波特性乘以系数5/6 慢1/6

//通道寄存器	0x05	单个配置	所有配置
/*************************** Setup1 ****************************************|******************************** Setup2 **************************************
D31	D30	D29	D28	|	D27	D26	D25	D24	|	D23	D22	D21	D20	|	D19	D18	D17	D16	|	D15	D14	D13	D12	|	D11	D10	D09	D08	|	D07	D06	D05	D04	|	D03	D02	D01	D00
CS1	CS0	G2	G1	|	G0	WR3	WR2	WR1	|	WR0	U/B	OL1	OL0	|	DT	OCD	OG1 OG0	|	CS1	CS0	G2	G1	|	G0	WR3	WR2	WR1	|	WR0	U/B	OL1	OL0	|	DT	OCD	OG1 OG0
****** 0 **************** 0 ******************* 0 ***************** 0 **************** 0 ****************** 0 ****************** 0 *************** 0 *****/
//对单个通道配置寄存器设置为 ChannelRegister + ChannelSet_1
//对所有通道配置寄存器设置为 ChannelRegister
#define ChannelRegister	0x05	//通道寄存器访问命令
#define ChannelSet_1	0x00	//通道设置寄存器1
#define ChannelSet_2	0x10	//通道设置寄存器2
#define ChannelSet_3	0x20	//通道设置寄存器3
#define ChannelSet_4	0x30	//通道设置寄存器4

//CS1 CS0	物理通道选择位
#define SelectChannel_1	0x00000000		//通道选择位(选择物理通道)		00	选择物理通道1 	//一个物理通道有2个设置单元，单元的选择在转换命令时发送
#define SelectChannel_2	0x40000000		//通道选择位(选择物理通道)		01	选择物理通道2
#define SelectChannel_3	0x80000000		//通道选择位(选择物理通道)		10	选择物理通道3(仅CS5533/34)
#define SelectChannel_4	0xC0000000		//通道选择位(选择物理通道)		11	选择物理通道4(仅CS5533/34)

//G2 G1		增益选择位			对VRS=0，A=2；VRS=1，A=1；双极性输入范围是单极性输入范围的两倍
#define Gain_1	0x00000000		//000	增益=1 ，单极性输入范围=[(VREF+)-(VREF-)]/1*A
#define Gain_2	0x08000000		//001	增益=2 ，单极性输入范围=[(VREF+)-(VREF-)]/2*A
#define Gain_4	0x10000000		//010	增益=4 ，单极性输入范围=[(VREF+)-(VREF-)]/4*A
#define Gain_8	0x18000000		//011	增益=8 ，单极性输入范围=[(VREF+)-(VREF-)]/8*A		
#define Gain_16	0x20000000		//100	增益=16，单极性输入范围=[(VREF+)-(VREF-)]/16*A
#define Gain_32	0x28000000		//101	增益=32，单极性输入范围=[(VREF+)-(VREF-)]/32*A
#define Gain_64	0x30000000		//110	增益=64，单极性输入范围=[(VREF+)-(VREF-)]/64*A

//WR3-WR0	字速率				时钟频率为4.9152MHZ的连续转换模式，连续转换模式的第一次转换所用时间比单次转换时间长
#define FRS_Seed_4	0x00000000		//0000	120  sps		100  sps
#define FRS_Seed_3	0x00800000		//0001	60   sps		50   sps
#define FRS_Seed_2	0x01000000		//0010	30   sps		25   sps
#define FRS_Seed_1	0x01800000		//0011	15   sps		12.5 sps
#define FRS_Seed_0	0x02000000		//0100	7.5  sps		6.25 sps
#define FRS_Seed_9	0x04000000		//1000	3840 sps		3200 sps
#define FRS_Seed_8	0x04800000		//1001	1920 sps		1600 sps
#define FRS_Seed_7	0x05000000		//1010	960  sps		800  sps
#define FRS_Seed_6	0x05800000		//1011	480  sps		400  sps
#define FRS_Seed_5	0x06000000		//1100	240  sps		200  sps

//U/B	单/双极性
#define UB0	0x00000000		//0	单极性模式	 //纠正：0为双极性模式
#define UB1	0x00400000		//1	双极性模式	 //纠正：1为单极性模式

//OL1 OL0	输出锁存位		当配置寄存器中的输出锁存选择(OLS)位设为逻辑‘0’时，输出锁存位将根据所执行的命令字被设置为相应的逻辑状态，注意片内逻辑输出由VA+和VA--供电
#define Latch_0	0x00000000		//00	A0=0，A1=0
#define Latch_1	0x00100000		//01	A0=0，A1=1
#define Latch_2	0x00200000		//10	A0=1，A1=0
#define Latch_3	0x00300000		//11	A0=1，A1=1

//DT	时延位	置位时，ADC将在转换前等待一段延迟时间，这将在转换开始前给出A0、A1输出的稳定时间，当FRS=0时，该延时时间为1280个MCLK周期，当FRS=1时，该延时时间为1536个MCLK周期
#define DT0	0x00000000		//0	立即开始转换
#define DT1	0x00080000		//1	等待1280或1536个MCLK周期后开始转换

//OCD	开路检测位	置位时，在通道选择位所选的输入通道(AIN+)将激活一个300nA的电流源，注意该电流源值是在温度为25摄氏度时的值，在-55摄氏度时，该电流源增加到600nA。当用户想要将驱动一个连到供电线的被怀疑开路的热电耦时，此功能特别有用
#define OCD0	0x00000000	//正常模式
#define OCD1	0x00040000	//激活电流源

//OG1 OG0	偏移/增益寄存器指针位		配置寄存器中的OGS位置1时有效。允许用户选择转换或校准时使用的偏移和增益寄存器，OGS=0时，将访问所选物理通道(由CS1-CS0选定)对应的偏移和增益寄存器
#define Gain_Excursion_1	0x00000000	//00	从物理通道1选用偏移和增益寄存器
#define Gain_Excursion_2	0x00010000	//01	从物理通道2选用偏移和增益寄存器
#define Gain_Excursion_3	0x00020000	//10	从物理通道3选用偏移和增益寄存器
#define Gain_Excursion_4	0x00030000	//11	从物理通道4选用偏移和增益寄存器

//执行转换
#define StartConvert		0x80	//执行转换
#define Single				0x00	//单次转换
#define Continuation		0x40	//连续转换
#define ChannelPointer_1	0x00	//000 Setup1		 //一个物理通道有2个设置单元：SETUP1 SETUP2
#define ChannelPointer_2	0x08	//001 Setup2
#define ChannelPointer_3	0x10	//010 Setup3
#define ChannelPointer_4	0x18	//011 Setup4
#define ChannelPointer_5	0x20	//100 Setup5
#define ChannelPointer_6	0x28	//101 Setup6
#define ChannelPointer_7	0x30	//110 Setup7
#define ChannelPointer_8	0x38	//111 Setup8



#define	SETUP1_VRS		VRS0
#define	SPIB_SETUP1_CH		SelectChannel_1
#define	SPIB_SETUP1_GAIN	Gain_64
#define	SPIB_SETUP1_FRS		FRS_Seed_7
#define	SPIB_SETUP1_UB		UB0
#define SPIB_SETUP1_LATCH	Latch_0
#define	SPIB_SETUP1_DT		DT0
#define	SPIB_SETUP1_OCD		OCD1
#define	SPIB_SETUP1_GE		Gain_Excursion_1

#define	SPIB_SETUP2_CH		SelectChannel_1
#define	SPIB_SETUP2_GAIN	Gain_64
#define	SPIB_SETUP2_FRS		FRS_Seed_7
#define	SPIB_SETUP2_UB		UB0
#define SPIB_SETUP2_LATCH	Latch_0
#define	SPIB_SETUP2_DT		DT0
#define	SPIB_SETUP2_OCD		OCD0
#define	SPIB_SETUP2_GE		Gain_Excursion_2	

#define		SETUP1		(SPIB_SETUP1_CH + 	\
						SPIB_SETUP1_GAIN + 	\
						SPIB_SETUP1_FRS + 	\
						SPIB_SETUP1_UB + 	\
						SPIB_SETUP1_LATCH + \
						SPIB_SETUP1_DT + 	\
						SPIB_SETUP1_OCD + 	\
						SPIB_SETUP1_GE)

#define		SETUP2		((SPIB_SETUP2_CH + 	\
						SPIB_SETUP2_GAIN + 	\
						SPIB_SETUP2_FRS + 	\
						SPIB_SETUP2_UB + 	\
						SPIB_SETUP2_LATCH + \
						SPIB_SETUP2_DT + 	\
						SPIB_SETUP2_OCD + 	\
						SPIB_SETUP2_GE)>>16)	
						
#define NS_50   6

#define CS5532_CS_Delay(x)   {u8 i; \
                              i = x;    \
                              while(i--);}
#endif
