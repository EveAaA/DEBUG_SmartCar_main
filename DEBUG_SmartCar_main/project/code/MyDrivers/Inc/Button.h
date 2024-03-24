#ifndef _BUTTON_H
#define _BUTTON_H

#include "zf_common_headfile.h"
void All_Button_Init();
void All_Button_Scan();
void Get_Button_Value(int8 KeyNum);
void Get_Switch_Button_Value(int8 KeyNum);
void Key_Delay();

extern int8 Button_Value[5];
extern int8 Switch_Button_Value[2];
extern uint16 Key_Time[5];
#endif