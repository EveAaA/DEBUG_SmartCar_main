/**
  ******************************************************************************
  * @file    FSM.c
  * @author  ����
  * @brief   ״̬��
  *
  @verbatim
	��
	@endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "FSM.h"

/* Define\Declare ------------------------------------------------------------*/

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ״̬����ʼ��
-- @param   FSM_t *fsm ״̬������
-- @param   uint8 state ��ʼ״̬
-- @param   FSMTable_t *fsmtable ״̬�� 
-- @date    2024/3/13
**/
void FSMInit(FSM_t *fsm, uint8 state, FSMTable_t *fsmtable)
{
	fsm->CurState = state;
	fsm->FsmTable = fsmtable;
}

/**@brief   ״̬������
-- @param   FSM_t *fsm ״̬������
-- @date    2024/3/13
**/
void FSMRun(FSM_t *fsm)
{
	uint8 curState = fsm->CurState;  //״̬����ǰ״̬
	FSMTable_t *fsmTable = fsm->FsmTable;  //״̬��״̬��
	void (*eventAction)(void) = fsmTable[curState].EventAction;  //״̬����ǰ״ִ̬�к���
	bool (*eventJumpCondition)(void) = fsmTable[curState].EventJumpCondition;  //״̬����ת�¼���������
	
	if(eventJumpCondition())
	{
		fsm->CurState = fsmTable[curState].NextState;
		eventAction = fsmTable[fsm->CurState].EventAction;
	}
	eventAction();
}
