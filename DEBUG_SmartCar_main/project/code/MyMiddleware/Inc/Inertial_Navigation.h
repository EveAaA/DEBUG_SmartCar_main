#ifndef _INERTIALNAVIGATION_H
#define _INERTIALNAVIGATION_H

#include "zf_common_headfile.h"
#include "PID.h"
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
    float X_Speed;//X轴速度
    float Y_Speed;//Y轴速度
    bool Start_Flag;//启动惯性导航
    bool Finish_Flag;//惯性导航结束
    bool X_Finish;
    bool Y_Finish;
}Navigation_Handle;

typedef enum
{
    Start_State = 0,//开始状态，记录初始值
    Move_State,//移动状态
    Move_State_Y,
    Move_Finish,//移动结束
}State;

extern Navigation_Handle Navigation;
extern Pid_TypeDef DistanceX_PID;
extern Pid_TypeDef DistanceY_PID;
void Reset_Navigation();
void Navigation_Process(float x,float y);
void Navigation_Process_Y(float x,float y);
void Navigation_Process_Image(float Target_Pos_X,float Target_Pos_Y);
void Navigation_Process_Y_Image(float x,float y);

#endif