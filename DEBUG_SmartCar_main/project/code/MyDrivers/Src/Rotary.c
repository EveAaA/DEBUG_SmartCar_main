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
-- @author  ׯ�ı�
-- @date    2024/2/16
**/
void Rotary_Init()
{
    exti_init(Rotary_A, EXTI_TRIGGER_BOTH);
    gpio_init(Rotary_B,GPI,0,GPI_PULL_UP);
    exti_init(Rotary_D, EXTI_TRIGGER_LOW);
    interrupt_set_priority(GPIO1_Combined_16_31_IRQn,1);
    interrupt_set_priority(GPIO2_Combined_16_31_IRQn,3);
}