#include "DyncSensorGetAxisAlgo.h"
#include <string.h>
#include <math.h>
#include "stdio.h"

typedef struct tag_TPlatForm 
{
	int index; 
	int max;  //本平台的最高点
    	int min;  //本平台的最低点
	int width;  //平台的长度
	int avg;  //平台长度内的平均值

	int firstMax; //平台每一个波峰
	int firstMin; //平台每一个波谷
	int limit;  //平台的阀值
	int limit_cnt; //平台的阀值个数

	int papo_len; //该平台爬坡长度
	int papo_avg; //平台爬坡平均值
}TPlatForm;
#define MAX_PLATFORM          30
#define up_cnt_limit  1
#define dn_cnt_limit  1

sSoftAlexDyncProc *pDync = NULL;
static int bOut = 0;

TPlatForm g_platform[MAX_PLATFORM];
TPlatForm g_tmp_platform;

int g_rate = 1;
int g_up_limit = 500;
int g_bfKg = 0;
int g_car_arrival = 0;
int g_cur_idx = 0;
int g_up_cnt = 0;
int g_dn_cnt = 0;
int g_up_flag = 0;
int g_dn_flag = 0;
int g_papo_flag = 0;
int g_first_flag = 0;
int g_new_platform_flag = 0;
int g_up_ad = 0;
int g_up_up_flag = 0;


 
void f_papo_process(int kg);
void f_copy_platform(void);

void hspd_Weight_PostAnalysisNoAddAxis(void);
void hspd_Weight_PostAnalysis(void);
void hspd_Weight_PostAnalysisAddAxis(void);
void hspd_Weight_PostAnalysisAllAddAxis(void);
void hspd_Weight_PostAnalysisFEAddAxis(void);
void hspd_CarOver(void);

void output_result(void);

void format_SerDebug(const char *param,...)
{

}
void spd_Weight_Init()
{
	g_car_arrival = 0;
	g_cur_idx = 0;
	g_up_cnt = 0;
	g_dn_cnt = 0;
	g_up_flag = 0;
	g_dn_flag = 0;
	g_papo_flag = 0;
	g_first_flag = 0;

	g_up_ad = 0;
	g_up_up_flag = 0;

	g_platform[0].firstMax = 0;
	g_cur_idx = 0;
	g_new_platform_flag = 0; 

	g_tmp_platform.papo_avg = 0;
	g_tmp_platform.papo_len = 0;

	g_first_flag = g_new_platform_flag = 1;
	memset(&g_tmp_platform,0,sizeof(TPlatForm));
	memset(g_platform,0,MAX_PLATFORM*sizeof(TPlatForm));

}


void Weight_Maybe_New_Platform(int kg)
{
	g_tmp_platform.papo_len = 1;
	g_tmp_platform.papo_avg = kg;
	g_papo_flag = 1;
}
void Weight_Maybe_New_Platform_Up(int kg)
{
	f_papo_process(kg);
}

void f_papo_process(int kg)
{
	//g_platform[g_cur_idx].papo_len++;
	int avg = g_tmp_platform.papo_avg;
	int len = g_tmp_platform.papo_len;


	avg = (avg * len + kg) / (len + 1);
	//g_platform[g_cur_idx].papo_avg = avg;
	//g_platform[g_cur_idx].papo_len++;

	g_tmp_platform.papo_avg = avg;
	g_tmp_platform.papo_len++;

}
void f_up_process(int kg)
{
	
}
void f_down_process(int kg)
{
	
}
/************************************************************************/
/* 拷贝临时平台数据到当前平台中                                                                     */
/************************************************************************/
void f_copy_platform()
{
	g_platform[g_cur_idx].firstMin  = g_tmp_platform.firstMin;
	g_platform[g_cur_idx].avg       = g_tmp_platform.avg;
	g_platform[g_cur_idx].firstMax  = g_tmp_platform.firstMax;
	g_platform[g_cur_idx].width     = g_tmp_platform.width;
	g_platform[g_cur_idx].max       = g_tmp_platform.max;
	g_platform[g_cur_idx].min       = g_tmp_platform.min;
	g_platform[g_cur_idx].limit     = g_tmp_platform.limit;
	g_platform[g_cur_idx].limit_cnt = g_tmp_platform.limit_cnt;
	g_platform[g_cur_idx].papo_len  = g_tmp_platform.papo_len;
	g_platform[g_cur_idx].papo_avg  = g_tmp_platform.papo_avg;
	g_platform[g_cur_idx].index     = g_cur_idx;

}
/************************************************************************
 找到一个最高点的时候              
************************************************************************/
void f_find_max_pt(int kg)
{
	if (g_papo_flag) //爬坡结束后的第一个最高点
	{

		f_copy_platform();

		g_tmp_platform.width    = 1;
		g_tmp_platform.avg      = kg;
		g_tmp_platform.limit_cnt= 0;
		g_tmp_platform.limit    = 0;
		g_tmp_platform.papo_avg = 0;
		g_tmp_platform.papo_len = 0;
		g_tmp_platform.min      = 0xffffff;

		g_papo_flag  = 0;
		g_first_flag = 1;
		g_up_up_flag = 0;
        
	}else{

	}

	g_tmp_platform.firstMax = kg;
	if(g_tmp_platform.max < kg){ //更新平台最高点
		g_tmp_platform.max = kg;
	}

}

