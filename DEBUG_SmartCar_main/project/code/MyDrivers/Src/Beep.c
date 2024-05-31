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
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define Beep_Pin B21
int16 Beep_Time = 0;
uint8 Beep_Count = 0;
uint16 Beep_Cnt = 0;
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

/**@brief   �������ж�ʱ��
-- @author  ׯ�ı�
-- @date    2024/5/5
**/
static bool Bufcnt_Beep(bool Cond,uint16 Cnt)
{
    if(Cond)//��������
    {
        if(Beep_Cnt == 0)
        {
            Beep_Cnt = 1;//��ʼ��ʱ
        }
    }

    if(Beep_Cnt >= Cnt)
    {
        Beep_Cnt = 0;
        return true;
    }
    else
    {
        return false;
    }
}

/**@brief   ��������̶�����
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/5/30
**/
void Beep_Freq()
{
    static uint8 Beep_State = 0;
    switch (Beep_State)
    {
        case 0:
            if(Beep_Count > 0)
            {
                Beep_State = 1;
            }
        break;
        case 1:
            Set_Beeptime(100);
            if(Bufcnt_Beep(true,100))
            {
                Beep_State = 2;
                Beep_Count -=1;
            }
        break;
        case 2:
            if(Bufcnt_Beep(true,100))
            {
                Beep_State = 3;
            }
        break;
        case 3:
            if(Beep_Count == 0)
            {
                Beep_State = 0;
            }
            else
            {
                Beep_State = 1;
            }
        break;
    }
}

/**@brief   ���÷�������Ĵ���
-- @param   uint8 Count ��������Ĵ���
-- @author  ׯ�ı�
-- @date    2024/5/30
**/
void Set_Beepfreq(uint8 Count)
{
    Beep_Count = Count;
}