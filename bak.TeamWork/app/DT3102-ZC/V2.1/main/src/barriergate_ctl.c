#include "barriergate_ctl.h"
#include "dt3102_io.h"
#include "wet_algorim.h"
#include "weight_param.h"
#include "device_info.h"
#include "task_timer.h"
#include "sys_config.h"
#include "debug_info.h"

static CONTROL_TMR	m_CtrDaoZa;	//道闸事件控制
static CONTROL_TMR	m_CtrDaoZaPrectTmr;	//道闸保护控制时间

#define	CONTROL_DELAY_TMR 	500 //5 S
#define	CONTROL_PRETECT_TMR 	300// //3 S

static int mLcGetWet = 50000;

static int gCloseStatue = CLOSE_IDLE;
static int mCmpCloseWet = 0;

static void BarrierGate_Control(u8 inDo)
{	
	if( inDo == ESTATUE_CTL_OPEN )   //起杆
	{
		m_CtrDaoZa.nCtrlTmr		= 100 ; // 1s 脉冲控制
		m_CtrDaoZa.nStartCtrlFlag = 1;
		m_CtrDaoZa.nCtrlIndex      = 0;	
		
		BarrierGate_JDQ_Control(BarrierGateClosePort, PortNormalOpen); 
		BarrierGate_JDQ_Control(BarrierGateOpenPort,  PortNormalClose);   
		  
		m_CtrDaoZa.nCtrlStatue = ESTATUE_CTL_OPEN;
		debug(Debug_Warning, "Open dao za, %ldms\r\n", Get_Sys_Tick());
	}
	else if( inDo == ESTATUE_CTL_CLOSE)     //下杆        
	{
		m_CtrDaoZa.nCtrlTmr		= 100 ; // 1s 脉冲控制
		m_CtrDaoZa.nStartCtrlFlag = 1;
		m_CtrDaoZa.nCtrlIndex      = 0;	
		//if(m_pWorkIo.eDiGanStatue ==  ESTATUE_DOWNOFF) //做防扎保护
		{
			BarrierGate_JDQ_Control(BarrierGateOpenPort,  PortNormalOpen);
			BarrierGate_JDQ_Control(BarrierGateClosePort, PortNormalClose);
			m_CtrDaoZa.nCtrlStatue = ESTATUE_CTL_CLOSE;
			
		}
		debug(Debug_Warning, "Close dao za, %ldms\r\n", Get_Sys_Tick());
	}
	else if(inDo == ESTATUE_CTL_RESET)
	{
		BarrierGate_JDQ_Control(BarrierGateOpenPort,  PortNormalOpen);	
		BarrierGate_JDQ_Control(BarrierGateClosePort, PortNormalOpen);    
		//m_CtrDaoZa.nCtrlStatue = ESTATUE_CTL_RESET;
		debug(Debug_Warning, "Reset dao za, %ldms\r\n", Get_Sys_Tick());
	}
}

