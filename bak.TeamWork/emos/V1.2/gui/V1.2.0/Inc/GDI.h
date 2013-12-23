#ifndef _GDI_H_
#define _GDI_H_

#ifdef __cplusplus		   //�����CPP����C���� //��ʼ����
extern "C" {
#endif
	
#include "lcd_init.h"
#include "stm32f4xx.h"

#ifdef LCD_DEBUG
int LCD_Debug(void);
#endif
	
/****************************************************************************
* ��	�ƣ�void BeginPaint(void)
* ��	�ܣ�GUI��ʼ��������������LCD�ĳ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	����Ҫʹ��LCDǰ������øú���һ��
****************************************************************************/	
void BeginPaint(void);

/****************************************************************************
* ��	�ƣ�void EraseScreen(void)
* ��	�ܣ�������MCU�ϵ��Դ�
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/	
void EraseScreen(void);

/****************************************************************************
* ��	�ƣ�void EraseBuffer(void)
* ��	�ܣ���MCU�Դ�
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/	
void EraseBuffer(void);

/****************************************************************************
* ��	�ƣ�void EraseRect(s16 x, s16 y, s16 cx, s16 cy)
* ��	�ܣ���MCU��ѡ��������Դ�
* ��ڲ�����s16 x			��������x����ʼλ��
						s16 y			��������y����ʼλ��
						s16 cx		���x��ĳ���
						s16 cy		���y��ĳ���
* ���ڲ�������
* ˵	����
****************************************************************************/
void EraseRect(s16 x, s16 y, s16 cx, s16 cy);

/****************************************************************************
* ��	�ƣ�SetRedraw(u8 bEnable)
* ��	�ܣ�����ͼ���ػ棬�Ƿ���������ˢ��
* ��ڲ�����u8 bEnable	LCD�Ƿ�����ˢ��:0 ��ˢ�´�����1	��1	��ˢ�´�����1
* ���ڲ�������
* ˵	����
****************************************************************************/	
void SetRedraw(u8 bEnable);

/****************************************************************************
* ��	�ƣ�void EnableScreenFlush(u8 bEnable)
* ��	�ܣ�ˢ��MCU�Դ濪�غ���
* ��ڲ�����u8 bEnable	
* ���ڲ�������
* ˵	�����������1:FALSE		��ֹˢ��MCU�Դ�
										TRUE		����ˢ��MCU�Դ�
****************************************************************************/	
void EnableScreenFlush(u8 bEnable);

/****************************************************************************
* ��	�ƣ�void EnableGdiView(u8 bEnable)
* ��	�ܣ���ͼ���غ���
* ��ڲ�����u8 bEnable	
* ���ڲ�������
* ˵	�����������1:FALSE		��ֹ��ͼ
										TRUE		������ͼ
****************************************************************************/	
void EnableGdiView(u8 bEnable);

/****************************************************************************
* ��	�ƣ�SetGdiView(s16 x, s16 y, s16 cx, s16 cy)
* ��	�ܣ����û���ͼ�εĴ�С
* ��ڲ�����s16 x		����ʼλ��(0~240)
						s16 y		����ʼλ��(0~128)
						s16 cx 	�л��Ƴ���
						s16 cy	�л��Ƴ���
* ���ڲ�������
* ˵	����
****************************************************************************/	
void SetGdiView(s16 x, s16 y, s16 cx, s16 cy);


/****************************************************************************
* ��	�ƣ�void FlushScreen(void)
* ��	�ܣ�ˢ��������Ļ
* ��ڲ�������
* ���ڲ�������
* ˵	������MCU�Դ������ˢ�µ�LCD���Դ��У����Ҫ��ʾ��Ҫ�ڸú����������Ӻ��� SetRedraw(TRUE);
****************************************************************************/	
void FlushScreen(void);

/****************************************************************************
* ��	�ƣ�void FlushRect(void)
* ��	�ܣ�ˢ��ѡ��������Ļ
* ��ڲ�����s16 x		ѡ�������x����ʼλ��
						s16 y		ѡ�������y����ʼλ��
						s16 cx	x��ĳ���
						s16 cy	y��ĳ���
* ���ڲ�������
* ˵	������MCUѡ��������Դ������ˢ�µ�LCD���Դ��У����Ҫ��ʾ��Ҫ�ڸú����������Ӻ��� SetRedraw(TRUE);
****************************************************************************/	
void FlushRect(s16 x, s16 y, s16 cx, s16 cy); 

/****************************************************************************
* ��	�ƣ�void SetColor(u32 nColor)
* ��	�ܣ�������ɫ
* ��ڲ�����u32 nColor	��0-�׵׺���	��1-�ڵװ��֣�
* ���ڲ�������
* ˵	����
****************************************************************************/	
void SetColor(u32 nColor);

/****************************************************************************
* ��	�ƣ�void InvertColor(void)
* ��	�ܣ���ת��ɫ
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/	
void InvertColor(void);

/****************************************************************************
* ��	�ƣ�FillRect(s16 x, s16 y, s16 cx, s16 cy)
* ��	�ܣ���ʵ�ľ���
* ��ڲ�����s16 x		���ε�x����ʼλ��
						s16 y		���ε�y����ʼλ��
						s16 cx	���ε�x�᳤��
						s16 cy	���ε�y�᳤��
* ���ڲ�������
* ˵	����
****************************************************************************/
void FillRect(s16 x, s16 y, s16 cx, s16 cy);		//x:[0,127]		y:[0,63]

/****************************************************************************
* ��	�ƣ�void DrawPoint(s16 x, s16 y)
* ��	�ܣ����㺯��
* ��ڲ�����s16	x	���x������
						s16	y ���y������
* ���ڲ�������
* ˵	����
****************************************************************************/	
void DrawPoint(s16 x, s16 y);

/****************************************************************************
* ��	�ƣ�void DrawHoriLine(s16 x, s16 y, s16 cx)
* ��	�ܣ���һ������
* ��ڲ�����s16 x		���ߵ�X����ʼλ��
						s16 y		���ߵ�Y����ʼλ��
						s16 cy	���ߵ�X�᳤��
* ���ڲ�������
* ˵	����
****************************************************************************/	
void DrawHoriLine(s16 x, s16 y, s16 cx);

/****************************************************************************
* ��	�ƣ�void DrawVertLine(s16 x, s16 y, s16 cy)
* ��	�ܣ���һ������
* ��ڲ�����s16 x		���ߵ�X����ʼλ��
						s16 y		���ߵ�Y����ʼλ��
						s16	cy	���ߵ�Y�᳤��
* ���ڲ�������
* ˵	����
****************************************************************************/	
void DrawVertLine(s16 x, s16 y, s16 cy);

/****************************************************************************
* ��	�ƣ�void DrawRect(s16 x, s16 y, s16 cx, s16 cy)
* ��	�ܣ�������
* ��ڲ�����s16	x		���ε�X����ʼλ��
						s16	y		���ε�Y����ʼλ��
						s16	cx	���ε�x�᳤��
						s16	cy	���ε�Y�᳤��
* ���ڲ�������
* ˵	����
****************************************************************************/	
void DrawRect(s16 x, s16 y, s16 cx, s16 cy);

/****************************************************************************
* ��	�ƣ�DrawImage(u8* pImage, s16 x, s16 y, s16 cx, s16 cy)
* ��	�ܣ����û���ͼ�εĴ�С1
* ��ڲ�����u8* pImage Ҫ��ʾ��ͼ��ĵ�ַ
						s16 x		����ʼλ��(0~240)
						s16 y		����ʼλ��(0~128)
						s16 cx 	�г���
						s16 cy	�г���
* ���ڲ�������
* ˵	����
****************************************************************************/	
void DrawImage(const u8* pImage, s16 x, s16 y, s16 cx, s16 cy);

/****************************************************************************
* ��	�ƣ�SupperDrawImage(u8* pImage, s16 _x, s16 _y, s16 _xImg, s16 _yImg, s16 _cxImg, s16 _cyImg, s16 _Width, s16 _Height)
* ��	�ܣ����û���ͼ�εĴ�С2
* ��ڲ�����u8* pImage Ҫ��ʾ��ͼ��ĵ�ַ
						s16 _x		��Ļ������ʼλ��(0~240)
						s16 _y		��Ļ������ʼλ��(0~128)
						s16 _xImg	����ͼ���ϵ������
						s16 _yImg	����ͼ���ϵ������
						s16 _cxImg	����ͼ���ϵ�����
						s16 _cyImg	����ͼ���ϵ�����
						
						s16 _Width 	ͼ��Ŀ���
						s16 _Height	ͼ��ĳ���
* ���ڲ�������
* ˵	����
****************************************************************************/	
void SupperDrawImage(const u8* pImage, s16 _x, s16 _y, s16 _xImg, s16 _yImg, s16 _cxImg, s16 _cyImg, s16 _Width, s16 _Height);


/****************************************************************************
* ��	�ƣ�void DrawBigChar(char c, s16 x, s16 y)
* ��	�ܣ����ƴ��ַ�(15*30)(Ŀǰ֧�ֵ��ַ���"0��9";".";"-";"t")
* ��ڲ�����char  c		Ҫ���Ƶ��ַ�
						s16 x		����ʼλ��(0~240)
						s16 y		����ʼλ��(0~128)
* ���ڲ�������
* ˵	����
****************************************************************************/	
 void DrawBigChar(char c, s16 x, s16 y);


/****************************************************************************
* ��	�ƣ� void DrawBigText(char *sNum, s16 x, s16 y)
* ��	�ܣ����ƴ��ַ���(15*30)(Ŀǰ֧�ֵ��ַ���"0��9";".";"-";"t")
* ��ڲ�����char *s		Ҫ���Ƶ��ַ���
						s16 x		����ʼλ��(0~240)
						s16 y		����ʼλ��(0~128)
* ���ڲ�������
* ˵	����
****************************************************************************/	
 void DrawBigText(char *sNum, s16 x, s16 y);
 
/****************************************************************************
* ��	�ƣ�DrawGbText(char *s, s16 x, s16 y)
* ��	�ܣ������ַ��ͺ���
* ��ڲ�����char *s		Ҫ���Ƶ��ַ���
						s16 x		����ʼλ��(0~240)
						s16 y		����ʼλ��(0~128)
* ���ڲ�������
* ˵	����
****************************************************************************/	
void DrawGbText(char *s, s16 x, s16 y);

#endif












