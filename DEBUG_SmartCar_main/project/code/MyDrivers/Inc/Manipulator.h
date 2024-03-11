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
}Servo_Handle;

void Manipulator_Init();
void Set_Servo_Angle(Servo_Handle Servo,float Angle);

#endif