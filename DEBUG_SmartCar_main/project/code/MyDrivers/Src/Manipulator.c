/**
  ******************************************************************************
  * @file    Manipulator.c
  * @author  ׯ�ı�
  * @brief   ��������
  * @date    11/5/2023
    @verbatim
    ������������͵����
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
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ���а�����ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void Manipulator_Init()
{
    pwm_init(Up_Servo_PWM,Servo_FREQ,Up_Servo_Init);
    pwm_init(Down_Servo_PWM,Servo_FREQ,Dowm_Servo_Init);
    gpio_init(Solenoid,GPO,0,GPO_PUSH_PULL);
}

/**@brief   ���ö���Ƕ�
-- @param   ServoHandle Servo ѡ����
-- @param   double Angle ���õĽǶ�
-- @auther  ׯ�ı�
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
