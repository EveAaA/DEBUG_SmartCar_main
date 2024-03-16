/**
  ******************************************************************************
  * @file    FSM.c
  * @author  俞立
  * @brief   状态机
  *
  @verbatim
	无
	@endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "FSM.h"

/* Define\Declare ------------------------------------------------------------*/

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   状态机初始化
-- @param   FSM_t *fsm 状态机名称
-- @param   uint8 state 初始状态
-- @param   FSMTable_t *fsmtable 状态表 
-- @date    2024/3/13
**/
void FSMInit(FSM_t *fsm, uint8 state, FSMTable_t *fsmtable,uint8 Number)
{
	fsm->CurState = state;
	fsm->FsmTable = fsmtable;
	fsm->Fsm_Number = Number;
}

/**@brief   状态机运行
-- @param   FSM_t *fsm 状态机名称
-- @date    2024/3/13
**/
void FSMRun(FSM_t *fsm)
{
	uint8 Current_Fsm = fsm->Fsm_Number;
	static uint8 Last_Fsm = 0;
	uint8 curState = fsm->CurState;  //状态机当前状态
	FSMTable_t *fsmTable = fsm->FsmTable;  //状态机状态表

	if(fsm->Fsm_Number != Last_Fsm)
	{
		fsm->CurState = fsmTable[0].CurState;
	}

	void (*eventAction)(void) = fsmTable[curState].EventAction;  //状态机当前状态执行函数
	bool (*eventJumpCondition)(void) = fsmTable[curState].EventJumpCondition;  //状态机跳转事件条件函数
	
	if(eventJumpCondition())
	{
		fsm->CurState = fsmTable[curState].NextState;
		eventAction = fsmTable[fsm->CurState].EventAction;
	}
	eventAction();
	Last_Fsm = Current_Fsm;
}
