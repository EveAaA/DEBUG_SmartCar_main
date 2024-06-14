#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Depart = 0,//����
    Line_Patrol,//Ѳ��
    Line_Board,//ɢ�俨Ƭ
    Unload,//����ж��
}Current_State;

typedef enum
{
    Find = 0,//�ҵ���Ƭ
    Wait_Data,//�ȴ�΢������
    Move,//�ƶ�����Ƭǰ��
    Confirm,//ȷ��X���Ƿ��ƶ���λ
    Move_Y,//�ƶ�����Ƭǰ��
    Confirm_Y,//ȷ��Y���Ƿ��ƶ���λ
    Pick,//����Ƭ
    Classify,//ʶ�����
    Return_Line,//��������
    Find_Zebra,//�ҵ�������
    Wait_Big_Data,//�ȴ����������������
    Unload_Next,//ǰ����һ��ж����
    Unload_Board,//ж��
}Fsm_State_Handle;


typedef struct 
{
    Current_State CurState;//��ǰ�����ĸ�״̬��
    uint8 Line_Board_State;//ɢ���������ԵĿ�Ƭ
    uint8 Unload_State;
    float Static_Angle;//��ֹ�ĽǶ�
    int8 Board_Dir;//ɢ������������һ��
    int8 Big_Board;//��Ƭ����
    uint8 Big_Pos_Count;//�������,Ҳ����ȥ�˼��������λ��
    uint8 Big_Count[3];//����ÿ������
    uint8 Unload_Count;//ж������
    bool Stop_Flag;//ͣ��
    Rotaryservo_Handle Depot_Pos;//�ֿ��λ��
    uint8 Big_Pos[3];
}FSM_Handle;
extern FSM_Handle MyFSM;
void FSM_main();

#endif