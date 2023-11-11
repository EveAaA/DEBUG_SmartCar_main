/**
  ******************************************************************************
  * @file    Car_Control.c
  * @author  ׯ�ı�
  * @brief   С����̬����
  * @date    11/5/2023
    @verbatim
    ��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Car_Control.h"
#include "PID.h"
#include "Motor.h"
#include "Encoder.h"

/* Define\Declare ------------------------------------------------------------*/
Incremental_PID_TypeDef LMotor_F_Speed;
Incremental_PID_TypeDef RMotor_F_Speed;
Incremental_PID_TypeDef LMotor_B_Speed;
Incremental_PID_TypeDef RMotor_B_Speed;

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ����PID������ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void All_PID_Init()
{
    //�ٶȻ�
    Incremental_PID_Init(&LMotor_F_Speed,0.6,0.2,0.4,40,-40);
    Incremental_PID_Init(&RMotor_F_Speed,0.5,0.25,0.35,40,-40);
    Incremental_PID_Init(&LMotor_B_Speed,0.7,0.3,0.5,40,-40);
    Incremental_PID_Init(&RMotor_B_Speed,0.5,0.35,0.5,40,-40);
}

void Set_Car_Speed(double Speed_X,double Speed_Y,double Speed_Z)
{
    double LF_Speed,LB_Speed,RF_Spped,RB_Speed;

//�˶�����
    LF_Speed = Speed_Y + Speed_X + Speed_Z;
    LB_Speed = Speed_Y - Speed_X + Speed_Z;
    RF_Spped = Speed_Y - Speed_X - Speed_Z;
    RB_Speed = Speed_Y + Speed_X - Speed_Z;

//�ٶȻ�
    Set_Motor_Speed(LMotor_F,Get_Incremental_PID_Value(&LMotor_F_Speed,LF_Speed-Get_LF_Speed()));
    Set_Motor_Speed(LMotor_B,Get_Incremental_PID_Value(&LMotor_B_Speed,LB_Speed-Get_LB_Speed()));
    Set_Motor_Speed(RMotor_F,Get_Incremental_PID_Value(&RMotor_F_Speed,RF_Spped-Get_RF_Speed()));
    Set_Motor_Speed(RMotor_B,Get_Incremental_PID_Value(&RMotor_B_Speed,RB_Speed-Get_RB_Speed()));
}