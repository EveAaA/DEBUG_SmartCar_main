/**
  ******************************************************************************
  * @file    Motor.c
  * @author  ׯ�ı�
  * @brief   �������
  * @date    2/11/2023
    @verbatim

    ��·�������

    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Motor.h"

/* Define\Declare ------------------------------------------------------------*/
#define PWM_Freq 17000
#define Motor_LF PWM1_MODULE0_CHA_D12 
#define Motor_RF PWM1_MODULE0_CHB_D13//ǰ��
#define Motor_RB PWM1_MODULE1_CHA_D14 //
#define Motor_LB PWM1_MODULE1_CHB_D15 
#define Motor_Dir_LF D27
#define Motor_Dir_RF C27
#define Motor_Dir_LB B23 
#define Motor_Dir_RB D4

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief     �����ʼ��
-- @param     ��
-- @auther    ׯ�ı�
-- @date      2023/11/2
**/
void Motor_Init()
{
    //ǰ���������
    pwm_init(Motor_LF,PWM_Freq,0);
    pwm_init(Motor_LB,PWM_Freq,0);
    pwm_init(Motor_RF,PWM_Freq,0);
    pwm_init(Motor_RB,PWM_Freq,0);

    gpio_init(Motor_Dir_LF,GPO,1,GPO_PUSH_PULL);
    gpio_init(Motor_Dir_RF,GPO,1,GPO_PUSH_PULL);
    gpio_init(Motor_Dir_LB,GPO,1,GPO_PUSH_PULL);
    gpio_init(Motor_Dir_RB,GPO,1,GPO_PUSH_PULL);
}

/**@brief     ����ٶ�����
-- @param     MotorHandle Motor ѡ�����ҵ��
-- @param     uint16 PWMDuty PWM��ռ�ձȣ���Ӧ0-100���ٶ�
-- @auther    ׯ�ı�
-- @date      2023/11/2
**/
void Set_Motor_Speed(MotorHandle Motor, float PWMDuty)
{
    
    int Forward = 0;
    double Pwm_Temp = 0;
    double Pwm_Set = 0;
    //�ж������PWM������
    if(PWMDuty < 0)
    {
        Pwm_Temp = -PWMDuty;
        Forward = 0;
    }
    else if(PWMDuty >= 0)
    {
        Pwm_Temp = PWMDuty;
        Forward = 1;
    }

    Pwm_Set = Pwm_Temp *100.0;//��һ��

    if(Pwm_Set > 10000)//�޷�
    {
        Pwm_Set = 9999;
    }

    switch (Motor)
    {
        case LMotor_F:
            if(Forward)
            {
                pwm_set_duty(Motor_LF,Pwm_Set);
                gpio_set_level(Motor_Dir_LF,1);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_LF,Pwm_Set);
                gpio_set_level(Motor_Dir_LF,0);
            }
        break;
        case RMotor_B:
            if(Forward)
            {
                pwm_set_duty(Motor_RB,Pwm_Set);
                gpio_set_level(Motor_Dir_RB,1);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_RB,Pwm_Set);
                gpio_set_level(Motor_Dir_RB,0);
            }
        break;
        case RMotor_F:
            if(Forward)
            {
                pwm_set_duty(Motor_RF,Pwm_Set);
                gpio_set_level(Motor_Dir_RF,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_RF,Pwm_Set);
                gpio_set_level(Motor_Dir_RF,1);
            }
        break;
        case LMotor_B:
            if(Forward)
            {
                pwm_set_duty(Motor_LB,Pwm_Set);
                gpio_set_level(Motor_Dir_LB,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_LB,Pwm_Set);
                gpio_set_level(Motor_Dir_LB,1);
            }
        break;
        default:
        break;
    }
}
