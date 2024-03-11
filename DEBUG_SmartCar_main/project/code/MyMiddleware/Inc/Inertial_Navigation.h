#ifndef _INERTIALNAVIGATION_H
#define _INERTIALNAVIGATION_H

#include "zf_common_headfile.h"

typedef struct
{
    float Start_Position_X;//初始X坐标
    float Start_Position_Y;//初始Y坐标
    float Cur_Position_X;//当前X坐标
    float Cur_Position_Y;//当前Y坐标
    float Start_Angle;//初始角度
    float Cur_Angle;//当前角度
    float Target_Position_X;//目标X坐标
    float Target_Position_Y;//目标Y坐标
    uint8 Start_Flag;//启动惯性导航
}Navigation_Handle;

extern Navigation_Handle Navigation;

void Enable_Navigation();
void Navigation_Process(float x,float y);
void Pid_Init();
#endif