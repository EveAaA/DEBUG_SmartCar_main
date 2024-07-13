/**
  ******************************************************************************
  * @file    Rotary.c
  * @author  庄文标
  * @brief   旋转编码器驱动
  * @date    2/16/2024
    @verbatim
    读取键值写中断里了，直接调用结构体就可以
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Rotary.h"

/* Define\Declare ------------------------------------------------------------*/
Rotary_Handle Rotary = {0,0,0};

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   旋转编码器初始化
-- @param   无
-- @author  庄文标
-- @date    2024/2/16
**/
void Rotary_Init()
{
    exti_init(Rotary_A, EXTI_TRIGGER_BOTH);
    gpio_init(Rotary_B,GPI,0,GPI_PULL_UP);
    gpio_init(Rotary_D,GPI,0,GPI_PULL_UP);
    interrupt_set_priority(GPIO1_Combined_16_31_IRQn,1);
}