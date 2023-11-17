/**
  ******************************************************************************
  * @file    Callback_Action.c
  * @author  ׯ�ı�
  * @brief   �ص�����
  * @date    11/4/2023
  * 
    @verbatim
    ������Ҫ���ж�ִ�еĺ�����������һ�ļ�
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Call_Back_Action.h"
#include "zf_common_headfile.h"
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define Sensor_CH                  (PIT_CH0 )// ʹ�õ������жϱ�� ����޸� ��Ҫͬ����Ӧ�޸������жϱ���� isr.c �еĵ���
#define Sensor_PRIORITY            (PIT_IRQn)// ��Ӧ�����жϵ��жϱ�� 

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��ʱ����ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
void Handler_Init()
{
    pit_ms_init(Sensor_CH, 5);                                                  // ��ʼ�� PIT_CH0 Ϊ�����ж� 5ms ����
    interrupt_set_priority(Sensor_PRIORITY, 0); 
    pit_ms_init(PIT_CH1, 20);                                                  // ��ʼ�� PIT_CH0 Ϊ�����ж� 20ms ����
    interrupt_set_priority(PIT_IRQn, 0);    
}


/**@brief   �������жϺ���
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
void Sensor_Handler()
{
    Gyro_Get_All_Angles();
}