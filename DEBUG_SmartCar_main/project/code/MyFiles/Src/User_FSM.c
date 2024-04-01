/**
  ******************************************************************************
  * @file    User_FSM.c
  * @author  庄文标
  * @brief   主状态机
  *
    @verbatim
	无
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "User_FSM.h"

/* Define\Declare ------------------------------------------------------------*/
FSM_t *CURRENT_FSM;//当前运行状态机
FSM_t Line_FSM;//巡线状态机
FSM_t Left_Board_FSM;//卡片状态机_赛道旁
FSM_t Right_Board_FSM;//卡片状态机_赛道旁


/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
 **/

/**@brief   发车选择
-- @auther  庄文标
-- @date    2024/3/13
**/
bool Depart_Select()
{
    if(Start == 1)
    {
        return true;
    }
    return false;
}

/**@brief   找到卡片
-- @auther  庄文标
-- @date    2024/3/13
**/
bool Board_Find()
{
    if(FINDBORDER_DATA.FINDBORDER_FLAG == true)
    {
        Forward_Speed = 3;
    }
    else
    {
        Forward_Speed = 5;
    }
    if(FINDBORDER_DATA.dir == LEFT)
    {
        CURRENT_FSM = &Left_Board_FSM;//切换到卡片状态机
    }
    return false;
}

/**@brief   找到卡片
-- @auther  庄文标
-- @date    2024/3/13
**/
bool TurnAction_Finish()
{
    if(Turn_Finsh)
    {
        Start_Angle = Gyro_YawAngle_Get();
        // printf("board_finishy\r\n");
        return true;
    }
    return false;
}

/**@brief   平移结束
-- @auther  庄文标
-- @date    2024/3/13
**/
bool Xmove_finish()
{
    if(fabs(FINETUNING_DATA.dx) <= 8)
    {
        return true;
    }
    return false;
}

/**@brief   前进结束
-- @auther  庄文标
-- @date    2024/3/13
**/
bool Ymove_finish()
{
    if(fabs(FINETUNING_DATA.dy) <= 8)
    {
        return true;
    }
#ifdef debug_switch
        printf("back_finish\n\r");
#endif
        return false;
}

bool Wait()
{
    return false;
}

/**
 ******************************************************************************
 *  @defgroup 状态表
 *  @brief
 *
 **/

FSMTable_t LineTable[] =
{
    //当前的状态		当前状态执行函数		跳转事件条件函数		下一个状态
	{Depart,           Motor_Disable,         Depart_Select,         LinePatrol    },
    {LinePatrol,       Car_run,               Board_Find,            FindBoard     },
};

FSMTable_t LeftBoardTable[] =
{
    //当前的状态		当前状态执行函数		跳转事件条件函数		下一个状态
	{Find,              Change_Direction,     TurnAction_Finish,      X_Move       },
    {X_Move,            X_Move_Action,        Xmove_finish,           Y_Move       },
    {Y_Move,            Y_Move_Action,        Ymove_finish,           finish       },
    {finish,            Motor_Disable,        Wait,                   fuck         },
};


/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
 **/

/**@brief   状态机初始化
-- @auther  庄文标
-- @date    2024/3/13
**/
void My_FSM_Init()
{
    CURRENT_FSM = &Line_FSM;
    FSMInit(&Line_FSM,Depart,LineTable,0);
    FSMInit(&Left_Board_FSM,Find,LeftBoardTable,1);
}
