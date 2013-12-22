#include "barriergate_ctl.h"
#include "dt3102_io.h"
#include "wet_algorim.h"
#include "device_info.h"
#include "task_timer.h"

static CONTROL_TMR	m_CtrDaoZa;	//道闸事件控制
static CONTROL_TMR	m_CtrDaoZaDelayTmr;	//道闸事件控制
static CONTROL_TMR	m_CtrDaoZaPrectTmr;	//道闸保护控制时间

static void BarrierGate_Control(u8 inDo)
{	
	if( inDo == ESTATUE_CTL_OPEN )   //起杆
	{
		m_CtrDaoZa.nCtrlTmr		= 1 * 5 ; // 1s 脉冲控制
		m_CtrDaoZa.nStartCtrlFlag = 1;
		m_CtrDaoZa.nCtrlIndex      = 0;	
		
		BarrierGate_JDQ_Control(BarrierGateClosePort, PortNormalOpen); 
		BarrierGate_JDQ_Control(BarrierGateOpenPort,  PortNormalClose);   
		  
		m_CtrDaoZa.nCtrlStatue = ESTATUE_CTL_OPEN;
		debug(Debug_Warning, "Open dao za, %ldms\r\n", Get_Sys_Tick());
	}
	else if( inDo == ESTATUE_CTL_CLOSE)     //下杆        
	{
		m_CtrDaoZa.nCtrlTmr		= 1 * 5 ; // 1s 脉冲控制
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

		debug(Debug_Warning, "Reset dao za, %ldms\r\n", Get_Sys_Tick());
	}
}

//200ms调用一次
static void BarrierGate_Auto_Bussines(void)
{
	//道闸延时控制
	if(m_CtrDaoZaDelayTmr.nStartCtrlFlag == 1 )
	{
		m_CtrDaoZaDelayTmr.nCtrlIndex++;
		if(m_CtrDaoZaDelayTmr.nCtrlIndex >= m_CtrDaoZaDelayTmr.nCtrlTmr)
		{
			m_CtrDaoZaDelayTmr.nCtrlIndex = 0;
			m_CtrDaoZaDelayTmr.nStartCtrlFlag = 0;
			
			m_CtrDaoZaPrectTmr.nCtrlStatue = 1;
			m_CtrDaoZaPrectTmr.nCtrlIndex   = 0;
			m_CtrDaoZaPrectTmr.nCtrlTmr	    = 7;
		}

		if( (Get_Static_Weight() > 400) && !Get_GC_Status(1)) 
		{
			m_CtrDaoZaPrectTmr.nCtrlStatue = 1;
			m_CtrDaoZaPrectTmr.nCtrlIndex   = 0;
			m_CtrDaoZaPrectTmr.nCtrlTmr	    = 7;
			BarrierGate_Control(ESTATUE_CTL_CLOSE);
			m_CtrDaoZaDelayTmr.nStartCtrlFlag = 0;
		}
	}
	
	//道闸保护时间
	if(m_CtrDaoZaPrectTmr.nCtrlStatue == 1)
	{
		m_CtrDaoZaPrectTmr.nCtrlIndex++;
		if(m_CtrDaoZaPrectTmr.nCtrlIndex >= m_CtrDaoZaPrectTmr.nCtrlTmr)
		{
			m_CtrDaoZaPrectTmr.nCtrlStatue = 0;
			m_CtrDaoZaPrectTmr.nCtrlIndex   = 0;
			m_CtrDaoZaPrectTmr.nCtrlTmr	    = 7;
		}
		
		//1.4s内后车触发前地感, 开道闸
		if(Get_GC_Status(1)) 
		{
			BarrierGate_Control(ESTATUE_CTL_OPEN);
		}

		//秤台重量大于400kg, 并且前地感已释放则关闭道闸
		if((Get_Static_Weight() > 400) && !Get_GC_Status(1)) 
		{
			BarrierGate_Control(ESTATUE_CTL_CLOSE);
		}
		
	}
	//道闸复位时间
	if( m_CtrDaoZa.nStartCtrlFlag == 1)
	{
		m_CtrDaoZa.nCtrlIndex++;
		if(m_CtrDaoZa.nCtrlIndex >= m_CtrDaoZa.nCtrlTmr )
		{
			m_CtrDaoZa.nCtrlIndex = 0;
			m_CtrDaoZa.nStartCtrlFlag  = 0;
			m_CtrDaoZa.nEndCtrlFlag = 1;
			BarrierGate_Control(ESTATUE_CTL_RESET);
		}
	}
}

//10ms调用一次
void BarrierGate_Service(void)
{
	static u8 cnt = 0;
	
	if(Get_Static_Weight() < 500)
	{
		if(m_CtrDaoZa.nCtrlStatue ==  ESTATUE_CTL_CLOSE)
			BarrierGate_Control(ESTATUE_CTL_OPEN);
	}

	cnt++;
	if(cnt >= 20)
	{
		cnt = 0;
		BarrierGate_Auto_Bussines();
	}
}

void BarrierGateAction_After_LC2DownOff(void)
{
	if(Get_Static_Weight() > 400) 	
	{
		m_CtrDaoZaDelayTmr.nStartCtrlFlag = 1;
		m_CtrDaoZaDelayTmr.nCtrlTmr	 	= 7;  ////1.4s
		m_CtrDaoZaDelayTmr.nCtrlIndex 	= 0;
		m_CtrDaoZaDelayTmr.nEndCtrlFlag	= 7;
	}
}

void BarrierGate_Reset_Init(void)
{
	m_CtrDaoZa.nCtrlIndex = 0;
	m_CtrDaoZa.nCtrlTmr		= 1 * 5 ; // 1s 脉冲控制
	m_CtrDaoZa.nEndCtrlFlag = 0;
	m_CtrDaoZa.nStartCtrlFlag = 0;
	m_CtrDaoZa.nCtrlStatue = ESTATUE_CTL_OPEN;

	m_CtrDaoZaDelayTmr.nCtrlIndex	= 0;
	m_CtrDaoZaDelayTmr.nCtrlStatue	= 0;
	m_CtrDaoZaDelayTmr.nCtrlTmr	= 0;
	m_CtrDaoZaDelayTmr.nEndCtrlFlag	= 0;
	m_CtrDaoZaDelayTmr.nStartCtrlFlag	= 0;

	m_CtrDaoZaPrectTmr.nCtrlIndex	= 0;
	m_CtrDaoZaPrectTmr.nCtrlStatue	= 0;
	m_CtrDaoZaPrectTmr.nCtrlTmr	= 0;
	m_CtrDaoZaPrectTmr.nEndCtrlFlag	= 0;
	m_CtrDaoZaPrectTmr.nStartCtrlFlag	= 0;
	
	BarrierGate_Control(ESTATUE_CTL_RESET);
}
