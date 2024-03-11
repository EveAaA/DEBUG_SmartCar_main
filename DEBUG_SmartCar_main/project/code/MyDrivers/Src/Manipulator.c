/**
  ******************************************************************************
  * @file    Manipulator.c
  * @author  ׯ�ı�
  * @brief   ��е������
  * @date    11/5/2023
    @verbatim
    ������������͵����
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Manipulator.h"

/* Define\Declare ------------------------------------------------------------*/
#define Servo_FREQ 330

#define Set_180Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 90.0))//����180�ȶ���Ƕ�תΪ����ռ�ձ�
#define Set_360Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 180.0))//����360�ȶ���Ƕ�תΪ����ռ�ձ�

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
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��е�۳�ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void Manipulator_Init()
{
    pwm_init(Stretch_Servo.Pin,Servo_FREQ,Stretch_Servo.Init_Angle);
    pwm_init(Raise_Servo.Pin,Servo_FREQ,Raise_Servo.Init_Angle);
}

/**@brief   ���ö���Ƕ�,180�ȶ��
-- @param   ServoHandle Servo ѡ����
-- @param   float Angle ���õĽǶ�
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void Set_Servo_Angle(Servo_Handle Servo,float Angle)
{
    if(Angle > Servo.Max_Angle)//�޷�
    {
        Servo.Set_Angle = Servo.Max_Angle;
    }
    else if(Angle < Servo.Min_Angle)
    {
        Servo.Set_Angle = Servo.Min_Angle;
    }

    pwm_set_duty(Servo.Pin,Set_180Servo_Angle(Servo.Set_Angle));
}