void f_find_min_pt(int kg)
{
	if (g_first_flag)
	{
		
		if (kg > g_platform[g_cur_idx].max) //说明上一个平台已经确定，现在在新的平台上了
		{
			g_cur_idx++;
			//f_copy_platform();
			g_new_platform_flag = 1;
            g_up_up_flag = 0;

		}else{
			int width = (g_platform[g_cur_idx].width+g_platform[g_cur_idx].papo_len+g_tmp_platform.width);
			int temp = (g_platform[g_cur_idx].avg * g_platform[g_cur_idx].width +
				        g_platform[g_cur_idx].papo_avg * g_platform[g_cur_idx].papo_len +
						g_tmp_platform.avg*g_tmp_platform.width) / width;
			//g_platform[g_cur_idx].avg   = temp;
			//g_platform[g_cur_idx].width = width;
			//g_platform[g_cur_idx].max   = g_tmp_platform.max;
			g_tmp_platform.avg   = temp;
			g_tmp_platform.width = width;
            g_tmp_platform.limit_cnt +=  g_platform[g_cur_idx].limit_cnt;
            g_tmp_platform.limit += g_platform[g_cur_idx].limit;
			//g_tmp_platform.width = width;



		}
		g_first_flag = 0;

	}

	g_tmp_platform.limit = g_rate * ( g_tmp_platform.limit * g_tmp_platform.limit_cnt + (g_tmp_platform.firstMax - kg) ) / (  (g_tmp_platform.limit_cnt + 1) * 2 );
	g_tmp_platform.limit_cnt++;

    if(g_tmp_platform.min > kg){ //更新平台最高点
		g_tmp_platform.min = kg;
	}
}
void f_lookfor_max_min_pt(int kg)
{
 
	if ( kg >= g_bfKg ) //找低点
	{
		g_up_cnt++;
		g_dn_cnt = 0;
		
		if(g_up_cnt >= up_cnt_limit)
		{

            g_up_cnt  = 0;
			g_dn_flag = 1;

            if(g_up_flag)
            {
                g_up_flag = 0;
                f_find_min_pt(g_bfKg);
            }


		}
	}else if (  kg < g_bfKg) //找高点
	{
		g_dn_cnt++;
		g_up_cnt = 0;
		if(g_dn_cnt >= dn_cnt_limit)
		{
          	g_dn_cnt  = 0;
			g_up_flag = 1; 


            if(g_dn_flag)
            {
                f_find_max_pt(g_bfKg);
                g_dn_flag = 0;
            }

		}
	}
}
void f_platform_process(int kg)
{
	int avg = g_tmp_platform.avg;
	int len = g_tmp_platform.width;


    /*
	if ( (kg > avg + g_tmp_platform.limit) ) //超出波动范围，也可能是新的爬坡过程
	{
		if(!g_papo_flag)
		{
			Weight_Maybe_New_Platform(kg);
		}
		Weight_Maybe_New_Platform_Up(kg);
		//g_new_platform_flag = 0;
	}
	else
	{
		avg = (avg * len + kg) / (len + 1);

		g_tmp_platform.avg = avg;
		g_tmp_platform.width++;
	}
    */

    if ( (kg > avg + g_tmp_platform.limit) && (kg > g_tmp_platform.max ) && (kg  > g_up_ad + 10 )  ) //超出波动范围，也可能是新的爬坡过程
	{
        g_up_up_flag = 1;
	}
	else
	{

	}
    if(g_up_up_flag )
    {
        if(!g_papo_flag)
		{
			Weight_Maybe_New_Platform(kg);
		}
		Weight_Maybe_New_Platform_Up(kg);
		//g_new_platform_flag = 0;
    }
    else
    {
        avg = (avg * len + kg) / (len + 1);

		g_tmp_platform.avg = avg;
		g_tmp_platform.width++;
    }
    
    g_up_ad = kg;  


}
void hspd_CarOver(void)
{
	  
    if(g_platform[g_cur_idx].width != 0 )
        g_cur_idx++;
    f_copy_platform();
    if(pDync->mFixAlexMode == 0) //平坡补偿模式 补偿 前轴
    {
    	hspd_Weight_PostAnalysis();
    }
    else if(pDync->mFixAlexMode == 1) //斜坡补偿模式 5轴以上补偿
    {
    	hspd_Weight_PostAnalysisAddAxis();
    }
    else if(pDync->mFixAlexMode == 2) //全斜坡补偿模式  全波 补偿
    {
    	hspd_Weight_PostAnalysisAllAddAxis();
    }
    else if(pDync->mFixAlexMode == 3) // 补偿 前轴 和 5轴以上补偿
    {
    	hspd_Weight_PostAnalysisFEAddAxis();
    }    
    else if(pDync->mFixAlexMode == 4) // 补偿 前轴 和 5轴以上补偿
    {
    	hspd_Weight_PostAnalysisNoAddAxis();
    }
    else
    {
    	hspd_Weight_PostAnalysisAllAddAxis();
    }
    
}



