#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Line_Patrol = 0,//Ѳ��
    Line_Left_Board,//��Ƭ���������
    Line_Right_Board,//��Ƭ�������ұ�
}Current_State;

typedef enum
{
    Find = 0,//�ҵ���Ƭ
    Wait_Data,//�ȴ�����
    Move,//�ƶ�����Ƭǰ��
    Confirm,//ȷ���Ƿ��ƶ���λ
    Pick,//����Ƭ
    Return_Line,//��������
}Line_Board_Handle;

typedef struct 
{
    Current_State CurState;
    Line_Board_Handle LeftBoard_State;
    Line_Board_Handle RightBoard_State;
}FSM_Handle;
void FSM_main();

#endif