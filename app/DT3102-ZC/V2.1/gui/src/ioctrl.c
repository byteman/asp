#include "includes.h"
#include "ioctrl.h"



// extern SDEVSET      gDevSet;
INTEST	  InTest;
OUTTEST	OutTest;	


void OutPortTest(void)//输出测试
{
	if( 1 == GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_0) )
	{
			OutTest.Out0 = 1;
	}
	else
	{
			OutTest.Out0 = 0;
	}
	
	if( 1 == GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_1) )
	{
			OutTest.Out1 = 1;
	}
	else
	{
			OutTest.Out1 = 0;
	}
	
	if( 1 == GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_2) )
	{
			OutTest.Out2 = 1;
	}
	else
	{
			OutTest.Out2 = 0;
	}
	
	if( 1 == GPIO_ReadOutputDataBit(GPIOE , GPIO_Pin_3) )
	{
			OutTest.Out3 = 1;
	}
	else
	{
			OutTest.Out3 = 0;
	}
}

void InPortTest(void)//输入测试
{
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_0) )
	{
			InTest.In0 = 1;
	}
	else
	{
			InTest.In0 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_1) )
	{
			InTest.In1 = 1;
	}
	else
	{
			InTest.In1 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_2) )
	{
			InTest.In2 = 1;
	}
	else
	{
			InTest.In2 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_3) )
	{
			InTest.In3 = 1;
	}
	else
	{
			InTest.In3 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_4) )
	{
			InTest.In4 = 1;
	}
	else
	{
			InTest.In4 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_5) )
	{
			InTest.In5 = 1;
	}
	else
	{
			InTest.In5 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_6) )
	{
			InTest.In6 = 1;
	}
	else
	{
			InTest.In6 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_7) )
	{
			InTest.In7 = 1;
	}
	else
	{
			InTest.In7 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_8) )
	{
			InTest.In8 = 1;
	}
	else
	{
			InTest.In8 = 0;
	}
	
	if( 1 == GPIO_ReadInputDataBit(GPIOF , GPIO_Pin_9) )
	{
			InTest.In9 = 1;
	}
	else
	{
			InTest.In9 = 0;
	}
}
// u8 ReadInfCurtain(void)
// {
//     if(gDevSet.GMCheck!=GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_3))     // 光幕异常
//         return 0;
//     if(gDevSet.GMNoCar==GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_4)) {
//         return 1;                                   // 无车
//     } else {
//         return 2;                                   // 有车
//     }
// }

// u8 ReadInductCoil(void)
// {
//     if(gDevSet.DGCheck!=GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_6))     // 地感异常
//         return 0;
//     if(gDevSet.DGNoCar==GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_5)) {
//         return 1;                                   // 无车
//     } else {
//         return 2;                                   // 有车
//     }
// }

// void LedSet(u8 en)
// {
//     if(en) {
//         GPIO_ResetBits(LED_GPIO_PORT,LED_PIN);
//     } else {
//         GPIO_SetBits(LED_GPIO_PORT,LED_PIN);
//     }
// }
// void LedToggle(void)
// {
//     GPIO_ToggleBits(LED_GPIO_PORT,LED_PIN);
// }