void hspd_Weight_PostAnalysisFEAddAxis(void)
{
     int i = 0,j=0;
     int m_Num = g_cur_idx;
     int m_OldNum = m_Num;

    for(i = m_Num; i >= 0;i--)   //计算轴重
    {         //g_platform[i].avg = g_platform[i ].avg - g_platform[i - 1].avg ;
        g_platform[i].width     = g_platform[i].width + g_platform[i].papo_len;
        format_SerDebug("Find width=%d ,avg=%d,min=%d,max=%d papo_len=%d\r\n",g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max,g_platform[i].papo_len);
    }
	format_SerDebug("###############pDync->m_CmpSpeddPT=%d####%d ##########\r\n",pDync->m_CmpSpeddPT,pDync->m_CmpSpeddPT);
     for(i = 1;i < m_Num ;i++)
     {
         if(  g_platform[i].width < pDync->m_CmpSpeddPT
           || g_platform[i].avg <  ( g_platform[i - 1].avg + (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK)
           || g_platform[i].avg >  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK ) // * 0.2
           || g_platform[i].min < g_platform[i - 1].max
           )
         {
         	format_SerDebug("del data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);
            j = 0;
            
            if( g_platform[i].width < pDync->m_CmpSpeddPT
             && g_platform[i].avg <  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK ) // * 0.2
           )
            {
               g_platform[i + j - 1].papo_len  += ( g_platform[i + j ].width );
               //g_platform[i + j].width  = g_platform[i + j + 1].width;
            }
            else
            {
                 //g_platform[i + j ].width += g_platform[i + j ].papo_len ;
                 //g_platform[i + j].width  += g_platform[i + j + 1].width;
            }
                
            g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
            g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
            g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
            g_platform[i + j].width     += g_platform[i + j + 1].width;
            g_platform[i + j].max       = g_platform[i + j + 1].max;
            g_platform[i + j].min       = g_platform[i + j + 1].min;
            g_platform[i + j].limit     = g_platform[i + j + 1].limit;
            g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
            g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
            g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
            g_platform[i + j].index     = g_platform[i + j + 1].index;
            for(j = 1;j<= m_Num - i -1;j++)
            {
              g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
              g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
              g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
              g_platform[i + j].width     = g_platform[i + j + 1].width;
              g_platform[i + j].max       = g_platform[i + j + 1].max;
              g_platform[i + j].min       = g_platform[i + j + 1].min;
              g_platform[i + j].limit     = g_platform[i + j + 1].limit;
              g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
              g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
              g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
              g_platform[i + j].index     = g_platform[i + j + 1].index;
            }
            i--;    //判断多个条件用
            g_cur_idx--;
            m_Num--;
         }
         //memcpy(g_Resultplatform+i,g_platform+i,sizeof(TPlatForm));
     }
     //memcpy(g_Resultplatform+i,g_platform+i,sizeof(TPlatForm));
     ////////////////补偿第一轴
     m_Num = g_cur_idx;// - 1 ;
     for(i = 0;i < 1;i++)   //判断爬坡上有没有一个轴数据
     {
        if(   g_platform[i].papo_len > pDync->m_CmpSpeddPT
           //&& g_platform[i].papo_avg >  ( g_platform[i - 1].avg + g_platform[i].papo_avg * 0.05 * g_AxisK )
           && g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * pDync->m_LimitK )
           )
         {
            g_platform[i].index = 0;
            
            for(j = 0;j< m_Num - i;j++)
            {
                g_platform[m_Num - j + 1].firstMin    = g_platform[m_Num - j ].firstMin;
                g_platform[m_Num - j + 1].avg         = g_platform[m_Num - j ].avg;
                g_platform[m_Num - j + 1].firstMax    = g_platform[m_Num - j ].firstMax;
                g_platform[m_Num - j + 1].width      += g_platform[m_Num - j ].width;
                g_platform[m_Num - j + 1].max         = g_platform[m_Num - j ].max;
                g_platform[m_Num - j + 1].min         = g_platform[m_Num - j ].min;
                g_platform[m_Num - j + 1].index         = g_platform[m_Num - j ].index;
                g_platform[m_Num - j + 1].limit       = g_platform[m_Num - j ].limit;
                g_platform[m_Num - j + 1].limit_cnt   = g_platform[m_Num - j ].limit_cnt;
                g_platform[m_Num - j + 1].papo_len   += g_platform[m_Num - j ].papo_len;
                g_platform[m_Num - j + 1].papo_avg    = g_platform[m_Num - j ].papo_avg;
            }
            g_platform[m_Num - j + 1].firstMin    = g_platform[i].firstMin;
            g_platform[m_Num - j + 1].avg         = g_platform[i].papo_avg;
            g_platform[m_Num - j + 1].firstMax    = g_platform[i].firstMax;
            g_platform[m_Num - j + 1].width       = g_platform[i].width;
            g_platform[m_Num - j + 1].max         = g_platform[i].max;
            g_platform[m_Num - j + 1].limit       = g_platform[i].limit;
            g_platform[m_Num - j + 1].limit_cnt   = g_platform[i].limit_cnt;
            g_platform[m_Num - j + 1].index		  = 0;
            g_platform[m_Num - j + 1].papo_len    = g_platform[i].papo_len;
            g_platform[m_Num - j + 1].papo_avg    = g_platform[i].papo_avg ;
            g_cur_idx++;
            
         }
     }
     //////////////////////补偿后面
     m_Num = g_cur_idx ;  
     m_OldNum = m_Num;  
     if(m_Num >= 5)
     {
	     for(i = 0;i < m_Num;i++)   //判断第一个 爬坡上有没有一个轴数据
	     {                                 //pDync->m_CmpSpeddPT * 2//
	        if(   g_platform[i].papo_len > pDync->m_CmpSpeddPT * 1.2 //1.5
	           //&& g_platform[i].papo_avg >  ( g_platform[i - 1].avg + g_platform[i].papo_avg * 0.05 * g_AxisK )
	           //&& g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * g_AxisK )
	           && g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * pDync->m_LimitK )
	           )
	         {
	         	if(m_Num < MAX_PLATFORM - 2 )
	         	{
	         	
	         		format_SerDebug("add %d data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",g_cur_idx,i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);
	            
		            g_platform[i].index = 0;
		            for(j = 0;j< m_Num - i;j++)
		            {
		                g_platform[m_Num - j + 1].firstMin    = g_platform[m_Num - j ].firstMin;
		                g_platform[m_Num - j + 1].avg         = g_platform[m_Num - j ].avg;
		                g_platform[m_Num - j + 1].firstMax    = g_platform[m_Num - j ].firstMax;
		                g_platform[m_Num - j + 1].width      += g_platform[m_Num - j ].width;
		                g_platform[m_Num - j + 1].max         = g_platform[m_Num - j ].max;
		                g_platform[m_Num - j + 1].min         = g_platform[m_Num - j ].min;
		                g_platform[m_Num - j + 1].index       = g_platform[m_Num - j ].index;
		                g_platform[m_Num - j + 1].limit       = g_platform[m_Num - j ].limit;
		                g_platform[m_Num - j + 1].limit_cnt   = g_platform[m_Num - j ].limit_cnt;
		                g_platform[m_Num - j + 1].papo_len    = g_platform[m_Num - j ].papo_len;
		                g_platform[m_Num - j + 1].papo_avg    = g_platform[m_Num - j ].papo_avg;
		            }
		            g_platform[m_Num - j + 1].firstMin    = g_platform[i].firstMin;
		            g_platform[m_Num - j + 1].avg         = g_platform[i].papo_avg;
		            g_platform[m_Num - j + 1].firstMax    = g_platform[i].firstMax;
		            g_platform[m_Num - j + 1].width       = g_platform[i].width;
		            g_platform[m_Num - j + 1].max         = g_platform[i].max;
		            g_platform[m_Num - j + 1].index	  = 0;
		            g_platform[m_Num - j + 1].limit       = g_platform[i].limit;
		            g_platform[m_Num - j + 1].limit_cnt   = g_platform[i].limit_cnt;
		            g_platform[m_Num - j + 1].papo_len    = pDync->m_CmpSpeddPT -20 ;//g_platform[i].papo_len;
		            g_platform[m_Num - j + 1].papo_avg    = g_platform[i].papo_avg ;
		            g_cur_idx++;
		            m_Num = g_cur_idx ;
	            }
	         }
		}
     }
     output_result();
}

