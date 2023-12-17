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
Navigation_Handle Navigation = {0,0,0,0,0,0,0,0,0};
Pid_TypeDef Distance_PID;
Pid_TypeDef Angle_PID;

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

void Pid_Init()
{
  PIDInit(&Distance_PID,3,0,0,3,-3);
  PIDInit(&Angle_PID,0.44,0,2,1.5,-1.5);
}

/**@brief   惯性导航使能
-- @param   无
-- @auther  庄文标
-- @date    2023/12/12
**/
void Enable_Navigation()
{
  if(Navigation.Start_Flag == 0)
  {
    Navigation.Start_Position_X = 0;//更新起点位置
    Navigation.Start_Position_Y = 0;
    Navigation.Cur_Position_X = 0;//当前位置初始化
    Navigation.Cur_Position_Y = 0;
    Navigation.Start_Angle = Gyro_YawAngle_Get();//当前角度
    Navigation.Start_Flag = 1;//开启惯性导航
  }
}

/**@brief   惯性导航进程
-- @param   double x 目标X坐标
-- @param   double y 目标Y坐标
-- @auther  庄文标
-- @date    2023/12/12
**/
void Navigation_Process(double x,double y)
{
    static double X_Output = 0;//PID输出
    static double Y_Output = 0;
    static double Z_Output = 0;

    Enable_Navigation();//开启惯性导航，设置当前信息
    Navigation.Cur_Angle = Navigation.Start_Angle - Gyro_YawAngle_Get();//获取自身角度
    // Navigation.Cur_Position_X = sin(Navigation.Cur_Angle / 180 * 3.14159f) * Get_X_Distance();
    Navigation.Cur_Position_X = Get_X_Distance();//获取自身坐标值
    Navigation.Cur_Position_Y = Get_Y_Distance();
    Navigation.Target_Position_X = x;//设置目标点 
    Navigation.Target_Position_Y = y;//设置目标点

    X_Output = GetPIDValue(&Distance_PID,(Navigation.Target_Position_X - Navigation.Cur_Position_X));//计算PID输出
    Y_Output = GetPIDValue(&Distance_PID,(Navigation.Target_Position_Y - Navigation.Cur_Position_Y));
    Z_Output = GetPIDValue(&Angle_PID,Navigation.Cur_Angle);
    printf("X = %lf\r\n",X_Output);
    printf("Y = %lf\r\n",Y_Output);
    printf("Z = %lf\r\n",Z_Output);
    if((Navigation.Target_Position_X - Navigation.Cur_Position_X) < 0.5 && (Navigation.Target_Position_Y - Navigation.Cur_Position_Y) < 0.5)
    {
      Set_Car_Speed(0,0,Z_Output);
    }
    else
    {
      Set_Car_Speed(X_Output,Y_Output,Z_Output);//设置速度
    }
    
} 