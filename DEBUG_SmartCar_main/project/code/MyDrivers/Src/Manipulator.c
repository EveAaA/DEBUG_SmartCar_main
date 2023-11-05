/**
  ******************************************************************************
  * @file    Manipulator.c
  * @author  庄文标
  * @brief   按键驱动
  * @date    11/5/2023
    @verbatim
    包含两个舵机和电磁铁
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Manipulator.h"

/* Define\Declare ------------------------------------------------------------*/
#define Servo_FREQ 50
#define Up_Servo_PWM PWM4_MODULE2_CHA_C30
#define Down_Servo_PWM PWM4_MODULE3_CHA_C31
#define Solenoid C26

#define Up_Servo_Init ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(90) / 90.0))
#define Dowm_Servo_Init ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(0) / 90.0))
/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   所有按键初始化
-- @param   无
-- @auther  庄文标
-- @date    2023/11/5
**/
void Manipulator_Init()
{
    pwm_init(Up_Servo_PWM,Servo_FREQ,Up_Servo_Init);
    pwm_init(Down_Servo_PWM,Servo_FREQ,Dowm_Servo_Init);
    gpio_init(Solenoid,GPO,0,GPO_PUSH_PULL);
}

/**@brief   设置舵机角度
-- @param   ServoHandle Servo 选择舵机
-- @param   double Angle 设置的角度
-- @auther  庄文标
-- @date    2023/11/5
**/
void Set_Servo_Angle(ServoHandle Servo,double Angle)
{
    double Servo_PWM = ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(Angle) / 90.0));

    switch (Servo)
    {
        case Up_Servo:
            pwm_set_duty(Up_Servo_PWM,Servo_PWM);
        break;
        case Down_Servo:
            pwm_set_duty(Down_Servo_PWM,Servo_PWM);
        break;
    }
}
