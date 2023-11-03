#ifndef _PID_H
#define _PID_H
#include "math.h"
//#include "UserMain.h"
typedef struct
{
	double Kp;//����ϵ��
	double Ki;//΢��ϵ��
	double Kd;//����ϵ��

	double previousError;

	double OutputMin;//�޷�
	double OutputMax;
	
	double Output;
}Pid_TypeDef;

typedef struct
{
	double Kp;//����ϵ��
	double Ki;//΢��ϵ��
	double Kd;//����ϵ��

	double Error;//�������
	double Error_Last;//�ϴ����
	double Error_Pre;//���ϴ����

	double Out_Put_Min;//�޷�
	double Out_Put_Max;
	
	double Output;
}Incremental_PID_TypeDef;

void PIDInit(Pid_TypeDef *PID, double Kp, double Ki, double Kd, double OutputMax, double OutputMin);
double GetPIDValue(Pid_TypeDef *PID, double error);

void Incremental_PID_Init(Incremental_PID_TypeDef *PID, double Kp, double Ki, double Kd, double Out_Put_Max, double Out_Put_Min);
double Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, double Error);
#endif