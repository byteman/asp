/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                            MEMORY MANAGEMENT
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : SYS_MEM.C
* By      : Jean J. Labrosse
* Version : V2.86
*
* LICENSING TERMS:
* ---------------
*   uC/Sys-II is provided in source form for FREE evaluation, for educational use or for peaceful research.  
* If you plan on using  uC/Sys-II  in a commercial product you need to contact Micriµm to properly license 
* its use in your product. We provide ALL the source code for your convenience and to help you experience 
* uC/Sys-II.   The fact that the  source is provided does  NOT  mean that you can use it without  paying a 
* licensing fee.
*********************************************************************************************************
*/

#ifndef  SYS_MASTER_FILE
#include <C2.h>
#endif

#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)
/*
*********************************************************************************************************
*                                        CREATE A MEMORY PARTITION
*
* Description : Create a fixed-sized memory partition that will be managed by uC/Sys-II.
*
* Arguments   : addr     is the starting address of the memory partition
*
*               nblks    is the number of memory blocks to create from the partition.
*
*               blksize  is the size (in bytes) of each block in the memory partition.
*
*               perr     is a pointer to a variable containing an error message which will be set by
*                        this function to either:
*
*                        SYS_ERR_NONE              if the memory partition has been created correctly.
*                        SYS_ERR_MEM_INVALID_ADDR  if you are specifying an invalid address for the memory
*                                                 storage of the partition or, the block does not align
*                                                 on a pointer boundary
*                        SYS_ERR_MEM_INVALID_PART  no free partitions available
*                        SYS_ERR_MEM_INVALID_BLKS  user specified an invalid number of blocks (must be >= 2)
*                        SYS_ERR_MEM_INVALID_SIZE  user specified an invalid block size
*                                                   - must be greater than the size of a pointer
*                                                   - must be able to hold an integral number of pointers
* Returns    : != (SYS_MEM *)0  is the partition was created
*              == (SYS_MEM *)0  if the partition was not created because of invalid arguments or, no
*                              free partition is available.
*********************************************************************************************************
*/

