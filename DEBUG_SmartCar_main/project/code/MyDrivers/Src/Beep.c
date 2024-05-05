/**
  ******************************************************************************
  * @file    Beep.c
  * @author  庄文标
  * @brief   蜂鸣器驱动
  * @date    11/13/2023
    @verbatim
    
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Beep.h"

/* Define\Declare ------------------------------------------------------------*/
#define Beep_Pin B21
uint16 Beep_Time = 0;

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   蜂鸣器初始化
-- @param   无
-- @author  庄文标
-- @date    2023/11/13
**/
void Beep_Init()
{
    gpio_init(Beep_Pin,GPO,0,GPO_PUSH_PULL);
}

/**@brief   蜂鸣器控制
-- @param   无
-- @author  庄文标
-- @date    2023/11/13
**/
void Beep(Beep_Handle Mode)
{
    switch(Mode)
    {
        case On:
            gpio_set_level(Beep_Pin,1);
        break;
        case Off:
            gpio_set_level(Beep_Pin,0);
        break;
    }
}

/**@brief   设置蜂鸣器响固定时间
-- @param   无
-- @author  庄文标
-- @date    2024/5/5
**/
void Beep_On()
{
    if(Beep_Time > 0)
    {
        Beep_Time-=1;
        Beep(On);
    }
    else
    {
        Beep(Off);
    }
}

/**@brief   设置蜂鸣器响的时间
-- @param   uint16 Set_Time 时间,单位是ms
-- @author  庄文标
-- @date    2024/5/5
**/
void Set_Beeptime(uint16 Set_Time)
{
    if(Beep_Time == 0)
    {
        Beep_Time = Set_Time/5;
    }
}