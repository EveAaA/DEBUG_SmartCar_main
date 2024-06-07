#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Depart = 0,//发车
    Line_Patrol,//巡线
    Line_Board,//散落卡片
}Current_State;

typedef enum
{
    Find = 0,//找到卡片
    Wait_Data,//等待数据
    Move,//移动到卡片前面
    Confirm,//识别类别
    Move_Y,//移动到卡片前面
    Confirm_Y,//识别类别
    Pick,//捡起卡片
    Classify,//识别分类
    Return_Line,//返回赛道
}Line_Board_Handle;

typedef struct 
{
    Current_State CurState;//当前处于哪个状态机
    Line_Board_Handle Line_Board_State;//散落在赛道旁的卡片
    float Static_Angle;//静止的角度
    int8 Board_Dir;//散落在赛道的哪一边
    int8 Big_Board;//卡片大类
}FSM_Handle;
void FSM_main();

#endif