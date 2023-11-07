#ifndef _PID_H
#define _PID_H
#define KP 0
#define KI 1
#define KD 2

typedef struct
{
	double Kp;//比例系数
	double Ki;//微分系数
	double Kd;//积分系数

	double previousError;

	double OutputMin;//限幅
	double OutputMax;
	
	double Output;
}Pid_TypeDef;

typedef struct
{
	double Kp;//比例系数
	double Ki;//微分系数
	double Kd;//积分系数

	double Error;//本次误差
	double Error_Last;//上次误差
	double Error_Pre;//上上次误差

	double Out_Put_Min;//限幅
	double Out_Put_Max;
	
	double Output;
}Incremental_PID_TypeDef;

extern float LF_Parameter[3];
extern float RF_Parameter[3];
extern float LB_Parameter[3];
extern float RB_Parameter[3];

void PIDInit(Pid_TypeDef *PID, double Kp, double Ki, double Kd, double OutputMax, double OutputMin);
double GetPIDValue(Pid_TypeDef *PID, double error);

void Incremental_PID_Init(Incremental_PID_TypeDef *PID, double Kp, double Ki, double Kd, double Out_Put_Max, double Out_Put_Min);
double Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, double Error);
#endif