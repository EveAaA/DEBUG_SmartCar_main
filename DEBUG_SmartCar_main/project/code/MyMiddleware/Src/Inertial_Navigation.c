/**
  ******************************************************************************
  * @file    Inertial_Navigation.c
  * @author  庄文标
  * @brief   惯性导航
  * @date    12/12/2023
    @verbatim
    无
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
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   惯性导航使能
-- @param   无
-- @auther  庄文标
-- @date    2023/12/12
**/
static void Enable_Navigation()
{
    if(Navigation.Start_Flag == false)
    {
        Navigation.Start_Position_X = 0;//更新起点位置
        Navigation.Start_Position_Y = 0;
        Navigation.Start_Angle = Gyro_YawAngle_Get();//当前角度 
        Navigation.Start_Flag = true;//开启惯性导航
        Navigation.Finish_Flag = false;
    }
}


/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   惯性导航重置
-- @param   无
-- @auther  庄文标
-- @date    2024/4/3
**/
void Reset_Navigation()
{
    if(Navigation.Start_Flag == true)
    {
        Navigation.Start_Position_X = 0;//更新起点位置
        Navigation.Start_Position_Y = 0;
        Navigation.Start_Angle = 0;//当前角度
        Navigation.Cur_Position_X = 0;
        Navigation.Cur_Position_Y = 0;
        Navigation.Start_Flag = false;//关闭惯性导航
        Navigation.Finish_Flag = false;
        Navigation_State = Start_State;//状态清零
    }
}

/**@brief   惯性导航进程
-- @param   float x 目标X坐标
-- @param   float y 目标Y坐标
-- @auther  庄文标
-- @date    2024/4/3
**/
void Navigation_Process(float x,float y)
{
    switch(Navigation_State)
    {
        case Start_State:
            Car.Speed_X = 0;//先停一会
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
            Navigation.Cur_Position_Y = Get_Y_Distance();
            if(fabs(x - Navigation.Cur_Position_X) <= 1.0f && fabs(y - Navigation.Cur_Position_Y) <= 1.0f)
            {
                Navigation_State = Move_Finish;
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
            Car.Speed_Z = Angle_Control(0);
        break;
        case Move_Finish:
            Car.Speed_X = 0;//先停一会
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(0);
            DistanceX_PID_M.I_Out = 0;
            DistanceY_PID_M.I_Out = 0;
            Navigation.Finish_Flag = true;//一次惯性导航完成
        break;
    }
} 
// void Navigation_Process(float x,float y)
// {
//     switch(Navigation_State)
//     {
//         case Start_State:
//             Car.Speed_X = 0;//先停一会
//             Car.Speed_Y = 0;
//             Car.Speed_Z = 0;
//             if(fabs(Get_X_Speed()) <= 0.1 && fabs(Get_Y_Speed()) <= 0.1)
//             {
//                 Navigation_State = X_State;
//                 Enable_Navigation();
//             }
//         break;
//         case X_State:
//             Navigation.Cur_Position_X = Get_X_Distance();
//             Car.Speed_Y = 0;
//             if(fabs(x - Navigation.Cur_Position_X) <= 1.0f)
//             {
//                 Navigation_State = Stop;
//             }

//             if(x >= 0)
//             {
//                 Car.Speed_X = Basic_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X));//先停一会
//                 //Set_Car_Speed(Basic_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X)),0,Angle_Control(Navigation.Start_Angle));
//             }
//             else
//             {
//                 Car.Speed_X = -Basic_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X));
//                 //Set_Car_Speed(-Basic_Speed + GetPIDValue(&DistanceX_PID,(x - Navigation.Cur_Position_X)),0,Angle_Control(Navigation.Start_Angle));
//             }
//             Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
//         break;
//         case Stop:
//             Car.Speed_X = 0;//先停一会
//             Car.Speed_Y = 0;
//             if(fabs(Get_X_Speed()) <= 0.1 && fabs(Get_Y_Speed()) <= 0.1)
//             {
//                 Navigation_State = Y_State;
//             }
//         break;
//         case Y_State:
//             Navigation.Cur_Position_Y = Get_Y_Distance();
//             Car.Speed_X = 0;
//             if(fabs(y - Navigation.Cur_Position_Y) <= 1.0f)
//             {
//                 Navigation_State = Move_Finish;
//             }

//             if(y >= 0)
//             {
//                 Car.Speed_Y = Basic_Speed + GetPIDValue(&DistanceY_PID,(y - Navigation.Cur_Position_Y));
//             }
//             else
//             {
//                 Car.Speed_Y = -Basic_Speed + GetPIDValue(&DistanceY_PID,(y - Navigation.Cur_Position_Y));
//             }
//             Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
//         break;
//         case Move_Finish:
//             Car.Speed_X = 0;//先停一会
//             Car.Speed_Y = 0;
//             Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
//             Navigation.Finish_Flag = true;//一次惯性导航完成
//         break;
//     }
// } 
