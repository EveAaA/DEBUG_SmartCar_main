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
    system_delay_ms(300);
    Motor_Init();
    Bluetooth_Init();
    tft180_init();
    imu660ra_init();
    Gyro_Offset_Init();
    All_Encoder_Init();
    All_PID_Init();
    All_Button_Init();
    Sensor_Handler_Init();
	interrupt_global_enable(0);
}


/**@brief   所有主循环内容
-- @param   无
-- @auther  庄文标
-- @date    2023/9/12
**/
void User_Loop()
{
  	Bluetooth_Store_Data();
	
}