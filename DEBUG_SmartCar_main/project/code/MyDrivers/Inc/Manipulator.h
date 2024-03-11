#ifndef _MANIPULATOR_H
#define _MANIPULATOR_H

#include "zf_common_headfile.h"

typedef struct 
{
    uint8_t Pin;//��Ӧ����
    float Max_Angle;//���Ƕ�
    float Min_Angle;//��С�Ƕ�
    float Init_Angle;//��ʼ���Ƕ�
    float Set_Angle;//���Ƶ�ʱ�����õĽǶ�
}Servo_Handle;

void Manipulator_Init();
void Set_Servo_Angle(Servo_Handle Servo,float Angle);

#endif