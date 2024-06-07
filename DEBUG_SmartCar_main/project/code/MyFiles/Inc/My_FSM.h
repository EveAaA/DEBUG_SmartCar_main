#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Depart = 0,//����
    Line_Patrol,//Ѳ��
    Line_Board,//ɢ�俨Ƭ
}Current_State;

typedef enum
{
    Find = 0,//�ҵ���Ƭ
    Wait_Data,//�ȴ�����
    Move,//�ƶ�����Ƭǰ��
    Confirm,//ʶ�����
    Move_Y,//�ƶ�����Ƭǰ��
    Confirm_Y,//ʶ�����
    Pick,//����Ƭ
    Classify,//ʶ�����
    Return_Line,//��������
}Line_Board_Handle;

typedef struct 
{
    Current_State CurState;//��ǰ�����ĸ�״̬��
    Line_Board_Handle Line_Board_State;//ɢ���������ԵĿ�Ƭ
    float Static_Angle;//��ֹ�ĽǶ�
    int8 Board_Dir;//ɢ������������һ��
    int8 Big_Board;//��Ƭ����
}FSM_Handle;
void FSM_main();

#endif