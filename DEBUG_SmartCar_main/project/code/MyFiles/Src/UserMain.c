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
uint16 count = 0;
extern float test_1;
extern float test_2;
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
    Beep_Init();
    tft180_init();
    Bluetooth_Init();
    imu660ra_init();
    Gyro_Offset_Init();
    All_Encoder_Init();
    All_PID_Init();
    Pid_Init();
    All_Button_Init();
    Motor_Init();
    mt9v03x_init();
    Handler_Init();
    Manipulator_Init();
    Beep(On);
    system_delay_ms(100);
    Beep(Off);
}

/**@brief   所有主循环内容
-- @param   无
-- @auther  庄文标
-- @date    2023/9/12
**/
void User_Loop()
{
    All_Button_Scan();
    Meau_Display();
    if(mt9v03x_finish_flag)
    {
      Image_Process();
      mt9v03x_finish_flag = 0;
    }
}