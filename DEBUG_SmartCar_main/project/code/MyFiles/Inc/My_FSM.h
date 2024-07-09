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
    Finsh_Return,
    Find_Cross,//�ҵ�ʮ��
    Ready_Find_Place,//׼���ҵ�һ��λ��
    Find_Place,//Ѱ�ҷ���λ��
    Ring_First_Place,//Բ����һ����������
    Ready_Find_Next_First,//Բ����һ�����õ�źú�ǰ����һ�����õ�
    Wait_PlaceData,//�ȴ���������΢������
    Move_Place,//�ƶ�����������
    Classify_Place,//ʶ��С���������
    Ready_Find_Next,//׼��Ѱ����һ��С���������
    No_Board_Return,//û�п�Ƭ��������
    No_Board_Ring,
    Find_Ring,//�ҵ�Բ��
    Ready_Ring,//׼��
    Find_Zebra,//�ҵ�������
    Wait_Big_Data,//�ȴ����������������
    Unload_Next,//ǰ����һ��ж����
    Unload_Board,//ж��
}Fsm_State_Handle;

typedef struct GetCard
{
	Place_t  CardName;
	uint8   WareIndex;
	uint8   cardNum;
}GetCard_t;

typedef struct 
{
    Current_State CurState;//��ǰ�����ĸ�״̬��
    volatile uint8 Line_Board_State;//ɢ���������ԵĿ�Ƭ
    volatile uint8 Unload_State;//�յ�ǰж��״̬��
    volatile uint8 Cross_Board_State;//ʮ�ֻػ���Ƭ
    volatile uint8 Ring_Board_State;//Բ����Ƭ

    float Static_Angle;//��ֹ�ĽǶ�
    bool Stop_Flag;//ͣ��
    int8 Board_Dir;//ɢ������������һ��
    float Target_Pos_X;
    float Target_Pos_Y;

    bool Cross_Flag_;
    uint8 Cross_Dir;//ʮ�ַ���
    int8 Small_Board[6];//��ƬС��
    int8 Small_Board_Depot[6];//��ƬС��ֿ�λ��
    int8 Small_Depot_Count;//С�࿨Ƭ�ֿ������
    uint8 Small_Count;//С�࿨Ƭ����
    int8 Small_Type;//��������С��
    uint8 Same_Type;//
    bool Same_Board_Flag;//��������ͬ���

    uint8 Ring_Dir;//Բ������
    uint8 Pick_Count;//

    bool Take_Board_Out;//�յ�ǰж������
    int8 Big_Board;//��Ƭ����
    uint8 Big_Pos_Count;//�������,Ҳ����ȥ�˼��������λ��
    uint8 Big_Count[3];//����ÿ������
    uint8 Unload_Count;//ж������
    Rotaryservo_Handle Depot_Pos;//�ֿ��λ��
    uint8 Big_Pos[3];//�����λ��
}FSM_Handle;

typedef struct WareState
{
    bool isSame; // �����ж��Ƿ���ͬ��־λ
    uint8 currWareNum; // ��ǰʹ�õĲֿ�����
    uint8 notEmptyNum; // ���ڼ�¼���ٸ��ֿ�����
    GetCard_t list[5];     // ͬʱ����ֿ���±���Ϣ�� ��Ӧ��Ƭ�����Ϣ�� ��ǰ�ֿ⿨Ƭ����
    bool isWareUsed[5];    // �б�����һ���ֿ�δ��ʹ��
}WareState_t;

extern FSM_Handle MyFSM;
void FSM_main();

#endif