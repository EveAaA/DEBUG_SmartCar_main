#ifndef _CARCONTROL_H
#define _CARCONTROL_H

#include "zf_common_headfile.h"
#include "UserMain.h"

// #define debug_switch  //ÊÇ·ñµ÷ÊÔ

void All_PID_Init();
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z);
void Turn_Angle(float Target_Angle);
void Car_run();
void Change_Direction(void);
void Back_Autodrome();
void Forward_Board();
extern float Direction_Err;
extern float Image_Erro_;
extern Pid_TypeDef Image_PID;
#endif