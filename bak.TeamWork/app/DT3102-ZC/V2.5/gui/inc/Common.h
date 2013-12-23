#ifndef __COMMON_H__
#define __COMMON_H__

 
#define ARRAY_SIZE(array,type) ((sizeof(array))/(sizeof(type)))
#define PACKED __attribute__((packed))


/*
 * 单位转换
 */
float kg2lg(float wet_kg);      // kg转换为lb
float lg2kg(float wet_lb);      // lb转换为kg
float CurVal2Kg(float wet);     // 当前系统单位的重量值转换为kg
float Kg2CurVal(float wet_kg);  // kg单位的重量值转换为当前显示的单位
unsigned char GetCurUnit(void);
char *GetCurUnitStr(void);      // 获取单位字符串
void SwitchUnit(void);			//单位转换
//unsigned char GetCurUnit(void); // 获取单位标志位 0:kg  1:lb
// void GetCpuIdHex(char* HCpuId);




#endif


