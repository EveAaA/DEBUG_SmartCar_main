#ifndef _ENCODER_H
#define _ENCODER_H

#include "zf_common_headfile.h"

void All_Encoder_Init();
void Encoder_Process();
float Encoder_YawAngle_Get();
float Get_X_Distance();
float Get_Y_Distance();
float Get_X_Speed();
float Get_Y_Speed();
extern float Encoder_Distance[4];
extern float Encoer_Speed[4];
#endif