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
Navigation_Handle Navigation = {0,0,0,0,0,0,0,0,0,0,0};
Pid_TypeDef DistanceX_PID = 
{
    .Kp = 0.15f,
    .Ki = 0.0f,//0.005
    .Kd = 0.20f,
    .OutputMax = 5,
    .OutputMin = -5,
};

Pid_TypeDef DistanceY_PID = 
{
    .Kp = 0.03f,
    .Ki = 0.0f,
    .Kd = 0.08f,
    .OutputMax = 4,
    .OutputMin = -4,
};

Pid_TypeDef Distance_PID = 
{
    .Kp = 0.01f,
    .Ki = 0.003f,
    .Kd = 0.02f,
    .OutputMax = 4,
    .OutputMin = -4,
};

uint8 Navigation_State = 0;
float Basic_Speed = 0.0f;
float Large_Speed = 0.0f;
/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   惯性导航使能
-- @param   无
-- @author  庄文标
-- @date    2023/12/12
**/
static void Enable_Navigation()
{
    if(Navigation.Start_Flag == false)
    {
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
-- @author  庄文标
-- @date    2024/4/3
**/
void Reset_Navigation()
{
    if(Navigation.Start_Flag == true)
    {
        Navigation.Start_Angle = 0;//当前角度
        Navigation.Cur_Position_X = 0;
        Navigation.Cur_Position_Y = 0;
        Navigation.Start_Flag = false;//关闭惯性导航
        Navigation_State = Start_State;//状态清零
        Navigation.X_Finish = false;
        Navigation.Y_Finish = false;
    }
}

/**@brief   惯性导航进程
-- @param   float x 目标X坐标
-- @author  庄文标
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
            if(Bufcnt((fabs(Get_X_Speed()) <= 0.1f && fabs(Get_Y_Speed()) <= 0.1f),500))
            {
                Navigation_State = Move_State;
                Enable_Navigation();
            }
        break;
        case Move_State:
            Navigation.Cur_Position_X = Get_X_Distance();
            if(fabs(x - Navigation.Cur_Position_X) <= 1.0f)
            {
                Navigation_State = Move_State_Y;
                Distance_PID.I_Out = 0;
            }
            else
            {
                if(x - Navigation.Cur_Position_X >= 0)
                {
                    Car.Speed_X = Large_Speed + GetPIDValue(&Distance_PID,(x - Navigation.Cur_Position_X));
                }
                else
                {
                    Car.Speed_X = -Large_Speed + GetPIDValue(&Distance_PID,(x - Navigation.Cur_Position_X));
                }
            }

            Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
        break;
        case Move_State_Y:
            Navigation.Cur_Position_Y = Get_Y_Distance();
            if(fabs(y - Navigation.Cur_Position_Y) <= 1.0f)
            {
                Navigation_State = Move_Finish;
                Navigation.End_Angle = Gyro_YawAngle_Get();
            }
            else
            {
                if(y - Navigation.Cur_Position_Y >= 0)
                {
                    Car.Speed_Y = Large_Speed + GetPIDValue(&Distance_PID,(y - Navigation.Cur_Position_Y));
                }
                else
                {
                    Car.Speed_Y = -Large_Speed + GetPIDValue(&Distance_PID,(y - Navigation.Cur_Position_Y));
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
        break;
        case Move_Finish:
            Navigation.Finish_Flag = true;//一次惯性导航完成
            Distance_PID.I_Out = 0;
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
            Reset_Navigation();
        break;
    }
} 

/**@brief   惯性导航进程
-- @param   float y 目标Y坐标
-- @author  庄文标
-- @date    2024/6/2
**/
void Navigation_Process_Y(float x,float y)
{
    switch(Navigation_State)
    {
        case Start_State:
            Car.Speed_X = 0;//先停一会
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
            if(Bufcnt((fabs(Get_X_Speed()) <= 0.1 && fabs(Get_Y_Speed()) <= 0.1),500))
            {
                Navigation_State = Move_State_Y;
                Enable_Navigation();
            }
        break;
        case Move_State_Y:
            Navigation.Cur_Position_Y = Get_Y_Distance();
            printf("y:%f\r\n",Navigation.Cur_Position_Y);
            if(fabs(y - Navigation.Cur_Position_Y) <= 1.0f)
            {
                Navigation_State = Move_State;
                Navigation.End_Angle = Gyro_YawAngle_Get();
                Distance_PID.I_Out = 0;
            }
            else
            {
                if(y - Navigation.Cur_Position_Y >= 0)
                {
                    Car.Speed_Y = Large_Speed + GetPIDValue(&Distance_PID,(y - Navigation.Cur_Position_Y));
                }
                else
                {
                    Car.Speed_Y = -Large_Speed + GetPIDValue(&Distance_PID,(y - Navigation.Cur_Position_Y));
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
        break;
        case Move_State:
            Navigation.Cur_Position_X = Get_X_Distance();
            if(fabs(x - Navigation.Cur_Position_X) <= 1.0f)
            {
                Navigation_State = Move_Finish;
            }
            else
            {
                if(x - Navigation.Cur_Position_X >= 0)
                {
                    Car.Speed_X = Large_Speed + GetPIDValue(&Distance_PID,(x - Navigation.Cur_Position_X));
                }
                else
                {
                    Car.Speed_X = -Large_Speed + GetPIDValue(&Distance_PID,(x - Navigation.Cur_Position_X));
                }
            }

            Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
        break;
        case Move_Finish:
            Navigation.Finish_Flag = true;//一次惯性导航完成
            Distance_PID.I_Out = 0;
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
            Reset_Navigation();
        break;
    }
} 

/**@brief   惯性导航进程
-- @param   float x 目标X坐标
-- @param   float y 目标Y坐标
-- @author  庄文标
-- @date    2024/5/3
**/
void Navigation_Process_Image(float Target_Pos_X,float Target_Pos_Y)
{
    static uint16 Wait_Time = 0;//等待的时间
    switch(Navigation_State)
    {
        case Start_State:
            Car.Speed_X = 0;//先停一会
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
            if(fabs(Get_X_Speed()) <= 0.1 && fabs(Get_Y_Speed()) <= 0.1)
            {
                Navigation_State = Move_State;
                Enable_Navigation();
            }
        break;
        case Move_State:
            //获取图像识别的坐标
            if(fabs(FINETUNING_DATA.dx/10.0f) <= 5)
            {
                Navigation.Cur_Position_X = (FINETUNING_DATA.dx/10.0f)*0.7f+(Target_Pos_X - Get_X_Distance())*0.3f;//互补滤波
            }
            else
            {
                Navigation.Cur_Position_X = (FINETUNING_DATA.dx/10.0f)*0.3f+(Target_Pos_X - Get_X_Distance())*0.7f;//互补滤波
            }
            
            if(fabs(FINETUNING_DATA.dy/10.0f) <= 5)
            {
                Navigation.Cur_Position_Y = (FINETUNING_DATA.dy/10.0f)*0.7f+(Target_Pos_Y - Get_Y_Distance())*0.3f;//互补滤波
            }
            else
            {
                Navigation.Cur_Position_Y = (FINETUNING_DATA.dy/10.0f)*0.3f+(Target_Pos_Y - Get_Y_Distance())*0.7f;//互补滤波
            }
            
            // printf("%f,%f,%d\r\n",Navigation.Cur_Position_X,Navigation.Cur_Position_Y,0);
            // Navigation.Cur_Position_Y = FINETUNING_DATA.dy/10.f;
            if((fabs(Navigation.Cur_Position_X) <= 1.0f) && (fabs(Navigation.Cur_Position_Y) <= 1.0f) &&(fabs(Get_X_Speed()) <= 0.1) && (fabs(Get_Y_Speed()) <= 0.1))
            {
                Wait_Time += 1;
                if(Wait_Time>=18)
                {
                    Wait_Time = 0;
                    Navigation_State = Move_Finish;
                    Navigation.End_Angle = Gyro_YawAngle_Get();
                }
            }

            Car.Speed_X = GetPIDValue(&DistanceX_PID,(Navigation.Cur_Position_X));
            Car.Speed_Y = GetPIDValue(&DistanceY_PID,(Navigation.Cur_Position_Y));
            Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
        break;
        case Move_Finish:
            Navigation.Finish_Flag = true;//一次惯性导航完成
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(Navigation.End_Angle);
            Reset_Navigation();
        break;
    }
}

