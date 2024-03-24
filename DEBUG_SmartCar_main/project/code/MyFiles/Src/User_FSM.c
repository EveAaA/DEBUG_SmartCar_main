/**
  ******************************************************************************
  * @file    MatrixKeyBoard.c
  * @author  俞立，何志远，葛子磊
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
FSM_t Board_FSM;//卡片状态机_赛道旁


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
    if(fabs(border.dx) > 5 && border.dx != 127)
    {
        CURRENT_FSM = &Board_FSM;//切换到卡片状态机
        Enable_Navigation();//使能惯导，记录数据
        // printf("board_find");
    }
    return false;
}

/**@brief   找到卡片
-- @auther  庄文标
-- @date    2024/3/13
**/
bool Board_Find_Finish()
{
    if(fabs(border.dx) < 5)
    {
        // printf("board_finishy\r\n");
        return true;
    }
    return false;
}

/**@brief   返回赛道结束
-- @auther  庄文标
-- @date    2024/3/13
**/
bool Back_Autodrome_Finish()
{
    Navigation.Cur_Position_X = Get_X_Distance();//获取自身坐标值
    if((fabs(0 - Navigation.Cur_Position_X) < 1.0f && fabs(Navigation.Cur_Angle < 1.0f)))
    {
        Navigation.Cur_Position_X = 0;//记录值清零
        Navigation.Cur_Position_Y = 0;
        Navigation.Start_Angle = 0;
        Navigation.Start_Flag = 0;//关闭惯性导航
#ifdef debug_switch
        printf("back_finish\n\r");
#endif
        CURRENT_FSM = &Line_FSM;//切换到巡线状态机
        return false;
    }
    return false;
}

bool Foward_finish()
{
    if(border.dx == 127)
    {
        return true;
    }
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

FSMTable_t BoardTable[] =
{
    //当前的状态		当前状态执行函数		跳转事件条件函数		下一个状态
	{Find,              Change_Direction,     Board_Find_Finish,      Forward           },
    {Forward,           Forward_Board,        Foward_finish,          BackAutodrome     },
    {BackAutodrome,     Back_Autodrome,       Back_Autodrome_Finish,  finish            },
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
    FSMInit(&Board_FSM,Find,BoardTable,1);
}
