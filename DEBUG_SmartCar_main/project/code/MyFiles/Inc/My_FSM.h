#ifndef _MYFSM_H
#define _MYFSM_H

#include "UserMain.h"

typedef enum
{
    Depart = 0,//����
    Line_Patrol,//Ѳ��
    Line_Board,//ɢ�俨Ƭ
    Cross_Board,//ʮ�ֻػ���Ƭ
    Ring_Board,//Բ����Ƭ
    Unload,//����ж��
}Current_State;

typedef enum
{
    Find = 0,//�ҵ���Ƭ
    Wait_Data,//�ȴ�΢������
    Move,//�ƶ�����Ƭǰ��
    Pick,//����Ƭ
    Classify,//ʶ�����
    Return_Line,//��������
    Find_Cross,//�ҵ�ʮ��
    Find_Place,//Ѱ�ҷ���λ��
    Wait_PlaceData,//�ȴ���������΢������
    Move_Place,//�ƶ�����������
    Confirm_Place,//ȷ�Ϸ�������X���Ƿ��ƶ���λ
    Move_PlaceY,//�ƶ�����������Ƭǰ��
    Confirm_PlaceY,//ȷ�Ϸ�������Y���Ƿ��ƶ���λ
    Classify_Place,//ʶ��С���������
    Ready_Find_Next,//׼��Ѱ����һ��С���������
    Find_Ring,//�ҵ�Բ��
    Find_Zebra,//�ҵ�������
    Wait_Big_Data,//�ȴ����������������
    Unload_Next,//ǰ����һ��ж����
    Unload_Board,//ж��
}Fsm_State_Handle;


typedef struct 
{
    Current_State CurState;//��ǰ�����ĸ�״̬��
    uint8 Line_Board_State;//ɢ���������ԵĿ�Ƭ
    uint8 Unload_State;//�յ�ǰж��״̬��
    uint8 Cross_Board_State;//ʮ�ֻػ���Ƭ
    uint8 Ring_Board_State;//Բ����Ƭ

    float Static_Angle;//��ֹ�ĽǶ�
    bool Stop_Flag;//ͣ��
    int8 Board_Dir;//ɢ������������һ��
    float Target_Pos_X;
    float Target_Pos_Y;

    bool Cross_Flag_;
    uint8 Cross_Dir;//ʮ�ַ���
    int8 Small_Board[6];//��ƬС��
    uint8 Small_Count;//С�࿨Ƭ����
    int8 Small_Type;//��������С��
    uint8 Same_Type;//��ͬ����

    uint8 Ring_Dir;//Բ������

    int8 Big_Board;//��Ƭ����
    uint8 Big_Pos_Count;//�������,Ҳ����ȥ�˼��������λ��
    uint8 Big_Count[3];//����ÿ������
    uint8 Unload_Count;//ж������
    Rotaryservo_Handle Depot_Pos;//�ֿ��λ��
    uint8 Big_Pos[3];//�����λ��
}FSM_Handle;
extern FSM_Handle MyFSM;
void FSM_main();

#endif