SYS_MEM  *SysMemCreate (void *addr, INT32U nblks, INT32U blksize, INT8U *perr)
{
    SYS_MEM    *pmem;
    INT8U     *pblk;
    void     **plink;
    INT32U     i;
#if SYS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0              
    if (perr == (INT8U *)0) {                         /* Validate 'perr'                               */
        return ((SYS_MEM *)0);
    }
    if (addr == (void *)0) {                          /* Must pass a valid address for the memory part.*/
        *perr = SYS_ERR_MEM_INVALID_ADDR;
        return ((SYS_MEM *)0);
    }
    if (((INT32U)addr & (sizeof(void *) - 1)) != 0){  /* Must be pointer size aligned                  */
        *perr = SYS_ERR_MEM_INVALID_ADDR;
        return ((SYS_MEM *)0);
    }
    if (nblks < 2) {                                  /* Must have at least 2 blocks per partition     */
        *perr = SYS_ERR_MEM_INVALID_BLKS;
        return ((SYS_MEM *)0);
    }
    if (blksize < sizeof(void *)) {                   /* Must contain space for at least a pointer     */
        *perr = SYS_ERR_MEM_INVALID_SIZE;
        return ((SYS_MEM *)0);
    }
#endif
    SYS_ENTER_CRITICAL();
    pmem = SysMemFreeList;                             /* Get next free memory partition                */
    if (SysMemFreeList != (SYS_MEM *)0) {               /* See if pool of free partitions was empty      */
        SysMemFreeList = (SYS_MEM *)SysMemFreeList->SysMemFreeList;
    }
    SYS_EXIT_CRITICAL();
    if (pmem == (SYS_MEM *)0) {                        /* See if we have a memory partition             */
        *perr = SYS_ERR_MEM_INVALID_PART;
        return ((SYS_MEM *)0);
    }
    plink = (void **)addr;                            /* Create linked list of free memory blocks      */
    pblk  = (INT8U *)((INT32U)addr + blksize);
    for (i = 0; i < (nblks - 1); i++) {
       *plink = (void *)pblk;                         /* Save pointer to NEXT block in CURRENT block   */
        plink = (void **)pblk;                        /* Position to  NEXT      block                  */
        pblk  = (INT8U *)((INT32U)pblk + blksize);    /* Point to the FOLLOWING block                  */
    }
    *plink              = (void *)0;                  /* Last memory block points to NULL              */
    pmem->SysMemAddr     = addr;                       /* Store start address of memory partition       */
    pmem->SysMemFreeList = addr;                       /* Initialize pointer to pool of free blocks     */
    pmem->SysMemNFree    = nblks;                      /* Store number of free blocks in MCB            */
    pmem->SysMemNBlks    = nblks;
    pmem->SysMemBlkSize  = blksize;                    /* Store block size of each memory blocks        */
    *perr               = SYS_ERR_NONE;
    return (pmem);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                          GET A MEMORY BLOCK
*
* Description : Get a memory block from a partition
*
* Arguments   : pmem    is a pointer to the memory partition control block
*
*               perr    is a pointer to a variable containing an error message which will be set by this
*                       function to either:
*
*                       SYS_ERR_NONE             if the memory partition has been created correctly.
*                       SYS_ERR_MEM_NO_FREE_BLKS if there are no more free memory blocks to allocate to caller
*                       SYS_ERR_MEM_INVALID_PMEM if you passed a NULL pointer for 'pmem'
*
* Returns     : A pointer to a memory block if no error is detected
*               A pointer to NULL if an error is detected
*********************************************************************************************************
*/

void  *SysMemGet (SYS_MEM *pmem, INT8U *perr)
{
    void      *pblk;
#if SYS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                         /* Validate 'perr'                               */
        return ((void *)0);
    }
    if (pmem == (SYS_MEM *)0) {                        /* Must point to a valid memory partition        */
        *perr = SYS_ERR_MEM_INVALID_PMEM;
        return ((void *)0);
    }
#endif
    SYS_ENTER_CRITICAL();
    if (pmem->SysMemNFree > 0) {                       /* See if there are any free memory blocks       */
        pblk                = pmem->SysMemFreeList;    /* Yes, point to next free memory block          */
        pmem->SysMemFreeList = *(void **)pblk;         /*      Adjust pointer to new free list          */
        pmem->SysMemNFree--;                           /*      One less memory block in this partition  */
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_NONE;                          /*      No error                                 */
        return (pblk);                                /*      Return memory block to caller            */
    }
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_MEM_NO_FREE_BLKS;                  /* No,  Notify caller of empty memory partition  */
    return ((void *)0);                               /*      Return NULL pointer to caller            */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                   GET THE NAME OF A MEMORY PARTITION
*
* Description: This function is used to obtain the name assigned to a memory partition.
*
* Arguments  : pmem      is a pointer to the memory partition
*
*              pname     is a pointer to an ASCII string that will receive the name of the memory partition.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the name was copied to 'pname'
*                        SYS_ERR_MEM_INVALID_PMEM    if you passed a NULL pointer for 'pmem'
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_NAME_GET_ISR        You called this function from an ISR
*
* Returns    : The length of the string or 0 if 'pmem' is a NULL pointer.
*********************************************************************************************************
*/

#if SYS_MEM_NAME_SIZE > 1
INT8U  SysMemNameGet (SYS_MEM *pmem, INT8U *pname, INT8U *perr)
{
    INT8U      len;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                    /* Validate 'perr'                                    */
        return (0);
    }
    if (pmem == (SYS_MEM *)0) {                   /* Is 'pmem' a NULL pointer?                          */
        *perr = SYS_ERR_MEM_INVALID_PMEM;
        return (0);
    }
    if (pname == (INT8U *)0) {                   /* Is 'pname' a NULL pointer?                         */
        *perr = SYS_ERR_PNAME_NULL;
        return (0);
    }
#endif
    if (SysIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        *perr = SYS_ERR_NAME_GET_ISR;
        return (0);
    }
    SYS_ENTER_CRITICAL();
    len   = SYS_StrCopy(pname, pmem->SysMemName);  /* Copy name from SYS_MEM                              */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return (len);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                 ASSIGN A NAME TO A MEMORY PARTITION
*
* Description: This function assigns a name to a memory partition.
*
* Arguments  : pmem      is a pointer to the memory partition
*
*              pname     is a pointer to an ASCII string that contains the name of the memory partition.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the name was copied to 'pname'
*                        SYS_ERR_MEM_INVALID_PMEM    if you passed a NULL pointer for 'pmem'
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_MEM_NAME_TOO_LONG   if the name doesn't fit in the storage area
*                        SYS_ERR_NAME_SET_ISR        if you called this function from an ISR
*
* Returns    : None
*********************************************************************************************************
*/

#if SYS_MEM_NAME_SIZE > 1
void  SysMemNameSet (SYS_MEM *pmem, INT8U *pname, INT8U *perr)
{
    INT8U      len;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                    /* Validate 'perr'                                    */
        return;
    }
    if (pmem == (SYS_MEM *)0) {                   /* Is 'pmem' a NULL pointer?                          */
        *perr = SYS_ERR_MEM_INVALID_PMEM;
        return;
    }
    if (pname == (INT8U *)0) {                   /* Is 'pname' a NULL pointer?                         */
        *perr = SYS_ERR_PNAME_NULL;
        return;
    }
#endif
    if (SysIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        *perr = SYS_ERR_NAME_SET_ISR;
        return;
    }
    SYS_ENTER_CRITICAL();
    len = SYS_StrLen(pname);                      /* Can we fit the string in the storage area?         */
    if (len > (SYS_MEM_NAME_SIZE - 1)) {          /* No                                                 */
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_MEM_NAME_TOO_LONG;
        return;
    }
    (void)SYS_StrCopy(pmem->SysMemName, pname);    /* Yes, copy name to the memory partition header      */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                         RELEASE A MEMORY BLOCK
*
* Description : Returns a memory block to a partition
*
* Arguments   : pmem    is a pointer to the memory partition control block
*
*               pblk    is a pointer to the memory block being released.
*
* Returns     : SYS_ERR_NONE              if the memory block was inserted into the partition
*               SYS_ERR_MEM_FULL          if you are returning a memory block to an already FULL memory
*                                        partition (You freed more blocks than you allocated!)
*               SYS_ERR_MEM_INVALID_PMEM  if you passed a NULL pointer for 'pmem'
*               SYS_ERR_MEM_INVALID_PBLK  if you passed a NULL pointer for the block to release.
*********************************************************************************************************
*/

INT8U  SysMemPut (SYS_MEM *pmem, void *pblk)
{
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (pmem == (SYS_MEM *)0) {                   /* Must point to a valid memory partition             */
        return (SYS_ERR_MEM_INVALID_PMEM);
    }
    if (pblk == (void *)0) {                     /* Must release a valid block                         */
        return (SYS_ERR_MEM_INVALID_PBLK);
    }
#endif
    SYS_ENTER_CRITICAL();
    if (pmem->SysMemNFree >= pmem->SysMemNBlks) {  /* Make sure all blocks not already returned          */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_MEM_FULL);
    }
    *(void **)pblk      = pmem->SysMemFreeList;   /* Insert released block into free block list         */
    pmem->SysMemFreeList = pblk;
    pmem->SysMemNFree++;                          /* One more memory block in this partition            */
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);                        /* Notify caller that memory block was released       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                          QUERY MEMORY PARTITION
*
* Description : This function is used to determine the number of free memory blocks and the number of
*               used memory blocks from a memory partition.
*
* Arguments   : pmem        is a pointer to the memory partition control block
*
*               p_mem_data  is a pointer to a structure that will contain information about the memory
*                           partition.
*
* Returns     : SYS_ERR_NONE               if no errors were found.
*               SYS_ERR_MEM_INVALID_PMEM   if you passed a NULL pointer for 'pmem'
*               SYS_ERR_MEM_INVALID_PDATA  if you passed a NULL pointer to the data recipient.
*********************************************************************************************************
*/

