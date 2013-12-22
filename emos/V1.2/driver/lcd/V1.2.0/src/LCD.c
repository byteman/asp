
#include "LCD.h"
#include "LCD_cfg.h"
#include <stdbool.h>


#define LCD_BYTES_X  			30 			//显示区宽度
#define TEXT_HOME_ADDRESS 		0x0000 		//文本显示区首地址
#define GRAPHIC_HOME_ADDRESS 	0x01E0 		//图形显示区首地址

#define	GPIO_SET(x)		{LCD_##x##_PORT -> BSRRL = LCD_##x##_PIN;}
#define	GPIO_RESET(x)	{LCD_##x##_PORT -> BSRRH = LCD_##x##_PIN;}

#define	LCD_WRITE_DATA(data)	LCD_DATA1_PORT -> BSRRH = 0x00FF;LCD_DATA1_PORT -> BSRRL = (data) ;		//写数据
#define	LCD_SET_DATA_OUT()		LCD_DATA1_PORT->MODER |= 0x00005555; 									//数据线设置为输出模式
#define	LCD_SET_DATA_IN()	    LCD_DATA1_PORT->MODER &= 0xffff0000; 									//数据线设置为输入模式

#define SdCmd	LCD_WriteCommand
#define SdData	LCD_WriteData

#define	RdData

// 坐标X为RAM地址坐标，X地址包含为3个像素
// 坐标Y为像素点行数
// *pstr为中文字符代码数组，对应字库为CCTAB
const unsigned char abcde[32] = {
0x00,0x04,0x7F,0xFE,0x40,0x24,0x5F,0xF4,
      0x41,0x04,0x41,0x04,0x41,0x44,0x4F,0xE4,
      0x41,0x04,0x41,0x44,0x41,0x24,0x41,0x04,
      0x5F,0xF4,0x40,0x04,0x7F,0xFC,0x40,0x04	
};
/****************************************************************************
* 名	称：u8 fLCD_ReadLEDstate(void)
* 功	能：读背光状态
* 入口参数：
* 出口参数：u8	1:背光已打开；	0：背光已关闭
* 说	明：无
****************************************************************************/
u8 fLCD_ReadLEDstate(void)
{
	return GPIO_ReadOutputDataBit(LCD_LED_PORT , LCD_LED_PIN);
}
/****************************************************************************
* 名	称：void fLCD_LEDON(void)
* 功	能：打开LCD背光
* 入口参数：
* 出口参数：
* 说	明：无
****************************************************************************/
void fLCD_LEDON(void)
{
	GPIO_SET(LED);
}
/****************************************************************************
* 名	称：void fLCD_LEDOFF(void)
* 功	能：关闭LCD背光
* 入口参数：
* 出口参数：
* 说	明：无
****************************************************************************/
void fLCD_LEDOFF(void)
{
	GPIO_RESET(LED);
}


static void WhileDly(u32 nDly)
{
	while(nDly--);
}

static void LCD_PortInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);		//开启引脚时钟G
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);		//开启引脚时钟G

 	//配置读写控制为推挽输出
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  
	GPIO_InitStructure.GPIO_Pin   = LCD_WR_PIN|LCD_RD_PIN|LCD_CE_PIN|LCD_CD_PIN|LCD_RST_PIN|LCD_FS_PIN|LCD_LED_PIN/
									LCD_DATA1_PIN|LCD_DATA2_PIN|LCD_DATA3_PIN|LCD_DATA4_PIN|LCD_DATA5_PIN|LCD_DATA6_PIN|LCD_DATA7_PIN|LCD_DATA8_PIN;		//LCD_WR
	GPIO_Init(LCD_WR_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin   = LCD_AD_CTRL_PIN ;					
	GPIO_Init(LCD_AD_CTRL_PORT, &GPIO_InitStructure);	
					
	GPIO_RESET(RST);
	
	GPIO_SET(CE);
	GPIO_SET(WR);
	GPIO_SET(AD_CTRL);
	GPIO_SET(RD);
	GPIO_SET(CD);
	GPIO_RESET(FS);		//使用8*8模式
	GPIO_SET(LED);		//打开背光
}

/****************************************************************************
* 名	称：	void LCD_WriteCommand(uint8 nCmd)
* 功	能：	写LCD命令
* 入口参数：u8	命令
* 出口参数：无
* 说	明：	无
****************************************************************************/
void LCD_WriteCommand(unsigned char nCmd)
{
	LCD_SET_DATA_OUT()				/* 将数据输出口设为输出方式 */
	GPIO_RESET(CE);
	GPIO_RESET(CD);
	GPIO_RESET(WR);
	LCD_WRITE_DATA(nCmd);			//向MCU写入数据，准备向LCD输出数据
	WhileDly(1);
	GPIO_SET(WR);
	GPIO_SET(CE);
	LCD_SET_DATA_IN();				/* 将数据输出口设为输入方式 */
}

