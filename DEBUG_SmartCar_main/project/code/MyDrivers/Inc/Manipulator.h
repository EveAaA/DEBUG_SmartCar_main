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