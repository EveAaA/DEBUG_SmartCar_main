#ifndef _CARCONTROL_H
#define _CARCONTROL_H

#include "zf_common_headfile.h"
#include "UserMain.h"

// #define debug_switch  //�Ƿ����

void All_PID_Init();
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z);
void Turn_Angle(float Target_Angle);
void Car_run();
void Change_Direction(void);
void X_Move_Action();
void Y_Move_Action();
extern float Direction_Err;
extern float Image_Erro_;
extern Pid_TypeDef Image_PID;
extern Pid_TypeDef BorderPlace_PID;
extern float Forward_Speed;
extern bool Turn_Finsh;
extern float Start_Angle;
extern Pid_TypeDef Angle_PID;
extern Pid_TypeDef Foward_PID;
extern Pid_TypeDef Turn_PID;

#endif