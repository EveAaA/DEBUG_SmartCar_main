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
Navigation_Handle Navigation = {0,0,0,0,0,0,0,0,0,0,0};
Pid_TypeDef DistanceX_PID = 
{
    .Kp = 0.2f,
    .Ki = 0.0f,
    .Kd = 5,
    .OutputMax = 5,
    .OutputMin = -5,
};

Pid_TypeDef DistanceX_PID_M = 
{
    .Kp = 0.5f,
    .Ki = 0.0f,
    .Kd = 5,
    .OutputMax = 5,
    .OutputMin = -5,
};

Pid_TypeDef DistanceY_PID = 
{
    .Kp = 0.2f,
    .Ki = 0.0f,
    .Kd = 3,
    .OutputMax = 5,
    .OutputMin = -5,
};

Pid_TypeDef DistanceY_PID_M = 
{
    .Kp = 0.5f,
    .Ki = 0.0f,
    .Kd = 5,
    .OutputMax = 5,
    .OutputMin = -5,
};

State Navigation_State = 0;
float Basic_Speed = 1.5f;
float Large_Speed = 2.5f;
/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   ���Ե���ʹ��
-- @param   ��
-- @author  ׯ�ı�
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
-- @author  ׯ�ı�
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
        Navigation_State = Start_State;//״̬����
    }
}

/**@brief   ���Ե�������
-- @param   float x Ŀ��X����
-- @param   float y Ŀ��Y����
-- @author  ׯ�ı�
-- @date    2024/4/3
**/
void Navigation_Process(float x,float y)
{
    switch(Navigation_State)
    {
        case Start_State:
            Car.Speed_X = 0;//��ͣһ��
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
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
                Navigation_State = Y_State;
                Navigation.End_Angle = Gyro_YawAngle_Get();
            }

            if(x - Navigation.Cur_Position_X >= 0)
            {
                if(x - Navigation.Cur_Position_X > 5)
                {
                    Car.Speed_X = Large_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X));
                }
                else if(x - Navigation.Cur_Position_X <= 5)
                {
                    Car.Speed_X = Basic_Speed + GetPIDValue(&DistanceX_PID_M,(x - Navigation.Cur_Position_X));
                }
            }
            else
            {
                if(x - Navigation.Cur_Position_X < -5)
                {
                    Car.Speed_X = -Large_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X));
                }
                else if(x - Navigation.Cur_Position_X > -5)
                {
                    Car.Speed_X = -Basic_Speed + GetPIDValue(&DistanceX_PID_M,(x - Navigation.Cur_Position_X));
                }
            }
            Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
        break;
        case Y_State:
            Car.Speed_X = 0;
            Navigation.Cur_Position_Y = Get_Y_Distance();
            if(fabs(y - Navigation.Cur_Position_Y) <= 1.0f)
            {
                Navigation_State = Move_Finish;
            }
            if(y - Navigation.Cur_Position_Y >= 0)
            {
                if(y - Navigation.Cur_Position_Y > 5)
                {
                    Car.Speed_Y = Large_Speed + GetPIDValue(&DistanceY_PID,(y - Navigation.Cur_Position_Y));
                }
                else if(y - Navigation.Cur_Position_Y <= 5)
                {
                    Car.Speed_Y = Basic_Speed + GetPIDValue(&DistanceY_PID_M,(y - Navigation.Cur_Position_Y));
                }
            }
            else
            {
                if(y - Navigation.Cur_Position_Y < -5)
                {
                    Car.Speed_Y = -Large_Speed + GetPIDValue(&DistanceY_PID,(y - Navigation.Cur_Position_Y));
                }
                else if(y - Navigation.Cur_Position_Y > -5)
                {
                    Car.Speed_Y = -Basic_Speed + GetPIDValue(&DistanceY_PID_M,(y - Navigation.Cur_Position_Y));
                }
            }
        break;
        case Move_Finish:
            Navigation.Finish_Flag = true;//һ�ι��Ե������
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(Navigation.End_Angle);
            DistanceX_PID_M.I_Out = 0;
            DistanceY_PID_M.I_Out = 0;
            Reset_Navigation();
        break;
    }
} 

