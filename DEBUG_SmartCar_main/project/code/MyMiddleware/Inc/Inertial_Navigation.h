#ifndef _INERTIALNAVIGATION_H
#define _INERTIALNAVIGATION_H

#include "zf_common_headfile.h"

typedef struct
{
    double Start_Position_X;
    double Start_Position_Y;
    double Cur_Position_X;
    double Cur_Position_Y;
    double Start_Angle;
    double Cur_Angle;
    double Target_Position_X;
    double Target_Position_Y;
    uint8 Start_Flag;
}Navigation_Handle;

extern Navigation_Handle Navigation;

void Enable_Navigation();
void Navigation_Process(double x,double y);
void Pid_Init();
#endif