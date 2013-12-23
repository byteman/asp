#include "includes.h"
#include "debug_info.h"

//CCM memory region
//extern unsigned  int Image$$RW_IRAM2$$Base;
//extern unsigned  int Image$$RW_IRAM2$$Length;
//extern unsigned  int Image$$RW_IRAM2$$Limit;

//ccm memory start 0x10000000 size 0x10000 (64Kbytes)

static unsigned char *pCMCCurBuf = (unsigned char *)(0x10000000); //&Image$$RW_IRAM2$$Base;
static unsigned char *pCMCEnd = (unsigned char *)(0x10010000);    //&Image$$RW_IRAM2$$Limit;


void *Ccm_Malloc(unsigned int size)
{
#if SYS_CRITICAL_METHOD == 3      /* Allocate storage for CPU status register      */
	SYS_CPU_SR	cpu_sr = 0;
#endif
	unsigned char *buf = NULL;
	
	SYS_ENTER_CRITICAL();
	
	//分配的内存强制4字节对齐
	size = (size + 3) & (~3);

	//内存不够
	if((pCMCCurBuf + size + 4) >= pCMCEnd)
	{
		SYS_EXIT_CRITICAL();
		return NULL;
	}
	
	buf = pCMCCurBuf;
	pCMCCurBuf += size;
	memset(buf, 0, size);
	
	SYS_EXIT_CRITICAL();
	
	return buf;
}


