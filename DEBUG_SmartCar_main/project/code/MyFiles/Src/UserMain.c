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
#include "My_FSM.h"
/* Define\Declare ------------------------------------------------------------*/
uint16 Start = 2;
uint8 Once = 1;

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
            system_delay_ms(1000);
            // tft180_show_string(Row_0, Row_0, "IMU reinit.");
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
            system_delay_ms(1000);
            // tft180_show_string(0, 16, "mt9v03x reinit.");
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
-- @author  庄文标
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
    Bluetooth_Init();
    tft180_show_string(Row_0, Line_3, "Rotary Init ...");
    Manipulator_Init();
    Motor_Init();
    tft180_show_string(Row_0, Line_4, "Motor Init ...");
    // Beep_Init();
    // dl1a_init();
    All_PID_Init();
    Flash_Init();
    UART_Init();
    tft180_show_string(Row_0, Line_5, "Soft Init ...");
    tft180_clear();
    system_delay_ms(1000);
    TIM_Init();
    // timer_init(GPT_TIM_1, TIMER_US); 
    // Beep(On);
    // Beep(Off);
	interrupt_global_enable(0);
}

/**@brief   所有主循环内容
-- @param   无
-- @author  庄文标
-- @date    2023/9/12
**/
void User_Loop()
{
    // test_1 = 90;
    // test_2 = Gyro_YawAngle_Get();
    // Bluetooth_Send_Float(Num_Address);
    Menu_Display();
    // if(Receivedata.Start_Flag == 1)
    // {
    //     if(Servo_Flag.Put_Up == 0)//卡片拿起来
    //     {
    //         Pick_Card();
    //     }
    //     Servo_Flag.Put_Depot = false;   
    // }
    // else if(Receivedata.Start_Flag == 0)
    // {
    //     if(Servo_Flag.Put_Depot == false)//放进仓库
    //     {
    //         Put_Depot();
    //     }
    //     Servo_Flag.Put_Down = false;
    //     Servo_Flag.Put_Up = false;
    //     Servo_Flag.Put_Out = false;
    // }
    // else if(Receivedata.Start_Flag == 3)
    // {
    //     if(Servo_Flag.Put_Out == false)//卡片拿出来
    //     {
    //         Take_Card_Out();
    //     }
    //     Servo_Flag.Put_Depot = false; 
    // }
    // else if(Receivedata.Start_Flag == 4)
    // {
    //     Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle);
    //     Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle);
    //     Servo_Flag.Put_Out = false;
    // }
}