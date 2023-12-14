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
#include "UserMain.h"
/* Define\Declare ------------------------------------------------------------*/
Incremental_PID_TypeDef LMotor_F_Speed;
Incremental_PID_TypeDef RMotor_F_Speed;
Incremental_PID_TypeDef LMotor_B_Speed;
Incremental_PID_TypeDef RMotor_B_Speed;
Pid_TypeDef Angle_PID;
Pid_TypeDef Image_PID;
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
    PIDInit(&Angle_PID,0.44,0,2,2,-2);
    PIDInit(&Image_PID,5,0,0,1.5,-1.5);
}


/**@brief   ���ó�����������ٶ�
-- @param   double Speed_X X���ٶȣ��Ⱥ����ٶ�
-- @param   double Speed_Y Y���ٶȣ���ǰ���ٶ�
-- @param   double Speed_Z Z���ٶȣ�����ת�ٶȣ�����Ϊ˳ʱ����ת
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void Set_Car_Speed(double Speed_X,double Speed_Y,double Speed_Z)
{
    double LF_Speed,LB_Speed,RF_Spped,RB_Speed;

//�˶�����
    LF_Speed = Speed_Y + Speed_X + Speed_Z;
    LB_Speed = Speed_Y - Speed_X + Speed_Z;
    RF_Spped = Speed_Y - Speed_X - Speed_Z;
    RB_Speed = Speed_Y + Speed_X - Speed_Z;

//�ٶȻ�
    Set_Motor_Speed(LMotor_F,Get_Incremental_PID_Value(&LMotor_F_Speed,LF_Speed-Encoer_Speed[0]));
    Set_Motor_Speed(LMotor_B,Get_Incremental_PID_Value(&LMotor_B_Speed,LB_Speed-Encoer_Speed[2]));
    Set_Motor_Speed(RMotor_F,Get_Incremental_PID_Value(&RMotor_F_Speed,RF_Spped-Encoer_Speed[1]));
    Set_Motor_Speed(RMotor_B,Get_Incremental_PID_Value(&RMotor_B_Speed,RB_Speed-Encoer_Speed[3]));
}

double Image_Erro_;
double Angle_Erro_;
void Car_run()
{
  Image_Erro_ = GetPIDValue(&Image_PID,(60 - Image_Erro)*0.01);
  // Angle_Erro_ = GetPIDValue(&Angle_PID,Gyro_YawAngle_Get() - Image_Erro);
  Set_Car_Speed(0,3,-Image_Erro_);
}