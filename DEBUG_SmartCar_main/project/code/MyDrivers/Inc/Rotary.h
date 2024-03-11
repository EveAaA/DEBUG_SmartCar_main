#ifndef _ROTARY_H
#define _ROTARY_H

#include "zf_common_headfile.h"
#define Rotary_A B23
#define Rotary_B B14
#define Rotary_D C26
void Rotary_Init();

typedef struct
{
    uint8 Clockwise;//顺时针旋转
    uint8 Anticlockwise;//逆时针旋转
    uint8 Press;//按下
    uint8 Press_Clockwise;//按下顺时针旋转
    uint8 Press_Anticlockwise;//按下逆时针旋转
}Rotary_Handle;

extern Rotary_Handle Rotary;

#endif