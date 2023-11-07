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
    Incremental_PID_Init(&LMotor_F_Speed, LF_Parameter[KP], LF_Parameter[KI], 0, 40, -40);            // kp: 0.65 ki: 0.52 
    Incremental_PID_Init(&RMotor_F_Speed, RF_Parameter[KP], RF_Parameter[KI], 0, 40, -40);//������     // kp: 0.00 ki: 0.01
    Incremental_PID_Init(&LMotor_B_Speed, LB_Parameter[KP], LB_Parameter[KI], 0, 40, -40);             // kp: 0.00 ki: 0.01
    Incremental_PID_Init(&RMotor_B_Speed, RB_Parameter[KP], RB_Parameter[KI], 0, 40, -40);             // kp: 0.50 ki: 0.45
}

void Set_Car_Speed(double Target_Speed)
{
    Set_Motor_Speed(RMotor_B,Get_Incremental_PID_Value(&RMotor_B_Speed,Target_Speed-Get_RB_Speed()));
    Set_Motor_Speed(LMotor_F,Get_Incremental_PID_Value(&LMotor_F_Speed,Target_Speed-Get_LF_Speed()));
}