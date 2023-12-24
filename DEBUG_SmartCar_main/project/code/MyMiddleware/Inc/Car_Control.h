#ifndef _CARCONTROL_H
#define _CARCONTROL_H

#include "zf_common_headfile.h"

extern double Direction_Err;

void All_PID_Init();
void Set_Car_Speed(double Speed_X,double Speed_Y,double Speed_Z);
extern double Image_Erro_;
void Car_run();
void Change_Direction(void);
#endif