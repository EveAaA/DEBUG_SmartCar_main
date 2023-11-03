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


/* Define\Declare ------------------------------------------------------------*/


/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief     λ��ʽPID��ʼ��
-- @param     Pid_TypeDef *PID PID�ṹ��
-- @param     double Kp �趨��Kp
-- @param     double Ki �趨��Ki
-- @param     double Kd �趨��Kd
-- @param     double outPutMin �޷�
-- @param     double outPutMax �޷�
-- @auther    ׯ�ı�
-- @date      2023/9/6
**/
void PIDInit(Pid_TypeDef *PID, double Kp, double Ki, double Kd, double OutputMax, double OutputMin)
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
-- @param     double error ���ֵ
-- @return    double outPut ���
-- @auther    ׯ�ı�
-- @date      2023/9/6
**/
double GetPIDValue(Pid_TypeDef *PID, double error)
{
	double P = 0.0, D=0.0;
	static double I=0.0;

	P = error;
	I = I + error;
	D = error - PID->previousError;

	PID->Output = PID->Kp * P+ PID->Ki * I+ PID->Kd * D;
	PID->previousError = error;
	
	if(fabs(I) > 1000000)
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
-- @param     double Kp �趨��Kp
-- @param     double Ki �趨��Ki
-- @param     double Kd �趨��Kd
-- @param     double outPutMin �޷�
-- @param     double outPutMax �޷�
-- @auther    ׯ�ı�
-- @date      2023/9/12
**/
void Incremental_PID_Init(Incremental_PID_TypeDef *PID, double Kp, double Ki, double Kd, double Out_Put_Max, double Out_Put_Min)
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
-- @param     double Target_Value Ŀ��ֵ
-- @param     double Current_Value ��ǰֵ
-- @return    double Output ���
-- @auther    ׯ�ı�
-- @date   	  2023/9/12
**/
double Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, double Error)
{
	double P = 0.0, I=0.0 , D=0.0;
	static double Increment = 0;//����
	
	PID->Error = Error;//��ȡ���
		
	P = PID->Error - PID->Error_Last;//��ǰ���-��һ�����
	I = PID->Error;
	D = PID->Error - 2*PID->Error_Last + PID->Error_Pre;
		
	
	if(fabs(Error) > 1)
	{
		Increment = PID->Kp*P + PID->Ki*I + PID->Kd*D;
	}
	else
	{
		Increment = 0;
	}

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