void hspd_Weight_PostAnalysisAllAddAxis(void)
{
     int i = 0,j=0;
     int m_Num = g_cur_idx;
     int m_OldNum = m_Num;

    for(i = m_Num; i >= 0;i--)   //计算轴重
    {         //g_platform[i].avg = g_platform[i ].avg - g_platform[i - 1].avg ;
        g_platform[i].width     = g_platform[i].width + g_platform[i].papo_len;
        format_SerDebug("Find width=%d ,avg=%d,min=%d,max=%d papo_len=%d\r\n",g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max,g_platform[i].papo_len);
    }
	format_SerDebug("###############pDync->m_CmpSpeddPT=%d####%d ##########\r\n",pDync->m_CmpSpeddPT,pDync->m_CmpSpeddPT);
     for(i = 1;i < m_Num ;i++)
     {
         if(  g_platform[i].width < pDync->m_CmpSpeddPT
           || g_platform[i].avg <  ( g_platform[i - 1].avg + (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK)
           || g_platform[i].avg >  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK ) // * 0.2
           || g_platform[i].min < g_platform[i - 1].max
           )
         {
         	format_SerDebug("del data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);
            j = 0;
            
            if( g_platform[i].width < pDync->m_CmpSpeddPT
             && g_platform[i].avg <  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK ) // * 0.2
           )
            {
               g_platform[i + j - 1].papo_len  += ( g_platform[i + j ].width );
               //g_platform[i + j].width  = g_platform[i + j + 1].width;
            }
            else
            {
                 //g_platform[i + j ].width += g_platform[i + j ].papo_len ;
                 //g_platform[i + j].width  += g_platform[i + j + 1].width;
            }
                
            g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
            g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
            g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
            g_platform[i + j].width     += g_platform[i + j + 1].width;
            g_platform[i + j].max       = g_platform[i + j + 1].max;
            g_platform[i + j].min       = g_platform[i + j + 1].min;
            g_platform[i + j].limit     = g_platform[i + j + 1].limit;
            g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
            g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
            g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
            g_platform[i + j].index     = g_platform[i + j + 1].index;
            for(j = 1;j<= m_Num - i -1;j++)
            {
              g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
              g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
              g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
              g_platform[i + j].width     = g_platform[i + j + 1].width;
              g_platform[i + j].max       = g_platform[i + j + 1].max;
              g_platform[i + j].min       = g_platform[i + j + 1].min;
              g_platform[i + j].limit     = g_platform[i + j + 1].limit;
              g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
              g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
              g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
              g_platform[i + j].index     = g_platform[i + j + 1].index;
            }
            i--;    //判断多个条件用
            g_cur_idx--;
            m_Num--;
         }
         //memcpy(g_Resultplatform+i,g_platform+i,sizeof(TPlatForm));
     }
     //memcpy(g_Resultplatform+i,g_platform+i,sizeof(TPlatForm));
     
     m_Num = g_cur_idx ;  
     m_OldNum = m_Num;  
     //if(m_Num >= 5)
     {
	     for(i = 0;i < m_Num;i++)   //判断第一个 爬坡上有没有一个轴数据
	     {                                 //pDync->m_CmpSpeddPT * 2//
	        if(   g_platform[i].papo_len > pDync->m_CmpSpeddPT * 1.2 //1.5
	           //&& g_platform[i].papo_avg >  ( g_platform[i - 1].avg + g_platform[i].papo_avg * 0.05 * g_AxisK )
	           //&& g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * g_AxisK )
	           && g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * pDync->m_LimitK )
	           )
	         {
	         	if(m_Num < MAX_PLATFORM - 2 )
	         	{
	         	
	         		format_SerDebug("add %d data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",g_cur_idx,i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);
	            
		            g_platform[i].index = 0;
		            for(j = 0;j< m_Num - i;j++)
		            {
		                g_platform[m_Num - j + 1].firstMin    = g_platform[m_Num - j ].firstMin;
		                g_platform[m_Num - j + 1].avg         = g_platform[m_Num - j ].avg;
		                g_platform[m_Num - j + 1].firstMax    = g_platform[m_Num - j ].firstMax;
		                g_platform[m_Num - j + 1].width      += g_platform[m_Num - j ].width;
		                g_platform[m_Num - j + 1].max         = g_platform[m_Num - j ].max;
		                g_platform[m_Num - j + 1].min         = g_platform[m_Num - j ].min;
		                g_platform[m_Num - j + 1].index       = g_platform[m_Num - j ].index;
		                g_platform[m_Num - j + 1].limit       = g_platform[m_Num - j ].limit;
		                g_platform[m_Num - j + 1].limit_cnt   = g_platform[m_Num - j ].limit_cnt;
		                g_platform[m_Num - j + 1].papo_len    = g_platform[m_Num - j ].papo_len;
		                g_platform[m_Num - j + 1].papo_avg    = g_platform[m_Num - j ].papo_avg;
		            }
		            g_platform[m_Num - j + 1].firstMin    = g_platform[i].firstMin;
		            g_platform[m_Num - j + 1].avg         = g_platform[i].papo_avg;
		            g_platform[m_Num - j + 1].firstMax    = g_platform[i].firstMax;
		            g_platform[m_Num - j + 1].width       = g_platform[i].width;
		            g_platform[m_Num - j + 1].max         = g_platform[i].max;
		            g_platform[m_Num - j + 1].index	  = 0;
		            g_platform[m_Num - j + 1].limit       = g_platform[i].limit;
		            g_platform[m_Num - j + 1].limit_cnt   = g_platform[i].limit_cnt;
		            g_platform[m_Num - j + 1].papo_len    = pDync->m_CmpSpeddPT -20 ;//g_platform[i].papo_len;
		            g_platform[m_Num - j + 1].papo_avg    = g_platform[i].papo_avg ;
		            g_cur_idx++;
		            m_Num = g_cur_idx ;
	            }
	         }
		}
     }
     output_result();
}

