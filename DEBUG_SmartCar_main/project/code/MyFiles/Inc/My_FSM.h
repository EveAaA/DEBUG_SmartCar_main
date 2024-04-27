#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Line_Patrol = 0,//巡线
    Line_Left_Board,//卡片在赛道左边
    Line_Right_Board,//卡片在赛道右边
}Current_State;

typedef enum
{
    Find = 0,//找到卡片
    Wait_Data,//等待数据
    Move,//移动到卡片前面
    Confirm,//确认是否移动到位
    Pick,//捡起卡片
    Return_Line,//返回赛道
}Line_Board_Handle;

typedef struct 
{
    Current_State CurState;
    Line_Board_Handle LeftBoard_State;
    Line_Board_Handle RightBoard_State;
}FSM_Handle;
void FSM_main();

#endif