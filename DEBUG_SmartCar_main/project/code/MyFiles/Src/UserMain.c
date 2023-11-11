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
uint16 Start = 0;
extern float test_1;
extern float test_2;
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
    system_delay_ms(300);
    Motor_Init();
    Bluetooth_Init();
    Bluetooth_Set_Watch_Variable(Num_Address, CH1, &test_1);
    Bluetooth_Set_Watch_Variable(Num_Address, CH2, &test_2);
    tft180_init();
    imu660ra_init();
    Gyro_Offset_Init();
    All_Encoder_Init();
    All_PID_Init();
    All_Button_Init();
    Sensor_Handler_Init();
}


/**@brief   ������ѭ������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/9/12
**/
void User_Loop()
{
    tft180_show_float(0,0,Gyro_YawAngle_Get(),2,1);
    test_1 = Gyro_YawAngle_Get();
    test_2 = 0;
    Bluetooth_Send_Float(Num_Address);
    All_Button_Scan();
    if(Button_Value[2] == 1)
    {
      Button_Value[2] = 0;
      Start = 1;
    }
    if(Button_Value[3] == 1)
    {
      Button_Value[3] = 0;
      Start = 0;
    }
}