#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Depart = 0,//发车
    Line_Patrol,//巡线
    Line_Board,//散落卡片
    Unload,//大类卸货
}Current_State;

typedef enum
{
    Find = 0,//找到卡片
    Wait_Data,//等待微调数据
    Move,//移动到卡片前面
    Confirm,//确认X轴是否移动到位
    Move_Y,//移动到卡片前面
    Confirm_Y,//确认Y轴是否移动到位
    Pick,//捡起卡片
    Classify,//识别分类
    Return_Line,//返回赛道
    Find_Zebra,//找到斑马线
    Wait_Big_Data,//等待大类放置区域数据
    Unload_Next,//前往下一个卸货点
    Unload_Board,//卸货
}Fsm_State_Handle;


typedef struct 
{
    Current_State CurState;//当前处于哪个状态机
    uint8 Line_Board_State;//散落在赛道旁的卡片
    uint8 Unload_State;
    float Static_Angle;//静止的角度
    int8 Board_Dir;//散落在赛道的哪一边
    int8 Big_Board;//卡片大类
    uint8 Big_Count;//大类次数
    uint8 Big_Count_1;//类别1数量
    uint8 Big_Count_2;//类别2数量
    uint8 Big_Count_3;//类别3数量
    uint8 Unload_Count;//卸货次数
    bool Stop_Flag;//停车
}FSM_Handle;
void FSM_main();

#endif