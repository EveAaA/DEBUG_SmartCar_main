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
    float End_Angle;//结束角度
    float Target_Position_X;//目标X坐标
    float Target_Position_Y;//目标Y坐标
    bool Start_Flag;//启动惯性导航
    bool Finish_Flag;//惯性导航结束
}Navigation_Handle;

typedef enum
{
    Start_State = 0,//开始状态，记录初始值
    X_State,//X轴移动状态
    Stop,//X轴移动结束
    Y_State,//Y轴移动状态
    Move_Finish,//移动结束
}State;

extern Navigation_Handle Navigation;

void Reset_Navigation();
void Navigation_Process(float x,float y);
#endif