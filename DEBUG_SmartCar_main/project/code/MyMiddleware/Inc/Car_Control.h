#ifndef _CARCONTROL_H
#define _CARCONTROL_H

#include "zf_common_headfile.h"
#include "UserMain.h"

void All_PID_Init();
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z);
void Turn_Angle(float Target_Angle);
float Angle_Control(float Start_Angle);
float Get_Image_Errox();
void Car_run(float Speed);
void Car_run_X(float Speed);
bool Car_Put_Board();

typedef struct 
{
    float Speed_X;//小车X轴速度
    float Speed_Y;//小车Y轴速度
    float Speed_Z;//小车Z轴速度
    bool Image_Flag;//是否进行图像处理
    bool Speed;
}Car_Handle;

typedef struct 
{
    float Offset;//累计转动的角度
    float Angle_Last;//上一次的角度
    float Current_Angle; //当前角度
    volatile bool Finish;//转动是否完成
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