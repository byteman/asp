#ifndef BEEP_H
#define BEEP_H

//蜂鸣器初始化
void Beep_Initial(void);

//cnt:  鸣叫次数
//time: 鸣叫时间 单位10ms
int32 Beep_Voice(uint32 cnt, uint16 time);

//蜂鸣器操作 1 on 0 off
void Beep_Power(uint8 type);


#endif
