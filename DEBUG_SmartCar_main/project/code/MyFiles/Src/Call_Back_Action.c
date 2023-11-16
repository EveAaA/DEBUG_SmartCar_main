/**
  ******************************************************************************
  * @file    Callback_Action.c
  * @author  庄文标
  * @brief   回调函数
  * @date    11/4/2023
  * 
    @verbatim
    所有需要在中断执行的函数都放在这一文件
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Call_Back_Action.h"
#include "zf_common_headfile.h"
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define Sensor_CH                  (PIT_CH0 )// 使用的周期中断编号 如果修改 需要同步对应修改周期中断编号与 isr.c 中的调用
#define Sensor_PRIORITY            (PIT_IRQn)// 对应周期中断的中断编号 

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   定时器初始化
-- @param   无
-- @auther  庄文标
-- @date    2023/11/4
**/
void Handler_Init()
{
    pit_ms_init(Sensor_CH, 5);                                                  // 初始化 PIT_CH0 为周期中断 5ms 周期
    interrupt_set_priority(Sensor_PRIORITY, 0); 
    pit_ms_init(PIT_CH1, 20);                                                  // 初始化 PIT_CH0 为周期中断 20ms 周期
    interrupt_set_priority(PIT_IRQn, 0);    
}


/**@brief   传感器中断函数
-- @param   无
-- @auther  庄文标
-- @date    2023/11/4
**/
void Sensor_Handler()
{
    Gyro_Get_All_Angles();
}