void LCD_WriteData(unsigned char nData)
{
	LCD_SET_DATA_OUT();				/* 将数据输出口设为输出方式 */
	GPIO_RESET(CE);					//GPIO_RESET(CE);                               
	GPIO_SET(CD);                   //GPIO_RESET(CD);                   
	GPIO_RESET(WR);                 //GPIO_RESET(WR);                        
	LCD_WRITE_DATA(nData);		    //LCD_WRITE_DATA(nCmd);	
	WhileDly(1);         			//WhileDly(NS100_DLY(4));                          
	GPIO_SET(WR);                   //GPIO_SET(WR);
	GPIO_SET(CE);
	LCD_SET_DATA_IN();				/* 将数据输出口设为输入方式 */
}


unsigned char Rd_Data(void)  
{
	unsigned char Ddata;
	
	LCD_SET_DATA_IN();
	GPIO_SET(CD); 
	GPIO_RESET(CE);
	GPIO_RESET(RD);
	Ddata = GPIO_ReadInputData(GPIOG);
	GPIO_SET(RD);
	GPIO_SET(CE);
	
    return (Ddata);
}

//-----绘点函数--------------------------
void Draw_Dot(int x, int y)
// 坐标(x,y)，x为水平方向像素点列；y为垂直方向像素点行
{
	int k1,k2,m;
	
   m=x/3;
   SdCmd(0xf4); SdCmd(m);                        // 设置操作窗口左边界
   SdCmd(0xf5); SdCmd(y);                        // 设置操作窗口上边界
   SdCmd(0xf6); SdCmd(m);                        // 设置操作窗口右边界
   SdCmd(0xf7); SdCmd(y);                        // 设置操作窗口下边界
   SdCmd(0xf8);			  	         			 // 设置窗口操作使能
   SdCmd(0xd6);					 				 // 设置数据位为16位
   SdCmd(m&0x0f); SdCmd(0x10|(m>>4));            // 设置起始列地址
   SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));     // 设置起始行地址
   k1 = Rd_Data();                                  // 空读
   k1 = Rd_Data();                                  // 读数据
   k2 = Rd_Data();                                  // 读数据
	
   m=x%3;
   switch (m) {
       case 0:   SdData(k1|0xf8);SdData(k2);		break;   // 写点
       case 1:   SdData(k1|0x07);SdData(k2|0xe0);	break;
       case 2:   SdData(k1);SdData(k2|0x1f);		break;
      }
   SdCmd(0xd5);					  // 恢复设置数据位为12位
}

