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
#include "User_FSM.h"

/* Define\Declare ------------------------------------------------------------*/
uint16 Start = 2;

/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
 **/
void IMU_Init()
{
    while (1)
    {
        if (imu660ra_init())
            tft180_show_string(Row_0, Row_0, "mt9v03x reinit.");
        else
            break;
        system_delay_ms(1000);
    }
    system_delay_ms(100);
    tft180_show_string(Row_0, Line_0, "IMU Init ...");
    Gyro_Offset_Init();
}

void Mt9v03x_Init()
{
    while (1)
    {
        if (mt9v03x_init())
            tft180_show_string(0, 16, "mt9v03x reinit.");
        else
            break;
        system_delay_ms(1000); // 闪灯表示异常
    }
    tft180_show_string(Row_0, Line_1, "Mt9v03x Init ...");
}

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
    system_delay_ms(100);//延时一会等待所有外设上电
    tft180_init();
    IMU_Init();
    Mt9v03x_Init();
    All_Encoder_Init();
    tft180_show_string(Row_0, Line_2, "Encoder Init ...");
    Rotary_Init();
    // Bluetooth_Init();
    tft180_show_string(Row_0, Line_3, "Rotary Init ...");
    Manipulator_Init();
    Motor_Init();
    tft180_show_string(Row_0, Line_4, "Motor Init ...");
    Beep_Init();
    // dl1a_init();
    Flash_Init();
    All_PID_Init();
    Pid_Init();
    UART_Init();
    TIM_Init();
    My_FSM_Init();
    tft180_show_string(Row_0, Line_5, "Soft Init ...");
    tft180_clear();
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
    // Start作为开始标志位其为1表示小车开始行进,否则继续显示菜单界面
    if (Start == 1)
    {
        if (mt9v03x_finish_flag)
        {
            Image_Process();
            mt9v03x_finish_flag = 0;
        }
        tft180_show_gray_image(0, 0, (const uint8 *)(Original_Image), MT9V03X_W, MT9V03X_H, (Row_18), (Line_5), Image_Thereshold);
        FSMRun(CURRENT_FSM);
    }
    else
    {
        Menu_Display();
    }
}