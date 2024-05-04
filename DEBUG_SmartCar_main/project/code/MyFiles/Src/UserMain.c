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
#include "User_FSM.h"
#include "My_FSM.h"
/* Define\Declare ------------------------------------------------------------*/
uint16 Start = 2;
uint8 Once = 1;

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
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
        system_delay_ms(1000); // ���Ʊ�ʾ�쳣
    }
    tft180_show_string(Row_0, Line_1, "Mt9v03x Init ...");
}

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
 **/

/**@brief   ���г�ʼ������
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/4
**/
void User_Init()
{
    system_delay_ms(100);//��ʱһ��ȴ����������ϵ�
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

/**@brief   ������ѭ������
-- @param   ��
-- @author  ׯ�ı�
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
    //     if(Servo_Flag.Put_Up == 0)//��Ƭ������
    //     {
    //         Pick_Card();
    //     }
    //     Servo_Flag.Put_Depot = false;   
    // }
    // else if(Receivedata.Start_Flag == 0)
    // {
    //     if(Servo_Flag.Put_Depot == false)//�Ž��ֿ�
    //     {
    //         Put_Depot();
    //     }
    //     Servo_Flag.Put_Down = false;
    //     Servo_Flag.Put_Up = false;
    //     Servo_Flag.Put_Out = false;
    // }
    // else if(Receivedata.Start_Flag == 3)
    // {
    //     if(Servo_Flag.Put_Out == false)//��Ƭ�ó���
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