void hspd_Weight_PostAnalysisAddAxis(void)
{
     int i = 0,j=0;
     int m_Num = g_cur_idx;
     int m_OldNum = m_Num;

    for(i = m_Num; i >= 0;i--)   //计算轴重
    {         //g_platform[i].avg = g_platform[i ].avg - g_platform[i - 1].avg ;
        g_platform[i].width     = g_platform[i].width + g_platform[i].papo_len;
        format_SerDebug("Find width=%d ,avg=%d,min=%d,max=%d papo_len=%d\r\n",g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max,g_platform[i].papo_len);
    }
	format_SerDebug("###############pDync->m_CmpSpeddPT=%d####%d ##########\r\n",pDync->m_CmpSpeddPT,pDync->m_CmpSpeddPT);
     for(i = 1;i < m_Num ;i++)
     {
         if(  g_platform[i].width < pDync->m_CmpSpeddPT
           || g_platform[i].avg <  ( g_platform[i - 1].avg + (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK)
           || g_platform[i].avg >  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK ) // * 0.2
           || g_platform[i].min < g_platform[i - 1].max
           )
         {
         	format_SerDebug("del data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);
            j = 0;
            
            if( g_platform[i].width < pDync->m_CmpSpeddPT
             && g_platform[i].avg <  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK ) // * 0.2
           )
            {
               g_platform[i + j - 1].papo_len  += ( g_platform[i + j ].width );
               //g_platform[i + j].width  = g_platform[i + j + 1].width;
            }
            else
            {
                 //g_platform[i + j ].width += g_platform[i + j ].papo_len ;
                 //g_platform[i + j].width  += g_platform[i + j + 1].width;
            }
                
            g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
            g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
            g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
            g_platform[i + j].width     += g_platform[i + j + 1].width;
            g_platform[i + j].max       = g_platform[i + j + 1].max;
            g_platform[i + j].min       = g_platform[i + j + 1].min;
            g_platform[i + j].limit     = g_platform[i + j + 1].limit;
            g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
            g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
            g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
            g_platform[i + j].index     = g_platform[i + j + 1].index;
            for(j = 1;j<= m_Num - i -1;j++)
            {
              g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
              g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
              g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
              g_platform[i + j].width     = g_platform[i + j + 1].width;
              g_platform[i + j].max       = g_platform[i + j + 1].max;
              g_platform[i + j].min       = g_platform[i + j + 1].min;
              g_platform[i + j].limit     = g_platform[i + j + 1].limit;
              g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
              g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
              g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
              g_platform[i + j].index     = g_platform[i + j + 1].index;
            }
            i--;    //判断多个条件用
            g_cur_idx--;
            m_Num--;
         }
         //memcpy(g_Resultplatform+i,g_platform+i,sizeof(TPlatForm));
     }
     //memcpy(g_Resultplatform+i,g_platform+i,sizeof(TPlatForm));
     
     m_Num = g_cur_idx ;  
     m_OldNum = m_Num;  
     if(m_Num >= 5)
     {
	     for(i = 0;i < m_Num;i++)   //判断第一个 爬坡上有没有一个轴数据
	     {                                 //pDync->m_CmpSpeddPT * 2//
	        if(   g_platform[i].papo_len > pDync->m_CmpSpeddPT * 1.2 //1.5
	           //&& g_platform[i].papo_avg >  ( g_platform[i - 1].avg + g_platform[i].papo_avg * 0.05 * g_AxisK )
	           //&& g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * g_AxisK )
	           && g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * pDync->m_LimitK )
	           )
	         {
	         	if(m_Num < MAX_PLATFORM - 2 )
	         	{
	         	
	         		format_SerDebug("add %d data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",g_cur_idx,i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);
	            
		            g_platform[i].index = 0;
		            for(j = 0;j< m_Num - i;j++)
		            {
		                g_platform[m_Num - j + 1].firstMin    = g_platform[m_Num - j ].firstMin;
		                g_platform[m_Num - j + 1].avg         = g_platform[m_Num - j ].avg;
		                g_platform[m_Num - j + 1].firstMax    = g_platform[m_Num - j ].firstMax;
		                g_platform[m_Num - j + 1].width      += g_platform[m_Num - j ].width;
		                g_platform[m_Num - j + 1].max         = g_platform[m_Num - j ].max;
		                g_platform[m_Num - j + 1].min         = g_platform[m_Num - j ].min;
		                g_platform[m_Num - j + 1].index       = g_platform[m_Num - j ].index;
		                g_platform[m_Num - j + 1].limit       = g_platform[m_Num - j ].limit;
		                g_platform[m_Num - j + 1].limit_cnt   = g_platform[m_Num - j ].limit_cnt;
		                g_platform[m_Num - j + 1].papo_len    = g_platform[m_Num - j ].papo_len;
		                g_platform[m_Num - j + 1].papo_avg    = g_platform[m_Num - j ].papo_avg;
		            }
		            g_platform[m_Num - j + 1].firstMin    = g_platform[i].firstMin;
		            g_platform[m_Num - j + 1].avg         = g_platform[i].papo_avg;
		            g_platform[m_Num - j + 1].firstMax    = g_platform[i].firstMax;
		            g_platform[m_Num - j + 1].width       = g_platform[i].width;
		            g_platform[m_Num - j + 1].max         = g_platform[i].max;
		            g_platform[m_Num - j + 1].index	  = 0;
		            g_platform[m_Num - j + 1].limit       = g_platform[i].limit;
		            g_platform[m_Num - j + 1].limit_cnt   = g_platform[i].limit_cnt;
		            g_platform[m_Num - j + 1].papo_len    = pDync->m_CmpSpeddPT -20 ;//g_platform[i].papo_len;
		            g_platform[m_Num - j + 1].papo_avg    = g_platform[i].papo_avg ;
		            g_cur_idx++;
		            m_Num = g_cur_idx ;
	            }
	         }
		}
     }
     output_result();
}

