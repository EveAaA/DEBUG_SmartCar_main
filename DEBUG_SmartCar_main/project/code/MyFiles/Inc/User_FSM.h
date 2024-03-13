#ifndef USERFSM_H_
#define USERFSM_H_

#include "FSM.h"
#include "UserMain.h"

enum State  //状态
{
    Depart = 0,//发车
    LinePatrol = 1,//巡线
    FindBoard = 2,//找到卡片
    BackAutodrome = 3,//返回赛道
};


void My_FSM_Init();
extern FSM_t *CURRENT_FSM;//当前运行状态机

#endif