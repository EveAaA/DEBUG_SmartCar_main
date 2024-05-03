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
    uint16 Servo_Time;
}Servo_Handle;

typedef struct
{
    bool Put_Down;
    bool Put_Up;
    bool Put_Depot;
    bool Put_Out;
}Servo_Flag_Handle;

void Manipulator_Init();
void Set_Servo_Angle(Servo_Handle Servo,float Angle);
void Manipulator_PutDown();
void Manipulator_PutUp();
void Pick_Card();
void Put_Depot();
void Take_Card_Out();
extern Servo_Handle Raise_Servo;
extern Servo_Handle Stretch_Servo;
extern Servo_Flag_Handle Servo_Flag;
#endif