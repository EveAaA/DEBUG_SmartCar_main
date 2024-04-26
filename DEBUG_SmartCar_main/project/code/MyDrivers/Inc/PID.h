#ifndef _PID_H
#define _PID_H

typedef struct
{
	float Kp;//比例系数
	float Ki;//微分系数
	float Kd;//积分系数

	float previousError;
	float I_Out;
	float OutputMin;//限幅
	float OutputMax;
	
	float Output;
}Pid_TypeDef;

typedef struct
{
	float Kp;//比例系数
	float Ki;//微分系数
	float Kd;//积分系数

	float Error;//本次误差
	float Error_Last;//上次误差
	float Error_Pre;//上上次误差

	float Out_Put_Min;//限幅
	float Out_Put_Max;
	
	float Output;
}Incremental_PID_TypeDef;

void PIDInit(Pid_TypeDef *PID, float Kp, float Ki, float Kd, float OutputMax, float OutputMin);
float GetPIDValue(Pid_TypeDef *PID, float error);

void Incremental_PID_Init(Incremental_PID_TypeDef *PID, float Kp, float Ki, float Kd, float Out_Put_Max, float Out_Put_Min);
float Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, float Error);
#endif