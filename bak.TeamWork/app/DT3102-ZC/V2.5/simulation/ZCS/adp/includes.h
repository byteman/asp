#ifndef _INCLUDES_H_ 
#define _INCLUDES_H_ 

#define _SIMULATION_
//#define _SIMULATION_NOPAY_

#define u8 unsigned char
#define uint8 unsigned char
#define u16 unsigned short
#define uint16 unsigned short
#define u32 unsigned int
#define uint32 unsigned int

#define int8 char
#define int16 short
#define int32 int
#define s8 char
#define s16 short
#define s32 int

#ifndef BOOL
#define BOOL   unsigned char
#endif

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "debug_info.h"

#define snprintf		_snprintf

#define SYS_ENTER_CRITICAL()
#define SYS_EXIT_CRITICAL()

unsigned long Get_Sys_Tick(void);
#endif