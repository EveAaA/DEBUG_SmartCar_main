/**
  ******************************************************************************
  * @file    Manipulator.c
  * @author  庄文标
  * @brief   机械臂驱动
  * @date    11/5/2023
    @verbatim
    包含两个舵机和电磁铁
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Manipulator.h"

/* Define\Declare ------------------------------------------------------------*/
#define Servo_FREQ 330

#define Set_180Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 90.0))//设置180度舵机角度转为具体占空比
#define Set_360Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 180.0))//设置360度舵机角度转为具体占空比

Servo_Handle Stretch_Servo = 
{
    .Pin = PWM2_MODULE0_CHA_C6,
    .Init_Angle = Set_180Servo_Angle(100),
};
Servo_Handle Raise_Servo = 
{
    .Pin = PWM1_MODULE3_CHB_B11,
    .Init_Angle = Set_180Servo_Angle(100),
};

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   机械臂初始化
-- @param   无
-- @auther  庄文标
-- @date    2023/11/5
**/
void Manipulator_Init()
{
    pwm_init(Stretch_Servo.Pin,Servo_FREQ,Stretch_Servo.Init_Angle);
    pwm_init(Raise_Servo.Pin,Servo_FREQ,Raise_Servo.Init_Angle);
}

/**@brief   设置舵机角度,180度舵机
-- @param   ServoHandle Servo 选择舵机
-- @param   float Angle 设置的角度
-- @auther  庄文标
-- @date    2023/11/5
**/
void Set_Servo_Angle(Servo_Handle Servo,float Angle)
{
    if(Angle > Servo.Max_Angle)//限幅
    {
        Servo.Set_Angle = Servo.Max_Angle;
    }
    else if(Angle < Servo.Min_Angle)
    {
        Servo.Set_Angle = Servo.Min_Angle;
    }

    pwm_set_duty(Servo.Pin,Set_180Servo_Angle(Servo.Set_Angle));
}
