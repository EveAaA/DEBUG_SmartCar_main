#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Depart = 0,//发车
    Line_Patrol,//巡线
    Line_Board,//散落卡片
    Cross_Board,//十字回环卡片
    Ring_Board,//圆环卡片
    Unload,//大类卸货
}Current_State;

typedef enum
{
    Find = 0,//找到卡片
    Wait_Data,//等待微调数据
    Move,//移动到卡片前面
    Pick,//捡起卡片
    Classify,//识别分类
    Return_Line,//返回赛道
    Find_Cross,//找到十字
    Find_Place,//寻找放置位置
    Wait_PlaceData,//等待放置区域微调数据
    Move_Place,//移动到放置区域
    Confirm_Place,//确认放置区域X轴是否移动到位
    Move_PlaceY,//移动到放置区域卡片前面
    Confirm_PlaceY,//确认放置区域Y轴是否移动到位
    Classify_Place,//识别小类放置区域
    Ready_Find_Next,//准备寻找下一个小类放置区域
    Find_Ring,//找到圆环
    Find_Zebra,//找到斑马线
    Wait_Big_Data,//等待大类放置区域数据
    Unload_Next,//前往下一个卸货点
    Unload_Board,//卸货
}Fsm_State_Handle;


typedef struct 
{
    Current_State CurState;//当前处于哪个状态机
    uint8 Line_Board_State;//散落在赛道旁的卡片
    uint8 Unload_State;//终点前卸货状态机
    uint8 Cross_Board_State;//十字回环卡片
    uint8 Ring_Board_State;//圆环卡片

    float Static_Angle;//静止的角度
    bool Stop_Flag;//停车
    int8 Board_Dir;//散落在赛道的哪一边
    float Target_Pos_X;
    float Target_Pos_Y;

    bool Cross_Flag_;
    uint8 Cross_Dir;//十字方向
    int8 Small_Board[6];//卡片小类
    uint8 Small_Count;//小类卡片张数
    int8 Small_Type;//具体是哪小类
    uint8 Same_Type;//相同种类

    uint8 Ring_Dir;//圆环方向

    int8 Big_Board;//卡片大类
    uint8 Big_Pos_Count;//大类次数,也就是去了几个大类的位置
    uint8 Big_Count[3];//大类每类数量
    uint8 Unload_Count;//卸货次数
    Rotaryservo_Handle Depot_Pos;//仓库的位置
    uint8 Big_Pos[3];//大类的位置
}FSM_Handle;
extern FSM_Handle MyFSM;
void FSM_main();

#endif