#if SYS_MEM_QUERY_EN > 0
INT8U  SysMemQuery (SYS_MEM *pmem, SYS_MEM_DATA *p_mem_data)
{
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (pmem == (SYS_MEM *)0) {                   /* Must point to a valid memory partition             */
        return (SYS_ERR_MEM_INVALID_PMEM);
    }
    if (p_mem_data == (SYS_MEM_DATA *)0) {        /* Must release a valid storage area for the data     */
        return (SYS_ERR_MEM_INVALID_PDATA);
    }
#endif
    SYS_ENTER_CRITICAL();
    p_mem_data->SysAddr     = pmem->SysMemAddr;
    p_mem_data->SysFreeList = pmem->SysMemFreeList;
    p_mem_data->SysBlkSize  = pmem->SysMemBlkSize;
    p_mem_data->SysNBlks    = pmem->SysMemNBlks;
    p_mem_data->SysNFree    = pmem->SysMemNFree;
    SYS_EXIT_CRITICAL();
    p_mem_data->SysNUsed    = p_mem_data->SysNBlks - p_mem_data->SysNFree;
    return (SYS_ERR_NONE);
}
#endif                                           /* SYS_MEM_QUERY_EN                                    */
/*$PAGE*/
/*
*********************************************************************************************************
*                                    INITIALIZE MEMORY PARTITION MANAGER
*
* Description : This function is called by uC/Sys-II to initialize the memory partition manager.  Your
*               application MUST NOT call this function.
*
* Arguments   : none
*
* Returns     : none
*
* Note(s)    : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/

void  SYS_MemInit (void)
{
#if SYS_MAX_MEM_PART == 1
    SYS_MemClr((INT8U *)&SysMemTbl[0], sizeof(SysMemTbl));   /* Clear the memory partition table          */
    SysMemFreeList               = (SYS_MEM *)&SysMemTbl[0]; /* Point to beginning of free list           */
