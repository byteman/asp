#ifndef __SPEED_CHECK_H__
#define __SPEED_CHECK_H__

#include "includes.h"

/* 速度计算函数 */
void Check_CarSpeed (void);
/* 速度获取函数 */
uint16 Get_CarSpeed (void);

void Set_Speed_Length(uint16 len);
uint16 Get_Speed_Length(void);


void Set_CarSpeed(int32 speed);


#endif //__SPEED_CHECK_H__