void hspd_Weight_PostAnalysis(void)
{
	int i = 0,j=0;
	int m_Num = g_cur_idx;
     
	format_SerDebug("save car cnt=%d\r\n",m_Num );
	
    for(i = m_Num; i >= 0;i--)   //计算轴重
    {         //g_platform[i].avg = g_platform[i ].avg - g_platform[i - 1].avg ;
        g_platform[i].width     = g_platform[i].width + g_platform[i].papo_len;
        format_SerDebug("Find width=%d ,avg=%d,min=%d,max=%d papo_len=%d\r\n",g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max,g_platform[i].papo_len);
    }
    format_SerDebug("###############pDync->m_CmpSpeddPT=%d##############\r\n",pDync->m_CmpSpeddPT);

     for(i = 1;i < m_Num ;i++)
     {
         if(  g_platform[i].width < pDync->m_CmpSpeddPT
           || g_platform[i].avg <  ( g_platform[i - 1].avg + (g_platform[i].avg - g_platform[i -1].avg)* 0.05 * pDync->m_LimitK)
           || g_platform[i].avg >  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK )
           || g_platform[i].min < g_platform[i - 1].max
           )
         {
         	format_SerDebug("del data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);

            //g_platform[i].index = 1;
            j = 0;
            g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
            g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
            g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
            g_platform[i + j].width     += g_platform[i + j + 1].width;
            g_platform[i + j].max       = g_platform[i + j + 1].max;
            g_platform[i + j].min       = g_platform[i + j + 1].min;
            g_platform[i + j].limit     = g_platform[i + j + 1].limit;
            g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
            g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
            g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
            g_platform[i + j].index     = g_platform[i + j + 1].index;
            for(j = 1;j<= m_Num - i -1;j++)
            {
              g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
              g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
              g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
              g_platform[i + j].width     = g_platform[i + j + 1].width;
              g_platform[i + j].max       = g_platform[i + j + 1].max;
              g_platform[i + j].min       = g_platform[i + j + 1].min;
              g_platform[i + j].limit     = g_platform[i + j + 1].limit;
              g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
              g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
              g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
              g_platform[i + j].index     = g_platform[i + j + 1].index;
            }
            i--;    //判断多个条件用
            g_cur_idx--;
            m_Num--;
            
         }
     }
     m_Num = g_cur_idx;// - 1 ;
     for(i = 0;i < 1;i++)   //判断爬坡上有没有一个轴数据
     {
        if(   g_platform[i].papo_len > pDync->m_CmpSpeddPT
           //&& g_platform[i].papo_avg >  ( g_platform[i - 1].avg + g_platform[i].papo_avg * 0.05 * g_AxisK )
           && g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * pDync->m_LimitK )
           )
         {
            g_platform[i].index = 0;
            
            for(j = 0;j< m_Num - i;j++)
            {
                g_platform[m_Num - j + 1].firstMin    = g_platform[m_Num - j ].firstMin;
                g_platform[m_Num - j + 1].avg         = g_platform[m_Num - j ].avg;
                g_platform[m_Num - j + 1].firstMax    = g_platform[m_Num - j ].firstMax;
                g_platform[m_Num - j + 1].width      += g_platform[m_Num - j ].width;
                g_platform[m_Num - j + 1].max         = g_platform[m_Num - j ].max;
                g_platform[m_Num - j + 1].min         = g_platform[m_Num - j ].min;
                g_platform[m_Num - j + 1].index         = g_platform[m_Num - j ].index;
                g_platform[m_Num - j + 1].limit       = g_platform[m_Num - j ].limit;
                g_platform[m_Num - j + 1].limit_cnt   = g_platform[m_Num - j ].limit_cnt;
                g_platform[m_Num - j + 1].papo_len   += g_platform[m_Num - j ].papo_len;
                g_platform[m_Num - j + 1].papo_avg    = g_platform[m_Num - j ].papo_avg;
            }
            g_platform[m_Num - j + 1].firstMin    = g_platform[i].firstMin;
            g_platform[m_Num - j + 1].avg         = g_platform[i].papo_avg;
            g_platform[m_Num - j + 1].firstMax    = g_platform[i].firstMax;
            g_platform[m_Num - j + 1].width       = g_platform[i].width;
            g_platform[m_Num - j + 1].max         = g_platform[i].max;
            g_platform[m_Num - j + 1].limit       = g_platform[i].limit;
            g_platform[m_Num - j + 1].limit_cnt   = g_platform[i].limit_cnt;
            g_platform[m_Num - j + 1].index		  = 0;
            g_platform[m_Num - j + 1].papo_len    = g_platform[i].papo_len;
            g_platform[m_Num - j + 1].papo_avg    = g_platform[i].papo_avg ;
            g_cur_idx++;
            
         }
     }        
     output_result();
}
void hspd_Weight_PostAnalysisNoAddAxis()
{
	int i = 0,j=0;
	int m_Num = g_cur_idx;
     
	format_SerDebug("save car cnt=%d\r\n",m_Num );
	
    for(i = m_Num; i >= 0;i--)   //计算轴重
    {         //g_platform[i].avg = g_platform[i ].avg - g_platform[i - 1].avg ;
        g_platform[i].width     = g_platform[i].width + g_platform[i].papo_len;
        format_SerDebug("Find width=%d ,avg=%d,min=%d,max=%d papo_len=%d\r\n",g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max,g_platform[i].papo_len);
    }
    format_SerDebug("###############pDync->m_CmpSpeddPT=%d##############\r\n",pDync->m_CmpSpeddPT);

     for(i = 1;i < m_Num ;i++)
     {
         if(  g_platform[i].width < pDync->m_CmpSpeddPT
           || g_platform[i].avg <  ( g_platform[i - 1].avg + (g_platform[i].avg - g_platform[i -1].avg)* 0.05 * pDync->m_LimitK)
           || g_platform[i].avg >  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK )
           || g_platform[i].min < g_platform[i - 1].max
           )
         {
         	format_SerDebug("del data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);

            //g_platform[i].index = 1;
            j = 0;
            g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
            g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
            g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
            g_platform[i + j].width     += g_platform[i + j + 1].width;
            g_platform[i + j].max       = g_platform[i + j + 1].max;
            g_platform[i + j].min       = g_platform[i + j + 1].min;
            g_platform[i + j].limit     = g_platform[i + j + 1].limit;
            g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
            g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
            g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
            g_platform[i + j].index     = g_platform[i + j + 1].index;
            for(j = 1;j<= m_Num - i -1;j++)
            {
              g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
              g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
              g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
              g_platform[i + j].width     = g_platform[i + j + 1].width;
              g_platform[i + j].max       = g_platform[i + j + 1].max;
              g_platform[i + j].min       = g_platform[i + j + 1].min;
              g_platform[i + j].limit     = g_platform[i + j + 1].limit;
              g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
              g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
              g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
              g_platform[i + j].index     = g_platform[i + j + 1].index;
            }
            i--;    //判断多个条件用
            g_cur_idx--;
            m_Num--;
            
         }
     }
     #if 0
     m_Num = g_cur_idx;// - 1 ;
     for(i = 0;i < 1;i++)   //判断爬坡上有没有一个轴数据
     {
        if(   g_platform[i].papo_len > pDync->m_CmpSpeddPT
           //&& g_platform[i].papo_avg >  ( g_platform[i - 1].avg + g_platform[i].papo_avg * 0.05 * g_AxisK )
           && g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * pDync->m_LimitK )
           )
         {
            g_platform[i].index = 0;
            
            for(j = 0;j< m_Num - i;j++)
            {
                g_platform[m_Num - j + 1].firstMin    = g_platform[m_Num - j ].firstMin;
                g_platform[m_Num - j + 1].avg         = g_platform[m_Num - j ].avg;
                g_platform[m_Num - j + 1].firstMax    = g_platform[m_Num - j ].firstMax;
                g_platform[m_Num - j + 1].width      += g_platform[m_Num - j ].width;
                g_platform[m_Num - j + 1].max         = g_platform[m_Num - j ].max;
                g_platform[m_Num - j + 1].min         = g_platform[m_Num - j ].min;
                g_platform[m_Num - j + 1].index         = g_platform[m_Num - j ].index;
                g_platform[m_Num - j + 1].limit       = g_platform[m_Num - j ].limit;
                g_platform[m_Num - j + 1].limit_cnt   = g_platform[m_Num - j ].limit_cnt;
                g_platform[m_Num - j + 1].papo_len   += g_platform[m_Num - j ].papo_len;
                g_platform[m_Num - j + 1].papo_avg    = g_platform[m_Num - j ].papo_avg;
            }
            g_platform[m_Num - j + 1].firstMin    = g_platform[i].firstMin;
            g_platform[m_Num - j + 1].avg         = g_platform[i].papo_avg;
            g_platform[m_Num - j + 1].firstMax    = g_platform[i].firstMax;
            g_platform[m_Num - j + 1].width       = g_platform[i].width;
            g_platform[m_Num - j + 1].max         = g_platform[i].max;
            g_platform[m_Num - j + 1].limit       = g_platform[i].limit;
            g_platform[m_Num - j + 1].limit_cnt   = g_platform[i].limit_cnt;
            g_platform[m_Num - j + 1].index		  = 0;
            g_platform[m_Num - j + 1].papo_len    = g_platform[i].papo_len;
            g_platform[m_Num - j + 1].papo_avg    = g_platform[i].papo_avg ;
            g_cur_idx++;
            
         }
     }  
     #endif     
     output_result();
}
#if 0
void hspd_Weight_PostAnalysis(void)
{
	int i = 0,j=0;
	int m_Num = g_cur_idx;
     
	format_SerDebug("save car cnt=%d\r\n",m_Num );
	
    for(i = m_Num; i >= 0;i--)   //计算轴重
    {         //g_platform[i].avg = g_platform[i ].avg - g_platform[i - 1].avg ;
        g_platform[i].width     = g_platform[i].width + g_platform[i].papo_len;
        format_SerDebug("Find width=%d ,avg=%d,min=%d,max=%d papo_len=%d\r\n",g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max,g_platform[i].papo_len);
    }
    format_SerDebug("###############pDync->m_CmpSpeddPT=%d##############\r\n",pDync->m_CmpSpeddPT);

     for(i = 1;i < m_Num ;i++)
     {
         if(  g_platform[i].width < pDync->m_CmpSpeddPT
           || g_platform[i].avg <  ( g_platform[i - 1].avg + (g_platform[i].avg - g_platform[i -1].avg)* 0.05 * pDync->m_LimitK)
           || g_platform[i].avg >  ( g_platform[i + 1].avg - (g_platform[i].avg - g_platform[i -1].avg) * 0.05 * pDync->m_LimitK )
           || g_platform[i].min < g_platform[i - 1].max
           )
         {
         	format_SerDebug("del data i=%d width=%d ,avg=%d,min=%d,max=%d \r\n",i,g_platform[i].width,g_platform[i].avg,g_platform[i].min,g_platform[i].max);

            //g_platform[i].index = 1;
            j = 0;
            g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
            g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
            g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
            g_platform[i + j].width     += g_platform[i + j + 1].width;
            g_platform[i + j].max       = g_platform[i + j + 1].max;
            g_platform[i + j].min       = g_platform[i + j + 1].min;
            g_platform[i + j].limit     = g_platform[i + j + 1].limit;
            g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
            g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
            g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
            g_platform[i + j].index     = g_platform[i + j + 1].index;
            for(j = 1;j<= m_Num - i -1;j++)
            {
              g_platform[i + j].firstMin  = g_platform[i + j + 1].firstMin;
              g_platform[i + j].avg       = g_platform[i + j + 1].avg;//(g_platform[i + j + 1].avg +  g_platform[i + j].avg )/2;
              g_platform[i + j].firstMax  = g_platform[i + j + 1].firstMax;
              g_platform[i + j].width     = g_platform[i + j + 1].width;
              g_platform[i + j].max       = g_platform[i + j + 1].max;
              g_platform[i + j].min       = g_platform[i + j + 1].min;
              g_platform[i + j].limit     = g_platform[i + j + 1].limit;
              g_platform[i + j].limit_cnt = g_platform[i + j + 1].limit_cnt;
              g_platform[i + j].papo_len  = g_platform[i + j + 1].papo_len;
              g_platform[i + j].papo_avg  = g_platform[i + j + 1].papo_avg;//(g_platform[i + j + 1].papo_avg + g_platform[i + j].papo_avg) /2;
              g_platform[i + j].index     = g_platform[i + j + 1].index;
            }
            i--;    //判断多个条件用
            g_cur_idx--;
            m_Num--;
            
         }
     }
     m_Num = g_cur_idx;// - 1 ;
     for(i = 0;i < 1;i++)   //判断爬坡上有没有一个轴数据
     {
        if(   g_platform[i].papo_len > pDync->m_CmpSpeddPT
           //&& g_platform[i].papo_avg >  ( g_platform[i - 1].avg + g_platform[i].papo_avg * 0.05 * g_AxisK )
           && g_platform[i].papo_avg <  ( g_platform[i + 1].avg - g_platform[i].papo_avg * 0.05 * pDync->m_LimitK )
           )
         {
            g_platform[i].index = 0;
            
            for(j = 0;j< m_Num - i;j++)
            {
                g_platform[m_Num - j + 1].firstMin    = g_platform[m_Num - j ].firstMin;
                g_platform[m_Num - j + 1].avg         = g_platform[m_Num - j ].avg;
                g_platform[m_Num - j + 1].firstMax    = g_platform[m_Num - j ].firstMax;
                g_platform[m_Num - j + 1].width      += g_platform[m_Num - j ].width;
                g_platform[m_Num - j + 1].max         = g_platform[m_Num - j ].max;
                g_platform[m_Num - j + 1].min         = g_platform[m_Num - j ].min;
                g_platform[m_Num - j + 1].index         = g_platform[m_Num - j ].index;
                g_platform[m_Num - j + 1].limit       = g_platform[m_Num - j ].limit;
                g_platform[m_Num - j + 1].limit_cnt   = g_platform[m_Num - j ].limit_cnt;
                g_platform[m_Num - j + 1].papo_len   += g_platform[m_Num - j ].papo_len;
                g_platform[m_Num - j + 1].papo_avg    = g_platform[m_Num - j ].papo_avg;
            }
            g_platform[m_Num - j + 1].firstMin    = g_platform[i].firstMin;
            g_platform[m_Num - j + 1].avg         = g_platform[i].papo_avg;
            g_platform[m_Num - j + 1].firstMax    = g_platform[i].firstMax;
            g_platform[m_Num - j + 1].width       = g_platform[i].width;
            g_platform[m_Num - j + 1].max         = g_platform[i].max;
            g_platform[m_Num - j + 1].limit       = g_platform[i].limit;
            g_platform[m_Num - j + 1].limit_cnt   = g_platform[i].limit_cnt;
            g_platform[m_Num - j + 1].index		  = 0;
            g_platform[m_Num - j + 1].papo_len    = g_platform[i].papo_len;
            g_platform[m_Num - j + 1].papo_avg    = g_platform[i].papo_avg ;
            g_cur_idx++;
            
         }
     }
             
     output_result();

}
#endif
void hspd_Weight_Service(int kg)
{

	if (kg < g_up_limit)
	{
		spd_Weight_Init();
		g_bfKg = kg;
		return;
	}

	if (!g_car_arrival)
	{
		Weight_Maybe_New_Platform(kg);
		g_car_arrival = 1;
	}
	f_lookfor_max_min_pt(kg);

	if (g_first_flag || g_new_platform_flag || g_papo_flag)
	{
		f_platform_process(kg);
	}
	if (g_up_flag) // 下降过程
	{
		if (g_first_flag) //平台第一次下降
		{
			
		}
	}else if(g_dn_flag){ //上升过程
		
	}
	g_bfKg = kg;
}

