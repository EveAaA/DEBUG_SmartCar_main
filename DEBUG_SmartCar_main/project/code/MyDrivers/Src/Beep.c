/**
  ******************************************************************************
  * @file    Beep.c
  * @author  ׯ�ı�
  * @brief   ����������
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
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��������ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/13
**/
void Beep_Init()
{
    gpio_init(Beep_Pin,GPO,0,GPO_PUSH_PULL);
}

/**@brief   ����������
-- @param   ��
-- @auther  ׯ�ı�
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