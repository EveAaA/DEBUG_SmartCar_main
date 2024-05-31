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
    volatile uint16 Servo_Time;
}Servo_Handle;

typedef struct
{
    volatile bool Put_Down;
    volatile bool Put_Up;
    volatile bool Put_Depot;
    volatile bool Put_Out;
    volatile bool Pick_End;
}Servo_Flag_Handle;

typedef enum
{
    White = 0,
    Black,
    Red,
    Yellow,
}Rotaryservo_Handle;

void Manipulator_Init();
void Pick_Card();
void Rotary_Switch(Rotaryservo_Handle RotaryServo,bool Door);
void Put_Depot(int8 Card_Class);
void Take_Card_Out();
void Set_Servo_Angle(Servo_Handle Servo,uint16 Angle);

extern Servo_Handle Raise_Servo;
extern Servo_Handle Stretch_Servo;
extern Servo_Handle Rotary_Servo;
extern Servo_Flag_Handle Servo_Flag;
#endif