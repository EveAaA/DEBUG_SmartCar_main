/**
  ******************************************************************************
  * @file    UserMain.c
  * @author  ׯ�ı�
  * @brief   ������
  *
    @verbatim
    ȫ����ʼ������д��UserInit�У�������while������д��UserLoop��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "UserMain.h"
#include "zf_common_headfile.h"

/* Define\Declare ------------------------------------------------------------*/
uint16 Start = 2;

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ���г�ʼ������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
void User_Init()
{
    system_delay_ms(100);
    Beep_Init();
    tft180_init();
    Bluetooth_Init();
    imu660ra_init();
    Gyro_Offset_Init();
    All_Encoder_Init();
    All_PID_Init();
    Pid_Init();
    Motor_Init();
    mt9v03x_init();
    Rotary_Init();
    Flash_Init();
    Handler_Init();
    Manipulator_Init();
    // Beep(On);
    // system_delay_ms(100);
    // Beep(Off);
}

/**@brief   ������ѭ������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/9/12
**/
void User_Loop()
{
    Menu_Display();
    if(mt9v03x_finish_flag)
    {
      Image_Process();
      mt9v03x_finish_flag = 0;
    }
}