//-----绘点函数--------------------------
void Draw_ClrDot(int x, int y)
// 坐标(x,y)，x为水平方向像素点列；y为垂直方向像素点行
{
	int k1,k2,m;
	
   m=x/3;
   SdCmd(0xf4); SdCmd(m);                        // 设置操作窗口左边界
   SdCmd(0xf5); SdCmd(y);                        // 设置操作窗口上边界
   SdCmd(0xf6); SdCmd(m);                        // 设置操作窗口右边界
   SdCmd(0xf7); SdCmd(y);                        // 设置操作窗口下边界
   SdCmd(0xf8);			  	         			 // 设置窗口操作使能
   SdCmd(0xd6);					 				 // 设置数据位为16位
   SdCmd(m&0x0f); SdCmd(0x10|(m>>4));            // 设置起始列地址
   SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));     // 设置起始行地址
   k1 = Rd_Data();                                  // 空读
   k1 = Rd_Data();                                  // 读数据
   k2 = Rd_Data();                                  // 读数据
	
   m=x%3;
   switch (m) {
       case 0:   SdData(k1&0x07);SdData(k2);		break;   // 写点
       case 1:   SdData(k1&0xF8);SdData(k2&0x1F);	break;
       case 2:   SdData(k1);SdData(k2&0xe0);		break;
      }
   SdCmd(0xd5);					  // 恢复设置数据位为12位
}
void LCD_DrawLine(int x1,int y1,int x2,int y2)
//  x为水平方向像素列；y为垂直方向像素点行
//  坐标(x1,y1)为线起始地址坐标；坐标(x2,y2)为线终止地址坐标。
{
  u32 temp;
  int dalt_x,dalt_y,err=0;
	
 if (y1>y2)
    {
    temp=x1;
    x1=x2;
    x2=temp;
    temp=y1;
    y1=y2;
    y2=temp;
    }
 Draw_Dot(x1,y1);
 dalt_x=x2-x1;
 dalt_y=y2-y1;
 if(dalt_x>=0)
     {
      if(dalt_y>dalt_x)//k>1
        {
         while(y1<y2)
         {
          if(err<0)
          {
           x1=x1+1;
           y1=y1+1;
           err=err+dalt_y-dalt_x;
           }
           else
           {
           y1=y1+1;
           err=err-dalt_x;
           }
          Draw_Dot(x1,y1);
         }
       }
     else  // 0<=k=<1
      {
       if (dalt_y==0)
          y1=y1-1;
       while(x1<x2)
         {
          if(err<0)
          {
           x1=x1+1;
           err=err+dalt_y;
           }
           else
           {
           y1=y1+1;
           x1=x1+1;
           err=err+dalt_y-dalt_x;
           }
          Draw_Dot(x1,y1);
         }
     }
     
  }
else
  { 
   dalt_x=x1-x2;  
    if(dalt_y>dalt_x)//k<-1
     {
      while(y1<y2)
       {
        if(err<0)
         {
          x1=x1-1;
          y1=y1+1;
          err=err+dalt_y-dalt_x;
         }
        else
         {
          y1=y1+1;
          err=err-dalt_x;
         }
        Draw_Dot(x1,y1);
      }
    }
   else       //0>k>=-1
    {
      if (dalt_y==0)
          y1=y1-1; 
    while(x1>x2)
      {
       if(err<0)
        {
         x1=x1-1;
         err=err+dalt_y;
        }
       else
        {
         x1=x1-1;
         y1=y1+1;
         err=err+dalt_y-dalt_x;
        }
       Draw_Dot(x1,y1);
      }
    } 
  }
     
}
void LCD_ShowBMP(int x, int y, int width, int high, const unsigned char bmp[])
{  
// 坐标X为像素列块0-79列块，3像素点数据/列块
// 坐标Y为像素点行数
// 图形宽度width为水平方向点列数，要求该值为6的倍数
// 图形高度high为垂直方向点行数
// 图形数组bmp[]为所要写入的图形数据，以1bpp（8dots/byte)、水平排列格式表示
// 写入图形前，要先设置显示前景色和背景色 
	int p; 
	int i,j,k,m;
 
	SdCmd(0xf4); SdCmd(x);                   		// 设置操作窗口左边界
	SdCmd(0xf5); SdCmd(y);                   		// 设置操作窗口上边界 
	SdCmd(0xf6); SdCmd(width/3);             		// 设置操作窗口右边界
	SdCmd(0xf7); SdCmd(high-1);              		// 设置操作窗口下边界
	SdCmd(0xf8);		                    		// 设置窗口操作使能
	SdCmd(x&0x0f);SdCmd(0x10|(x>>4));        		// 设置起始列地址
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));		// 设置起始行地址
	p = 0;                                     		// 数组指针初始化
	for (i=0; i<high; i++) {                      	// 循环行数据量
		for (j=0; j<width/8; j++) {              	// 循环字节数/行
			k = bmp[p++];                 			// 取图形数据
			for (m=0; m<8; m++) {              		// 1字节转换成4个字节数据写入
				if (k & 0x80) {
					Draw_Dot(x+j*8+m, y+i);
				} else {
					Draw_ClrDot(x+j*8+m, y+i);
				}
				k = (k<<1);
			}
		}
	} 
}

void ClearScreen(void)
{
	int x = 0,y = 0;
	int width = 239;
	int high = 127;
	int i,j;
	
	SdCmd(0xf4); SdCmd(x);                   		// 设置操作窗口左边界
	SdCmd(0xf5); SdCmd(y);                   		// 设置操作窗口上边界 
	SdCmd(0xf6); SdCmd(width/3);             		// 设置操作窗口右边界
	SdCmd(0xf7); SdCmd(high);              			// 设置操作窗口下边界
	SdCmd(0xf8);		                    		// 设置窗口操作使能
	SdCmd(x&0x0f);SdCmd(0x10|(x>>4));        		// 设置起始列地址
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));		// 设置起始行地址	
	for (i = 0; i < 128; i++) {
		for (j = 0; j < 120; j++) {
			SdData(0x00);
		}
	}	
}

