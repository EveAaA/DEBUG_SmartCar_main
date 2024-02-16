#ifndef _PID_H
#define _PID_H

typedef struct
{
	float Kp;//����ϵ��
	float Ki;//΢��ϵ��
	float Kd;//����ϵ��
}Pid_Par_Handle;

typedef struct
{
	Pid_Par_Handle (*Pid_Par);//Pid����
	float previousError;

	float OutputMin;//�޷�
	float OutputMax;
	
	float Output;
}Pid_TypeDef;

typedef struct
{
	float Kp;//����ϵ��
	float Ki;//΢��ϵ��
	float Kd;//����ϵ��
}Incremental_PID_Par_Handle;

typedef struct
{
	Incremental_PID_Par_Handle (*Inc_PID_Par);

	float Error;//�������
	float Error_Last;//�ϴ����
	float Error_Pre;//���ϴ����

	float Out_Put_Min;//�޷�
	float Out_Put_Max;
	
	float Output;
}Incremental_PID_TypeDef;

void PIDInit(Pid_TypeDef *PID, float Kp, float Ki, float Kd, float OutputMax, float OutputMin);
float GetPIDValue(Pid_TypeDef *PID, float error);

void Incremental_PID_Init(Incremental_PID_TypeDef *PID, float Kp, float Ki, float Kd, float Out_Put_Max, float Out_Put_Min);
float Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, float Error);
#endif