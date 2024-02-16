#ifndef _INERTIALNAVIGATION_H
#define _INERTIALNAVIGATION_H

#include "zf_common_headfile.h"

typedef struct
{
    float Start_Position_X;
    float Start_Position_Y;
    float Cur_Position_X;
    float Cur_Position_Y;
    float Start_Angle;
    float Cur_Angle;
    float Target_Position_X;
    float Target_Position_Y;
    uint8 Start_Flag;
}Navigation_Handle;

extern Navigation_Handle Navigation;

void Enable_Navigation();
void Navigation_Process(float x,float y);
void Pid_Init();
#endif