#ifndef FSM_H_
#define FSM_H_

#include "zf_common_headfile.h"

typedef void (*EventAction_t)(void);  //��ǰ״ִ̬�к�����ָ��
typedef bool (*EventJumpCondition_t)(void);   //��ת�¼�����������ָ�루����ֻ�ܷ���1��0��

typedef struct FSMTable  //״̬��
{
	uint8 CurState;  //��ǰ״̬��ö�����ʹ�0��ʼ��
	EventAction_t EventAction;  //��ǰ״ִ̬�к�����ָ��
	EventJumpCondition_t EventJumpCondition;   //��ת�¼�����������ָ��
	uint8_t NextState;  //��һ��״̬
}FSMTable_t;

typedef struct FSM  //״̬��
{
	uint8 CurState;  //��ǰ״̬��ö�����ʹ�0��ʼ��
	uint8 Fsm_Number;//״̬�����
	FSMTable_t *FsmTable;  //״̬��
}FSM_t;


void FSMInit(FSM_t *fsm, uint8 state, FSMTable_t *fsmtable,uint8 Number);
void FSMRun(FSM_t *fsm);

#endif