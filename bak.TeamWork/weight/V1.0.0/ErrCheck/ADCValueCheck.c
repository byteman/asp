/*! 
\file ADCValueCheck.c
*/

#include "ADCValueCheck.h"

/*!
 * \brief		传感器断线检测
 * \param[in]	ad  AD值
 * \return		返回检测结果         
 * \retval 0    正常   
 * \retval -1   异常
 */
int  ADCValueCheck(long ad)
{
    return ((ad > MAXADCVALUE)||(ad < MinADCVALUE))?(char)-1:0;
}





