#ifndef _CARCONTROL_H
#define _CARCONTROL_H

#include "zf_common_headfile.h"

extern float Direction_Err;

void All_PID_Init();
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z);
extern float Image_Erro_;
void Car_run();
void Change_Direction(void);
#endif