/**
  ******************************************************************************
  * @file    FSM.c
  * @author  ������ׯ�ı�
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
-- @auther  ׯ�ı�
-- @param   FSM_t *fsm ״̬������
-- @param   uint8 state ��ʼ״̬
-- @param   FSMTable_t *fsmtable ״̬�� 
-- @date    2024/3/13
**/
void FSMInit(FSM_t *fsm, uint8 state, FSMTable_t *fsmtable,uint8 Number)
{
	fsm->CurState = state;
	fsm->FsmTable = fsmtable;
	fsm->Fsm_Number = Number;
}

/**@brief   ״̬������
-- @auther  ׯ�ı�
-- @param   FSM_t *fsm ״̬������
-- @date    2024/3/13
**/
void FSMRun(FSM_t *fsm)
{
	uint8 Current_Fsm = fsm->Fsm_Number;
	static uint8 Last_Fsm = 0;
	uint8 curState = fsm->CurState;  //״̬����ǰ״̬
	FSMTable_t *fsmTable = fsm->FsmTable;  //״̬��״̬��

	if(Current_Fsm != Last_Fsm)//Ϊ��ȷ��״̬����ת���Ǵӵ�һ��״̬��ʼ
	{
		fsm->CurState = fsmTable[0].CurState;
	}

	void (*eventAction)(void) = fsmTable[curState].EventAction;  //״̬����ǰ״ִ̬�к���
	bool (*eventJumpCondition)(void) = fsmTable[curState].EventJumpCondition;  //״̬����ת�¼���������
	
	if(eventJumpCondition())
	{
		fsm->CurState = fsmTable[curState].NextState;
		eventAction = fsmTable[fsm->CurState].EventAction;
	}
	eventAction();
	Last_Fsm = Current_Fsm;
}
