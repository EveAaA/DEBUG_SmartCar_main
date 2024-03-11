#ifndef _INERTIALNAVIGATION_H
#define _INERTIALNAVIGATION_H

#include "zf_common_headfile.h"

typedef struct
{
    float Start_Position_X;//��ʼX����
    float Start_Position_Y;//��ʼY����
    float Cur_Position_X;//��ǰX����
    float Cur_Position_Y;//��ǰY����
    float Start_Angle;//��ʼ�Ƕ�
    float Cur_Angle;//��ǰ�Ƕ�
    float Target_Position_X;//Ŀ��X����
    float Target_Position_Y;//Ŀ��Y����
    uint8 Start_Flag;//�������Ե���
}Navigation_Handle;

extern Navigation_Handle Navigation;

void Enable_Navigation();
void Navigation_Process(float x,float y);
void Pid_Init();
#endif