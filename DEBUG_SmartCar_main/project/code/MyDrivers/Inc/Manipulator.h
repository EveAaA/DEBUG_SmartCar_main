#ifndef _MANIPULATOR_H
#define _MANIPULATOR_H

#include "zf_common_headfile.h"

typedef struct 
{
    uint8_t Pin;//对应引脚
    float Max_Angle;//最大角度
    float Min_Angle;//最小角度
    float Init_Angle;//初始化角度
    float Set_Angle;//控制的时候设置的角度
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