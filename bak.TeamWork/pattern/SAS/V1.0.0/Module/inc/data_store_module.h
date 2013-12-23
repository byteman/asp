#ifndef __DATA_STORE_MODULE_H__
#define __DATA_STORE_MODULE_H__

#include "includes.h"

#define MAX_NORMAL_VEHICLE_NUMBER					4096					//存储正常车辆数最大值
#define MAX_OVERWEIGHT_VEHICLE_NUMBER				4096					//存储超载车辆数最大值

//DataFlash初始化函数
void init_flash(void);

//历史车辆数操作
uint16 Get_Flash_OverWetCarNum(void);
uint16 Get_Flash_NormalCarNum(void);
uint16 Get_Flash_NormalCarNumFront(void);
uint16 Get_Flash_NormalCarNumRear(void);

void Erase_Flash_Chip(void);


//存储正常过车数据
uint8 Store_Normal_Vehicle(const void *data, uint8 len);

//存储超载车辆数据
uint8 Store_Overweight_Vehicle(const void *data, uint8 len);

//查询正常过车数据
void *Query_Normal_Vehicle(uint16 id, uint8 len);

//查询超载车辆数据
void *Query_Overweight_Vehicle(uint16 id, uint8 len);


#endif //__DATA_STORE_MODULE_H__