void LCD_DrawGbChines(int x, int y, const unsigned char *pstr) 
{  
	char Ddata = 1;
	int m,ix,iy;

	SdCmd(0xf4); SdCmd(x);               		// 设置操作窗口左边界
	SdCmd(0xf5); SdCmd(y);           			// 设置操作窗口上边界 
	SdCmd(0xf6); SdCmd(x+16/3);         		// 设置操作窗口右边界
	SdCmd(0xf7); SdCmd(y+14);           		// 设置操作窗口下边界
	SdCmd(0xf8);		               			// 设置窗口操作使能
	SdCmd(x&0x0f);SdCmd(0x10|(x>>4));        	// 设置起始列地址
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));	// 设置起始行地址	

	for (iy = 0; iy < 14; iy++) {
		for (ix = 0; ix < 2; ix++) {
			Ddata = *pstr++;
			for (m = 0; m < 8; m++) {
				if (Ddata&0x80) {
					Draw_Dot(x+ix*8+m, y+iy);
				} else {
					Draw_ClrDot(x+ix*8+m, y+iy);
				}
				Ddata <<= 1; 
			}
			
		}
	}
}
void LCD_DrawGbAsc(int x, int y, const unsigned char *pstr) 
{  
	char Ddata = 1;
	int m,ix,iy;

	SdCmd(0xf4); SdCmd(x);               		// 设置操作窗口左边界
	SdCmd(0xf5); SdCmd(y);           			// 设置操作窗口上边界 
	SdCmd(0xf6); SdCmd(x+8/3);         		// 设置操作窗口右边界
	SdCmd(0xf7); SdCmd(y+14);           		// 设置操作窗口下边界
	SdCmd(0xf8);		               			// 设置窗口操作使能
	SdCmd(x&0x0f);SdCmd(0x10|(x>>4));        	// 设置起始列地址
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));	// 设置起始行地址	

	for (iy = 0; iy < 14; iy++) {
		for (ix = 0; ix < 1; ix++) {
			Ddata = *pstr++;
			for (m = 0; m < 8; m++) {
				if (Ddata&0x80) {
					Draw_Dot(x+ix*8+m, y+iy);
				} else {
					Draw_ClrDot(x+ix*8+m, y+iy);
				}
				Ddata <<= 1; 
			}
			
		}
	}
}
/****************************************************************************
* 名	称：void LCD_Init(void)
* 功	能：初始化LCD
* 入口参数：无
* 出口参数：
* 说	明：无
****************************************************************************/
void LCD_Init(void)
{
	//LCD管脚初始化
	LCD_PortInit();	

    GPIO_RESET(RST); LCD_Delay(NS100_DLY(200)); 	//硬件复位
    GPIO_SET(RST); 	LCD_Delay(NS100_DLY(80000)); 	//复位后延迟800ms以上时间

    LCD_WriteCommand(0x25);                           	// 设置温度补偿系数-0.05%/C
    LCD_WriteCommand(0x2b);                           	// 内部DC-DC
	LCD_WriteCommand(0xc4);                          	// LCD映像MY=1，MX=0，LC0=0  c4
	LCD_WriteCommand(0xa3);                        		// 设置行扫描频率  fr=76.34hz a1
	LCD_WriteCommand(0xd1);                          	// 彩色数据格式R-G-B
	LCD_WriteCommand(0xd5);								// 设置数据位为12位RRRR-GGGG-BBBB
	LCD_WriteCommand(0xc8);	LCD_WriteCommand(0x3f);    	// 设置M信号为19行翻转 11
	LCD_WriteCommand(0xec);                            	// 设置偏压比1/12
	LCD_WriteCommand(0xa6);                            	// 正性显示
	LCD_WriteCommand(0xa4);                            	// 正常显示
	LCD_WriteCommand(0x81);	LCD_WriteCommand(0x4f);   	//设置对比度bf
	LCD_WriteCommand(0xd8);                            	// 设置扫描模式d8

    LCD_WriteCommand(0xf1);	LCD_WriteCommand(0x9f);    	//结束行CEN,159
    LCD_WriteCommand(0xf2);	LCD_WriteCommand(0x20);    	//起始DST
    LCD_WriteCommand(0xf3);	LCD_WriteCommand(0x9f);    	//结束DEN
	LCD_WriteCommand(0x84); 
	LCD_WriteCommand(0xa9); 
	
	LCD_Delay(NS100_DLY(800));// 开显示ad
	ClearScreen();
	
// 	LCD_ShowBMP(0,0,14,14,abcde);
// 	
// 	LCD_DrawGbChines(50,50,abcde);
	
}

