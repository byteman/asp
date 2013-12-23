#ifndef __ADCVALUECHECK_H_
#define __ADCVALUECHECK_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#define  MAXADCVALUE    (long)5000000
#define  MinADCVALUE    (long)-5000000

/*
 * 功能:    传感器断线检测
 * 输入:
 *          ad : AD值
 * 输出     0: 正常   -1: 异常
 */
int  ADCValueCheck(long ad);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
