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
Navigation_Handle Navigation = {0,0,0,0,0,0,0,0,0};
Pid_TypeDef Distance_PID;
Pid_TypeDef Angle_PID;

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

void Pid_Init()
{
  PIDInit(&Distance_PID,3,0,0,3,-3);
  PIDInit(&Angle_PID,0.44,0,2,1.5,-1.5);
}

/**@brief   ���Ե���ʹ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/12
**/
void Enable_Navigation()
{
  if(Navigation.Start_Flag == 0)
  {
    Navigation.Start_Position_X = 0;//�������λ��
    Navigation.Start_Position_Y = 0;
    Navigation.Cur_Position_X = 0;//��ǰλ�ó�ʼ��
    Navigation.Cur_Position_Y = 0;
    Navigation.Start_Angle = Gyro_YawAngle_Get();//��ǰ�Ƕ�
    Navigation.Start_Flag = 1;//�������Ե���
  }
}

/**@brief   ���Ե�������
-- @param   double x Ŀ��X����
-- @param   double y Ŀ��Y����
-- @auther  ׯ�ı�
-- @date    2023/12/12
**/
void Navigation_Process(double x,double y)
{
    static double X_Output = 0;//PID���
    static double Y_Output = 0;
    static double Z_Output = 0;

    Enable_Navigation();//�������Ե��������õ�ǰ��Ϣ
    Navigation.Cur_Angle = Navigation.Start_Angle - Gyro_YawAngle_Get();//��ȡ����Ƕ�
    // Navigation.Cur_Position_X = sin(Navigation.Cur_Angle / 180 * 3.14159f) * Get_X_Distance();
    Navigation.Cur_Position_X = Get_X_Distance();//��ȡ��������ֵ
    Navigation.Cur_Position_Y = Get_Y_Distance();
    Navigation.Target_Position_X = x;//����Ŀ��� 
    Navigation.Target_Position_Y = y;//����Ŀ���

    X_Output = GetPIDValue(&Distance_PID,(Navigation.Target_Position_X - Navigation.Cur_Position_X));//����PID���
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
      Set_Car_Speed(X_Output,Y_Output,Z_Output);//�����ٶ�
    }
    
} 