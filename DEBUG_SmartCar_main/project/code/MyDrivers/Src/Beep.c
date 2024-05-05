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
uint16 Beep_Time = 0;

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��������ʼ��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/13
**/
void Beep_Init()
{
    gpio_init(Beep_Pin,GPO,0,GPO_PUSH_PULL);
}

/**@brief   ����������
-- @param   ��
-- @author  ׯ�ı�
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

/**@brief   ���÷�������̶�ʱ��
-- @param   ��
-- @author  ׯ�ı�
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

/**@brief   ���÷��������ʱ��
-- @param   uint16 Set_Time ʱ��,��λ��ms
-- @author  ׯ�ı�
-- @date    2024/5/5
**/
void Set_Beeptime(uint16 Set_Time)
{
    if(Beep_Time == 0)
    {
        Beep_Time = Set_Time/5;
    }
}