//////////////////////////////////////////////
void  Weight_Analysis_Speed(int inAD1,int inAD2,int wet)
{
       int tempa = 0;
       int tempb = 0;
       static int cnt = 0;
       static int  flag = 0;

       if(wet < 500)
       {
           cnt = 0;
           flag = 0;
            pDync->m_CmpSpeddPT = 0;
           return;
       }
       if(flag )
          return;

       tempa = inAD1; 
       tempb = inAD2;
	   
       if((tempb < 500) && (tempa < 500 ))
       {
           cnt = 0;
           flag = 0;
       }
       if((tempb > 500) || (tempa > 500 ))
       {
           cnt++;
       }
       if((tempb > 500) && (tempa > 500 ))  //都上了
       {
       	pDync->CarSpeed = 20.0 * 7 / cnt; //200.0 认为是7m称台
       	
		pDync->m_CmpSpeddPT = cnt / ( 7 +   0.3 * pDync->m_SumLen);
		if(pDync->m_CmpSpeddPT > 150)
		    pDync->m_CmpSpeddPT = 150;
		flag = 1;
      }
}

/*
	轴结果输出
*/
void output_result(void)
{
	pDync->nAlexNum	=	g_cur_idx;
	
	if(pDync->pDecb != NULL)
		pDync->pDecb(pDync);
	
	spd_Weight_Init();
}
int  FindStableWet(int wet)
{
	static int bfWet = 0;
	static int bCnt	= 0;

	if(fabs(wet - bfWet) < 900)
	{
		bCnt++;
		if(bCnt > 800)
		{
			bCnt = 0;
			return 1;
		}
	}
	else
	{
		bCnt = 0;
		bfWet = wet;
	}
	return 0;
	
}

