#ifndef _ENCODER_H
#define _ENCODER_H

#include "zf_common_headfile.h"

void All_Encoder_Init();
void Get_Encoder_Pulses();
double Get_LF_Speed();
double Get_RF_Speed();
double Get_LB_Speed();
double Get_RB_Speed();
void Get_Distant();
void Encoder_Process();
double Encoder_YawAngle_Get();
extern double Encoder_Distant[4];
extern double Encoer_Speed[4];
#endif