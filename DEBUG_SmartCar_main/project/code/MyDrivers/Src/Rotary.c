/**
  ******************************************************************************
  * @file    Rotary.c
  * @author  ׯ�ı�
  * @brief   ��ת����������
  * @date    2/16/2024
    @verbatim
    
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Rotary.h"

/* Define\Declare ------------------------------------------------------------*/
Rotary_Handle Rotary = {0,0,0};

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��ת��������ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/2/16
**/
void Rotary_Init()
{
    exti_init(Rotary_A, EXTI_TRIGGER_FALLING);
    exti_init(Rotary_B, EXTI_TRIGGER_FALLING);
    exti_init(Rotary_D, EXTI_TRIGGER_LOW);
    // gpio_init(Rotary_D,GPI,1,GPI_PULL_UP);
    interrupt_set_priority(GPIO1_Combined_16_31_IRQn,1);
    interrupt_set_priority(GPIO1_Combined_0_15_IRQn,2);
    interrupt_set_priority(GPIO2_Combined_16_31_IRQn,3);
}