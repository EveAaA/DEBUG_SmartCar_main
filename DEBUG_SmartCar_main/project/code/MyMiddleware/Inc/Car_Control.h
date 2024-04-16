#ifndef _CARCONTROL_H
#define _CARCONTROL_H

#include "zf_common_headfile.h"
#include "UserMain.h"

// #define debug_switch  //????

void All_PID_Init();
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z);
void Turn_Angle(float Target_Angle);
float Angle_Control(float Start_Angle);
void Car_run();
void Change_Direction(void);
void Change_Right(void);
void Return_Right();
void Return_Action();
void Car_Stop_Wait_Data_L();
void Car_Stop_Wait_Data_R();
void Move_Action();
extern float Direction_Err;
extern Pid_TypeDef Image_PID;
extern Pid_TypeDef BorderPlace_PID;
extern float Forward_Speed;
extern bool Turn_Finsh;
extern float Start_Angle;
extern Pid_TypeDef Angle_PID;
extern Pid_TypeDef Foward_PID;
extern Pid_TypeDef Turn_PID;

#endif