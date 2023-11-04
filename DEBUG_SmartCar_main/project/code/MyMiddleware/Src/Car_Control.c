/**
  ******************************************************************************
  * @file    Car_Control.c
  * @author  庄文标
  * @brief   小车姿态控制
  *
    @verbatim
    无
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
 *  @defgroup 外部调用
 *  @brief
 *
**/
void All_PID_Init()
{
    //速度环
    Incremental_PID_Init(&LMotor_F_Speed,0.65,0.52,0,40,-40);
    Incremental_PID_Init(&RMotor_F_Speed,0,0.01,0,40,-40);//有问题
    Incremental_PID_Init(&LMotor_B_Speed,0,0.3,0,40,-40);
    Incremental_PID_Init(&RMotor_B_Speed,0.5,0.45,0,40,-40);
}

void Set_Car_Speed(double Target_Speed)
{
    Set_Motor_Speed(RMotor_B,Get_Incremental_PID_Value(&RMotor_B_Speed,Target_Speed-Get_RB_Speed()));
    Set_Motor_Speed(LMotor_F,Get_Incremental_PID_Value(&LMotor_F_Speed,Target_Speed-Get_LF_Speed()));
}