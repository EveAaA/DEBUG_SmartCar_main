#ifndef _INERTIALNAVIGATION_H
#define _INERTIALNAVIGATION_H

#include "zf_common_headfile.h"
#include "PID.h"
typedef struct
{
    float Start_Position_X;//��ʼX����
    float Start_Position_Y;//��ʼY����
    float Cur_Position_X;//��ǰX����
    float Cur_Position_Y;//��ǰY����
    float Start_Angle;//��ʼ�Ƕ�
    float Cur_Angle;//��ǰ�Ƕ�
    float End_Angle;//�����Ƕ�
    float Target_Position_X;//Ŀ��X����
    float Target_Position_Y;//Ŀ��Y����
    float X_Speed;//X���ٶ�
    float Y_Speed;//Y���ٶ�
    bool Start_Flag;//�������Ե���
    bool Finish_Flag;//���Ե�������
}Navigation_Handle;

typedef enum
{
    Start_State = 0,//��ʼ״̬����¼��ʼֵ
    X_State,//X���ƶ�״̬
    Stop,//X���ƶ�����,�ж��Ƿ��ƶ���λ
    Y_State,//Y���ƶ�״̬
    Move_Finish,//�ƶ�����
}State;

extern Navigation_Handle Navigation;
extern Pid_TypeDef DistanceX_PID;
extern Pid_TypeDef DistanceY_PID;
void Reset_Navigation();
void Navigation_Process(float x,float y);
void Navigation_Process_Image(float x,float y);
#endif