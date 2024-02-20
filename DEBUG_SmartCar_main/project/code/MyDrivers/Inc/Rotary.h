#ifndef _ROTARY_H
#define _ROTARY_H

#include "zf_common_headfile.h"
#define Rotary_A B18
#define Rotary_B C0
#define Rotary_D C24
void Rotary_Init();

typedef struct
{
    uint8 Clockwise;//˳ʱ����ת
    uint8 Anticlockwise;//��ʱ����ת
    uint8 Press;//����
    uint8 Press_Clockwise;//����˳ʱ����ת
    uint8 Press_Anticlockwise;//������ʱ����ת
}Rotary_Handle;

extern Rotary_Handle Rotary;

#endif