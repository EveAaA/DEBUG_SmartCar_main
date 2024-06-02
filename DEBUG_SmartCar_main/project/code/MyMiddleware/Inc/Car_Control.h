#ifndef _CARCONTROL_H
#define _CARCONTROL_H

#include "zf_common_headfile.h"
#include "UserMain.h"

void All_PID_Init();
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z);
void Turn_Angle(float Target_Angle);
float Angle_Control(float Start_Angle);
float Get_Image_Errox();
void Car_run();
void Car_run_X(float Speed);

typedef struct 
{
    float Speed_X;
    float Speed_Y;
    float Speed_Z;
    Rotaryservo_Handle Depot_Pos;
}Car_Handle;

typedef struct 
{
    float Offset;//�ۼ�ת���ĽǶ�
    float Angle_Last;//��һ�εĽǶ�
    float Current_Angle; //��ǰ�Ƕ�
    volatile bool Finish;//ת���Ƿ����
}Turn_Handle;


extern float Direction_Err;
extern Pid_TypeDef Image_PID;
extern Pid_TypeDef BorderPlace_PID;
extern float Forward_Speed;
extern bool Turn_Finsh;
extern float Start_Angle;
extern Pid_TypeDef Angle_PID;
extern Pid_TypeDef Foward_PID;
extern Pid_TypeDef Turn_PID;
extern Car_Handle Car;
extern Turn_Handle Turn;
extern Pid_TypeDef AngleControl_PID;
#endif