#ifndef _BEEP_H
#define _BEEP_H

#include "zf_common_headfile.h"

typedef enum
{
    On,//常开
    Off,//常关
}Beep_Handle;

void Beep_Init();
void Beep(Beep_Handle Mode);

#endif