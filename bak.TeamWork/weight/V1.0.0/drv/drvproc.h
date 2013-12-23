#ifndef _DRVPROC_H    
#define _DRVPROC_H

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#define SDIGSAMPLE(x)	(*((sDigSample*)x))



/*
 * 功能:   分度值处理 
 */
float WetDrvProc(float Wet, float Drv);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif

