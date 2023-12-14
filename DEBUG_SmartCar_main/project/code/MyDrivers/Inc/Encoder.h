#ifndef _ENCODER_H
#define _ENCODER_H

#include "zf_common_headfile.h"

void All_Encoder_Init();
void Get_Encoder_Pulses();
double Get_LF_Speed();
double Get_RF_Speed();
double Get_LB_Speed();
double Get_RB_Speed();
void Get_Distance();
void Encoder_Process();
double Encoder_YawAngle_Get();
extern double Encoder_Distance[4];
extern double Encoer_Speed[4];
double Get_X_Distance();
double Get_Y_Distance();
#endif