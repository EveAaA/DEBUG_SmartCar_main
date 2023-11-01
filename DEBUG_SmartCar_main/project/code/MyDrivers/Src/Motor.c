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
#include "zf_common_headfile.h"

/* Define\Declare ------------------------------------------------------------*/
#define PWM_Freq 17000
#define Motor_LF_1 PWM1_MODULE0_CHB_D13//ǰ��
#define Motor_LF_2 PWM1_MODULE1_CHB_D15
#define Motor_RF_1 PWM1_MODULE1_CHA_D14//ǰ��
#define Motor_RF_2 PWM1_MODULE0_CHA_D12
#define Motor_LB_1 PWM1_MODULE3_CHA_D0 //ǰ��
#define Motor_LB_2 PWM2_MODULE3_CHA_D2
#define Motor_RB_1 PWM2_MODULE3_CHB_D3 //ǰ��
#define Motor_RB_2 PWM1_MODULE3_CHB_D1

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
    pwm_init(Motor_LF_1,PWM_Freq,0);
    pwm_init(Motor_LF_2,PWM_Freq,0);
    pwm_init(Motor_RF_1,PWM_Freq,0);
    pwm_init(Motor_RF_2,PWM_Freq,0);
    //�����������
    pwm_init(Motor_LB_1,PWM_Freq,0);
    pwm_init(Motor_LB_2,PWM_Freq,0);
    pwm_init(Motor_RB_1,PWM_Freq,0);
    pwm_init(Motor_RB_2,PWM_Freq,0);
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
                pwm_set_duty(Motor_LF_1,Pwm_Set);
                pwm_set_duty(Motor_LF_2,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_LF_1,0);
                pwm_set_duty(Motor_LF_2,Pwm_Set);
            }
        break;
        case RMotor_B:
            if(Forward)
            {
                pwm_set_duty(Motor_LB_1,Pwm_Set);
                pwm_set_duty(Motor_LB_2,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_LB_1,0);
                pwm_set_duty(Motor_LB_2,Pwm_Set);
            }
        break;
        case RMotor_F:
            if(Forward)
            {
                pwm_set_duty(Motor_RF_1,Pwm_Set);
                pwm_set_duty(Motor_RF_2,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_RF_1,0);
                pwm_set_duty(Motor_RF_2,Pwm_Set);
            }
        break;
        case LMotor_B:
            if(Forward)
            {
                pwm_set_duty(Motor_RB_1,Pwm_Set);
                pwm_set_duty(Motor_RB_2,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_RB_1,0);
                pwm_set_duty(Motor_RB_2,Pwm_Set);
            }
        break;
        default:
        break;
    }
}