#if SYS_MEM_NAME_SIZE > 1
    SysMemFreeList->SysMemName[0] = '?';                    /* Unknown name                              */
    SysMemFreeList->SysMemName[1] = SYS_ASCII_NUL;
#endif
#endif

#if SYS_MAX_MEM_PART >= 2
    SYS_MEM  *pmem;
    INT16U   i;


    SYS_MemClr((INT8U *)&SysMemTbl[0], sizeof(SysMemTbl));   /* Clear the memory partition table          */
    pmem = &SysMemTbl[0];                                  /* Point to memory control block (MCB)       */
    for (i = 0; i < (SYS_MAX_MEM_PART - 1); i++) {         /* Init. list of free memory partitions      */
        pmem->SysMemFreeList = (void *)&SysMemTbl[i+1];     /* Chain list of free partitions             */
#if SYS_MEM_NAME_SIZE > 1
        pmem->SysMemName[0]  = '?';                        /* Unknown name                              */
        pmem->SysMemName[1]  = SYS_ASCII_NUL;
#endif
        pmem++;
    }
    pmem->SysMemFreeList = (void *)0;                      /* Initialize last node                      */
#if SYS_MEM_NAME_SIZE > 1
    pmem->SysMemName[0]  = '?';                            /* Unknown name                              */
    pmem->SysMemName[1]  = SYS_ASCII_NUL;
#endif

    SysMemFreeList       = &SysMemTbl[0];                   /* Point to beginning of free list           */
#endif
}
#endif                                                    /* SYS_MEM_EN                                 */
