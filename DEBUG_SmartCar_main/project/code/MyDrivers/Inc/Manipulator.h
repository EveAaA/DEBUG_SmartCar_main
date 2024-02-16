#ifndef _MANIPULATOR_H
#define _MANIPULATOR_H

#include "zf_common_headfile.h"

typedef enum
{
    Up_Servo,
    Down_Servo,
}ServoHandle;

void Manipulator_Init();
void Set_Servo_Angle(ServoHandle Servo,float Angle);

#endif