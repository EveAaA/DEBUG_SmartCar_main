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

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   蜂鸣器初始化
-- @param   无
-- @auther  庄文标
-- @date    2023/11/13
**/
void Beep_Init()
{
    gpio_init(Beep_Pin,GPO,0,GPO_PUSH_PULL);
}

/**@brief   蜂鸣器控制
-- @param   无
-- @auther  庄文标
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