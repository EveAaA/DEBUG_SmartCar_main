/**
  ******************************************************************************
  * @file    PID.c
  * @author  ׯ�ı�
  * @brief   PID����
  * @date    09/6/2023
    @verbatim
    λ��ʽPID ����ʽPID
    @endverbatim
  * @{
**/


/* Includes ------------------------------------------------------------------*/
#include "PID.h"
#include "math.h"

/* Define\Declare ------------------------------------------------------------*/


/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief     λ��ʽPID��ʼ��
-- @param     Pid_TypeDef *PID PID�ṹ��
-- @param     float Kp �趨��Kp
-- @param     float Ki �趨��Ki
-- @param     float Kd �趨��Kd
-- @param     float outPutMin �޷�
-- @param     float outPutMax �޷�
-- @auther    ׯ�ı�
-- @date      2023/9/6
**/
void PIDInit(Pid_TypeDef *PID, float Kp, float Ki, float Kd, float OutputMax, float OutputMin)
{
	PID->Kp = Kp;
	PID->Ki = Ki;
	PID->Kd = Kd;
	PID->previousError = 0;
	PID->OutputMin = OutputMin;
	PID->OutputMax = OutputMax;
}

/**@brief     λ��ʽPID��ȡֵ
-- @param     Pid_TypeDef *PID PID�ṹ��
-- @param     float error ���ֵ
-- @return    float outPut ���
-- @auther    ׯ�ı�
-- @date      2023/9/6
**/
float GetPIDValue(Pid_TypeDef *PID, float error)
{
	float P = 0.0f, D=0.0f;
	static float I=0.0f;

	P = error;
	I = I + error;
	D = error - PID->previousError;

	PID->Output = PID->Kp * P+ PID->Ki * I+ PID->Kd * D;
	PID->previousError = error;
	
	if(fabs(I) > 100)
	{
		I = 0;
	}	
	if(PID->Output > PID->OutputMax)
	{
		PID->Output = PID->OutputMax;
	}
	else if(PID->Output < PID->OutputMin)
	{
		PID->Output = PID->OutputMin;
	}

	return PID->Output;
}


/**@brief     ����ʽPID��ʼ��
-- @param     Incremental_PID_TypeDef *PID  PID�ṹ��
-- @param     float Kp �趨��Kp
-- @param     float Ki �趨��Ki
-- @param     float Kd �趨��Kd
-- @param     float outPutMin �޷�
-- @param     float outPutMax �޷�
-- @auther    ׯ�ı�
-- @date      2023/9/12
**/
void Incremental_PID_Init(Incremental_PID_TypeDef *PID, float Kp, float Ki, float Kd, float Out_Put_Max, float Out_Put_Min)
{
	PID->Kp = Kp;
	PID->Ki = Ki;
	PID->Kd = Kd;
	PID->Error = 0;
	PID->Error_Last= 0;
	PID->Error_Pre= 0;
	PID->Out_Put_Min = Out_Put_Min;
	PID->Out_Put_Max = Out_Put_Max;
}

/**@brief     ����ʽPID��ȡֵ
-- @param     Incremental_PID_TypeDef *PID PID�ṹ��
-- @param     float Target_Value Ŀ��ֵ
-- @param     float Error ���
-- @return    float Output ���
-- @auther    ׯ�ı�
-- @date   	  2023/9/12
**/
float Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, float Error)
{
	float P = 0.0f, I=0.0f, D=0.0f;
	static float Increment = 0;//����
	
	PID->Error = Error;//��ȡ���
		
	P = PID->Error - PID->Error_Last;//��ǰ���-��һ�����
	I = PID->Error;
	D = PID->Error - 2*PID->Error_Last + PID->Error_Pre;
	
	Increment = PID->Kp*P + PID->Ki*I + PID->Kd*D;

	PID->Error_Pre = PID->Error_Last;
	PID->Error_Last = PID->Error;
	
	PID->Output = PID->Output + Increment;
	//�޷�
	if(PID->Output > PID->Out_Put_Max)
	{
		PID->Output = PID->Out_Put_Max;
	}
	else if(PID->Output < PID->Out_Put_Min)
	{
		PID->Output = PID->Out_Put_Min;
	}

	return PID->Output;
}
