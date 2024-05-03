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
#define Motor_LF PWM2_MODULE2_CHB_C11 
#define Motor_RF PWM1_MODULE0_CHA_D12//ǰ��
#define Motor_RB PWM4_MODULE2_CHA_C30 //
#define Motor_LB PWM1_MODULE1_CHB_D15 
#define Motor_Dir_LF C31
#define Motor_Dir_RF D13
#define Motor_Dir_LB D14 
#define Motor_Dir_RB C27

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief     �����ʼ��
-- @param     ��
-- @author    ׯ�ı�
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
-- @author    ׯ�ı�
-- @date      2023/11/2
**/
void Set_Motor_Speed(MotorHandle Motor, float PWMDuty)
{
    
    int Forward = 0;
    float Pwm_Temp = 0;
    float Pwm_Set = 0;
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
                pwm_set_duty(Motor_LF,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_LF,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_LF,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_LF,1);
            }
        break;
        case RMotor_B:
            if(Forward)
            {
                pwm_set_duty(Motor_RB,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_RB,1);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_RB,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_RB,0);
            }
        break;
        case RMotor_F:
            if(Forward)
            {
                pwm_set_duty(Motor_RF,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_RF,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_RF,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_RF,1);
            }
        break;
        case LMotor_B:
            if(Forward)
            {
                pwm_set_duty(Motor_LB,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_LB,0);
            }
            else if(!Forward)
            {
                pwm_set_duty(Motor_LB,(uint32)Pwm_Set);
                gpio_set_level(Motor_Dir_LB,1);
            }
        break;
        default:
        break;
    }
}

/**@brief     ���ʧ��
-- @param     ��
-- @author    ׯ�ı�
-- @date      2024/3/13
**/
void Motor_Disable()
{
    Set_Motor_Speed(LMotor_F,0);
    Set_Motor_Speed(LMotor_B,0);
    Set_Motor_Speed(RMotor_F,0);
    Set_Motor_Speed(RMotor_B,0);
    // printf("Motor_Disable\r\n");
}
