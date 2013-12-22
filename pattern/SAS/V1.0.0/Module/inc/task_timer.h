#ifndef TASK_TIMER_H
#define TASK_TIMER_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif


void Task_Timer_Create(void);

void Task_Timer_Start(void);	

/*
 *	These inlines deal with timer wrapping correctly. You are 
 *	strongly encouraged to use them
 *	1. Because people otherwise forget
 *	2. Because if the timer wrap changes in future you won't have to
 *	   alter your driver code.
 *
 * time_after(a,b) returns TRUE if the time a is after time b.
 *
 * Do this with "<0" and ">=0" to only test the sign of the result. A
 * good compiler would generate better code (and a really good compiler
 * wouldn't care). Gcc is currently neither.
 */

//a and b is unsigned long type
#define time_after(a,b)	((long)(b) - (long)(a) < 0)
#define time_before(a,b) time_after(b,a)

#define time_after_eq(a,b) ((long)(a) - (long)(b) >= 0)
#define time_before_eq(a,b)	time_after_eq(b,a)

unsigned long Get_Sys_Tick(void);

#ifdef __cplusplus
}
#endif

#endif
