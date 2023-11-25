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
uint16 count = 0;
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
    system_delay_ms(100);
    // Beep_Init();
    tft180_init();
    // // Bluetooth_Init();
    imu660ra_init();
    Gyro_Offset_Init();
    // All_Encoder_Init();
    // All_PID_Init();
    All_Button_Init();
    // Motor_Init();
    // mt9v03x_init();
    Handler_Init();
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
//  test_1 = Gyro_YawAngle_Get();
//  test_2 = 0;
//  Bluetooth_Send_Float(Num_Address);
    All_Button_Scan();
    tft180_show_float(Row_0,Line_0,count,3,1);
    if(Button_Value[0])
    {
      Button_Value[0] = 0;
      count++;
    }
    if(Button_Value[1])
    {
      Button_Value[1] = 0;
      count+=2;
    }
    if(Button_Value[2])
    {
      Button_Value[2] = 0;
      count--;
    }
    if(Button_Value[3])
    {
      Button_Value[3] = 0;
      count-=2;
    }
    if(Button_Value[4])
    {
      Button_Value[4] = 0;
      count+=5;
    }
    // Meau_Display();
    // Get_Image(mt9v03x_image);//��ȡһ��ͼ��
    // tft180_show_gray_image(0, 0, (Original_Image[0]), 188, 120, (159), (120), Otsu_Threshold(Original_Image[0], 188, 120));
    // Set_Motor_Speed(LMotor_F,50);
}