static void BarrierGate_Ctrl_ResetService(void)
{
	//道闸复位时间
	if( m_CtrDaoZa.nStartCtrlFlag == 1)
	{
		m_CtrDaoZa.nCtrlIndex++;
		if(m_CtrDaoZa.nCtrlIndex >= m_CtrDaoZa.nCtrlTmr )
		{
			m_CtrDaoZa.nCtrlIndex = 0;
			m_CtrDaoZa.nStartCtrlFlag  = 0;

			BarrierGate_Control(ESTATUE_CTL_RESET);
		}
	}
}
static char barriergate_open_flag = 0;//开道闸标志，如果有倒车或者逆向车辆要开道闸
void barriergate_open(void)
{
	barriergate_open_flag = 1;
}
//10ms调用一次
void BarrierGate_Service(void)
{
	static u8 WaitBreakIndex = 0;

#if 0	
	//重量降低后，开道闸
	if((mLcCutoffWet - Get_Dynamic_Weight()) > (float)mLcGetWet * 0.9f) 
	{
		mLcCutoffWet = 0;
		mLcGetWet = 50000;
		BarrierGate_Control(ESTATUE_CTL_OPEN);
	}
#endif
	
	//空称开道闸
	if( Get_Dynamic_Weight() < 350)
	{
		if(m_CtrDaoZa.nCtrlStatue ==  ESTATUE_CTL_CLOSE)
		{		
			BarrierGate_Control(ESTATUE_CTL_OPEN);
		}
	}
	//是否有倒车或者逆向行驶车辆
	if(1 == barriergate_open_flag)
	{
		barriergate_open_flag = 0;
		BarrierGate_Control(ESTATUE_CTL_OPEN);
	}
	else
	{
		//关道闸控制
		switch(gCloseStatue)
		{
			case CLOSE_IDLE:
				if(Get_GC_Status(1) && Get_LC_Status(2))
				{
					WaitBreakIndex++;
					if(WaitBreakIndex >= 3)
					{
						WaitBreakIndex = 0;
						mCmpCloseWet = Get_Dynamic_Weight() ;
						gCloseStatue = CLOSE_PREPARE;					
					}
				}
				break;
		
			case CLOSE_PREPARE:
				if((!Get_GC_Status(1)) && ( !Get_LC_Status(2)))
				{
					WaitBreakIndex = 0;
					gCloseStatue = CLOSE_WAIT;
				}
				
				break;			
				
			case CLOSE_WAIT:
			{
				static char disturb_index = 0;
				if(Get_GC_Status(1) && Get_LC_Status(2))
				{
					disturb_index = 0;
					WaitBreakIndex++;
					if(WaitBreakIndex >= 3)
					{
						WaitBreakIndex = 0;
						mCmpCloseWet = Get_Dynamic_Weight();
						gCloseStatue = CLOSE_PREPARE;
						break;
					}
				}			
				else if((Get_Dynamic_Weight() - mCmpCloseWet) > 500)
				{
					disturb_index = 0;
					m_CtrDaoZaPrectTmr.nCtrlIndex	= 0;
					m_CtrDaoZaPrectTmr.nCtrlTmr 	= CONTROL_PRETECT_TMR;
					
					BarrierGate_Control(ESTATUE_CTL_CLOSE);
						
					gCloseStatue = CLOSE_PROTECT;
				}	
				//add_by_StreakingMCU_2013_8_27
				//可能是摩托车干扰
				else
				{
					disturb_index++;
					if(disturb_index >= 4)
					{
						disturb_index = 0;
						gCloseStatue = CLOSE_IDLE;
					}
				}
				//end_add_by_StreakingMCU_2013_8_27
			}
				break;
				
			case CLOSE_PROTECT:
					//道闸保护时间
				m_CtrDaoZaPrectTmr.nCtrlIndex++;
				if(m_CtrDaoZaPrectTmr.nCtrlIndex >= m_CtrDaoZaPrectTmr.nCtrlTmr)
				{
					m_CtrDaoZaPrectTmr.nCtrlIndex	= 0;
					
					gCloseStatue = CLOSE_IDLE;
				}
				
				////3s内后车触发前地感, 开道闸
				if(Get_GC_Status(1)) 
				{
					BarrierGate_Control(ESTATUE_CTL_OPEN);
					gCloseStatue = CLOSE_IDLE;
				}
					
				break;
		}

	}

	//道闸复位控制
	BarrierGate_Ctrl_ResetService();
	
}
extern int GetPreCarAxleNum(void);

void SetLastCarWet(int inwet)
{
	mLcGetWet = inwet;
	if(mLcGetWet < 500)
	{
		mLcGetWet = 500;
	}
}

void BarrierGateAction_After_LC2DownOff(void)
{

}

void BarrierGate_Reset_Init(void)
{
	m_CtrDaoZa.nCtrlIndex = 0;
	m_CtrDaoZa.nCtrlTmr		= 1 * 5 ; // 1s 脉冲控制
	m_CtrDaoZa.nEndCtrlFlag = 0;
	m_CtrDaoZa.nStartCtrlFlag = 0;
	m_CtrDaoZa.nCtrlStatue = ESTATUE_CTL_OPEN;

	m_CtrDaoZaPrectTmr.nCtrlIndex	= 0;
	m_CtrDaoZaPrectTmr.nCtrlStatue	= 0;
	m_CtrDaoZaPrectTmr.nCtrlTmr	= 0;
	m_CtrDaoZaPrectTmr.nEndCtrlFlag	= 0;
	m_CtrDaoZaPrectTmr.nStartCtrlFlag	= 0;
	
	BarrierGate_Control(ESTATUE_CTL_RESET);
}

/**************红绿灯道闸控制******************/
void TrafficSignal_Business(void)
{
	//道闸自动控制
	if(Get_BGFlag()) 
	{
		BarrierGate_Service();
	}
	
	//红绿灯根据设置的重量阈值控制
	if(Get_TrafficSignal_Flag())
	{
		if(Get_Static_Weight() > (int32)gWetPar.MaxTrafficWet)
		{
			//debug(Debug_Business, "------红灯亮-----\r\n");
			fPeripheral_Control(TrafficSignal, 1);
		}
		else
		{
			//debug(Debug_Business, "-----绿灯亮-----\r\n");
			fPeripheral_Control(TrafficSignal, 0);
		}
	}
}


