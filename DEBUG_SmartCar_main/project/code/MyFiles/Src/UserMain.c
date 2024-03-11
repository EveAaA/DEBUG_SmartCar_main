/**
  ******************************************************************************
  * @file    UserMain.c
  * @author  庄文标
  * @brief   主程序
  *
    @verbatim
    全部初始化内容写在UserInit中，主程序while的内容写在UserLoop中
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
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   所有初始化内容
-- @param   无
-- @auther  庄文标
-- @date    2023/11/4
**/
void User_Init()
{
    system_delay_ms(100);
    // Beep_Init();
    tft180_init();
    Bluetooth_Init();
    imu660ra_init();
    system_delay_ms(100);
    Gyro_Offset_Init();
    // All_Encoder_Init();
    // All_PID_Init();
    // Pid_Init();
    Manipulator_Init();
    Motor_Init();
    // dl1a_init();
    mt9v03x_init();
    Rotary_Init();
    Flash_Init();
    TIM_Init();
    
    // Beep(On);
    // system_delay_ms(100);
    // Beep(Off);
}

/**@brief   所有主循环内容
-- @param   无
-- @auther  庄文标
-- @date    2023/9/12
**/
void User_Loop()
{  
    if(mt9v03x_finish_flag)
    {
      Image_Process();
      mt9v03x_finish_flag = 0;
    }
    tft180_show_gray_image(0, 0, (const uint8 *)(Original_Image), MT9V03X_W, MT9V03X_H, (Row_18), (Line_5), Image_Thereshold);
    // Menu_Display();
}