void   SoftAlexDyncProcResult(TAlexArr* alexarr)
{
	int i = 0;
	if(g_cur_idx >= MAX_ALEX_NUM)
		g_cur_idx = 7;
	
	for(i = 0;i< g_cur_idx;i++)
	{
		alexarr->width[i]    = g_platform[i].width;
		alexarr->avg[i]    	= g_platform[i].avg;
	}
	alexarr->num	= g_cur_idx;
	alexarr->fSpeed  = pDync->CarSpeed;

	spd_Weight_Init();
}

void  SoftAlexDyncProcStop(void)
{
	if(bOut == 0)
		hspd_CarOver();
	bOut = 1;
}

/*
轴信号处理模块初始化
*/
/*
 * 功能:    动态处理
 */


char  SoftAlexDyncProcInit(void* pDecb)
{
	pDync = (sSoftAlexDyncProc *)pDecb;
	#if 1	
	if (pDecb == 0) return (char)-1;

	pDync->fWetUp = 0;
	pDync->fWetDown = 0;
	pDync->iPowerOnIndex = 0;
	pDync->nAlexINorRemove = 0;
	pDync->nAlexWet = 0;
	pDync->nAlexPulseWith = 0;
	pDync->nAlexMaxWet = 0;
	pDync->m_CmpSpeddPT = 0;
	
	g_rate = pDync->m_rate;


	//
	//pDync->pDync->m_CmpSpeddPT = 0;
	//pDync->pDync->m_LimitK = 0;
	//pDync->mFixAlexMode = 0;
	

	spd_Weight_Init();
	#endif
	return 0;
}

/*
分析轴的方向，进轴还是倒轴
axisAd : 轴传感器AD
bigScalerKg: 大称实时重量
*/
float SoftAlexDyncProc(void* pDecb, float * pInArr, int nInCount)
{	
	float mScaleWet = pInArr[0] +pInArr[2];
	float mAlex1Wet 	= pInArr[0];
	float mAlex2Wet 	= pInArr[2];
	
	#if 1
	hspd_Weight_Service(mScaleWet);
	Weight_Analysis_Speed(mAlex1Wet,mAlex2Wet,mScaleWet);

	if((bOut == 0) && (FindStableWet(mScaleWet) != 0))
	{
		bOut = 1;
		hspd_CarOver();
	}
	if(mScaleWet < 500)
	{
		bOut = 0;
	}
	#endif
	return mScaleWet;
}

