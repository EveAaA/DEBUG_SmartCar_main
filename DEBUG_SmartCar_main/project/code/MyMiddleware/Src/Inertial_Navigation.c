/**
  ******************************************************************************
  * @file    Inertial_Navigation.c
  * @author  ׯ�ı�
  * @brief   ���Ե���
  * @date    12/12/2023
    @verbatim
    ��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Inertial_Navigation.h"
#include "UserMain.h"
#include "math.h"

/* Define\Declare ------------------------------------------------------------*/
Navigation_Handle Navigation = {0,0,0,0,0,0,0,0,0,0};
Pid_TypeDef DistanceX_PID = 
{
    .Kp = 0.6f,
    .Ki = 0,
    .Kd = 0,
    .OutputMax = 3,
    .OutputMin = -3,
};

Pid_TypeDef DistanceY_PID = 
{
    .Kp = 0.3f,
    .Ki = 0,
    .Kd = 0,
    .OutputMax = 3,
    .OutputMin = -3,
};

State Navigation_State = 0;
float Basic_Speed = 0;

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   ���Ե���ʹ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/12
**/
static void Enable_Navigation()
{
    if(Navigation.Start_Flag == false)
    {
        Navigation.Start_Position_X = 0;//�������λ��
        Navigation.Start_Position_Y = 0;
        Navigation.Start_Angle = Gyro_YawAngle_Get();//��ǰ�Ƕ� 
        Navigation.Start_Flag = true;//�������Ե���
        Navigation.Finish_Flag = false;
    }
}


/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ���Ե�������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/4/3
**/
void Reset_Navigation()
{
    if(Navigation.Start_Flag == true)
    {
        Navigation.Start_Position_X = 0;//�������λ��
        Navigation.Start_Position_Y = 0;
        Navigation.Start_Angle = 0;//��ǰ�Ƕ�
        Navigation.Cur_Position_X = 0;
        Navigation.Cur_Position_Y = 0;
        Navigation.Start_Flag = false;//�رչ��Ե���
        Navigation.Finish_Flag = false;
        Navigation_State = Start_State;//״̬����
    }
}

/**@brief   ���Ե�������
-- @param   float x Ŀ��X����
-- @param   float y Ŀ��Y����
-- @auther  ׯ�ı�
-- @date    2024/4/3
**/
void Navigation_Process(float x,float y)
{
    switch(Navigation_State)
    {
        case Start_State:
            Set_Car_Speed(0,0,0);//�Ⱦ�ֹһ��
            if(fabs(Get_X_Speed()) <= 0.1 && fabs(Get_Y_Speed()) <= 0.1)
            {
                Navigation_State = X_State;
                Enable_Navigation();
            }
        break;
        case X_State:
            Navigation.Cur_Position_X = Get_X_Distance();

            if(fabs(x - Navigation.Cur_Position_X) <= 1.0f)
            {
                Navigation_State = Stop;
            }

            if(x >= 0)
            {
                Set_Car_Speed(Basic_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X)),0,Angle_Control(Navigation.Start_Angle));
            }
            else
            {
                Set_Car_Speed(-Basic_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X)),0,Angle_Control(Navigation.Start_Angle));
            }
        break;
        case Stop:
            Set_Car_Speed(0,0,Angle_Control(Navigation.Start_Angle));//�Ⱦ�ֹһ��
            if(fabs(Get_X_Speed()) <= 0.1 && fabs(Get_Y_Speed()) <= 0.1)
            {
                Navigation_State = Y_State;
            }
        break;
        case Y_State:
            Navigation.Cur_Position_Y = Get_Y_Distance();

            if(fabs(y - Navigation.Cur_Position_Y) <= 1.0f)
            {
                Navigation_State = Move_Finish;
            }

            if(y >= 0)
            {
                Set_Car_Speed(0,Basic_Speed + GetPIDValue(&DistanceY_PID,(y - Navigation.Cur_Position_Y)),Angle_Control(Navigation.Start_Angle));
            }
            else
            {
                Set_Car_Speed(0,-Basic_Speed + GetPIDValue(&DistanceY_PID,(y - Navigation.Cur_Position_Y)),Angle_Control(Navigation.Start_Angle));
            }
        break;
        case Move_Finish:
            Set_Car_Speed(0,0,Angle_Control(Navigation.Start_Angle));
            Navigation.Finish_Flag = true;//һ�ι��Ե������
        break;
    }
} 
