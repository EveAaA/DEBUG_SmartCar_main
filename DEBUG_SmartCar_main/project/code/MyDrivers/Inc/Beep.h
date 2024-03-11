#ifndef _BEEP_H
#define _BEEP_H

#include "zf_common_headfile.h"

typedef enum
{
    On,//?
    Off,//?
}Beep_Mode;

void Beep_Init();
void Beep(Beep_Mode Mode);

#endif