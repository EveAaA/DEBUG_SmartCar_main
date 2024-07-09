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
    Finsh_Return,
    Find_Cross,//找到十字
    Ready_Find_Place,//准备找第一个位置
    Find_Place,//寻找放置位置
    Ring_First_Place,//圆环第一个放置区域
    Ready_Find_Next_First,//圆环第一个放置点放好后前往下一个放置点
    Wait_PlaceData,//等待放置区域微调数据
    Move_Place,//移动到放置区域
    Classify_Place,//识别小类放置区域
    Ready_Find_Next,//准备寻找下一个小类放置区域
    No_Board_Return,//没有卡片返回赛道
    No_Board_Ring,
    Find_Ring,//找到圆环
    Ready_Ring,//准备
    Find_Zebra,//找到斑马线
    Wait_Big_Data,//等待大类放置区域数据
    Unload_Next,//前往下一个卸货点
    Unload_Board,//卸货
}Fsm_State_Handle;

typedef struct GetCard
{
	Place_t  CardName;
	uint8   WareIndex;
	uint8   cardNum;
}GetCard_t;

typedef struct 
{
    Current_State CurState;//当前处于哪个状态机
    volatile uint8 Line_Board_State;//散落在赛道旁的卡片
    volatile uint8 Unload_State;//终点前卸货状态机
    volatile uint8 Cross_Board_State;//十字回环卡片
    volatile uint8 Ring_Board_State;//圆环卡片

    float Static_Angle;//静止的角度
    bool Stop_Flag;//停车
    int8 Board_Dir;//散落在赛道的哪一边
    float Target_Pos_X;
    float Target_Pos_Y;

    bool Cross_Flag_;
    uint8 Cross_Dir;//十字方向
    int8 Small_Board[6];//卡片小类
    int8 Small_Board_Depot[6];//卡片小类仓库位置
    int8 Small_Depot_Count;//小类卡片仓库的索引
    uint8 Small_Count;//小类卡片张数
    int8 Small_Type;//具体是哪小类
    uint8 Same_Type;//
    bool Same_Board_Flag;//出现了相同类别

    uint8 Ring_Dir;//圆环方向
    uint8 Pick_Count;//

    bool Take_Board_Out;//终点前卸货策略
    int8 Big_Board;//卡片大类
    uint8 Big_Pos_Count;//大类次数,也就是去了几个大类的位置
    uint8 Big_Count[3];//大类每类数量
    uint8 Unload_Count;//卸货次数
    Rotaryservo_Handle Depot_Pos;//仓库的位置
    uint8 Big_Pos[3];//大类的位置
}FSM_Handle;

typedef struct WareState
{
    bool isSame; // 用于判断是否相同标志位
    uint8 currWareNum; // 当前使用的仓库数量
    uint8 notEmptyNum; // 用于记录多少个仓库已用
    GetCard_t list[5];     // 同时储存仓库的下标信息， 对应卡片类别信息， 当前仓库卡片数量
    bool isWareUsed[5];    // 判别有哪一个仓库未被使用
}WareState_t;

extern FSM_Handle MyFSM;
void FSM_main();

#endif