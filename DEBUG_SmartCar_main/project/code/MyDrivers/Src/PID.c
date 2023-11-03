/**
  ******************************************************************************
  * @file    PID.c
  * @author  庄文标
  * @brief   PID驱动
  * @date    09/6/2023
    @verbatim
    位置式PID 增量式PID
    @endverbatim
  * @{
**/


/* Includes ------------------------------------------------------------------*/
#include "PID.h"


/* Define\Declare ------------------------------------------------------------*/


/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief     位置式PID初始化
-- @param     Pid_TypeDef *PID PID结构体
-- @param     double Kp 设定的Kp
-- @param     double Ki 设定的Ki
-- @param     double Kd 设定的Kd
-- @param     double outPutMin 限幅
-- @param     double outPutMax 限幅
-- @auther    庄文标
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

/**@brief     位置式PID获取值
-- @param     Pid_TypeDef *PID PID结构体
-- @param     double error 误差值
-- @return    double outPut 输出
-- @auther    庄文标
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


/**@brief     增量式PID初始化
-- @param     Incremental_PID_TypeDef *PID  PID结构体
-- @param     double Kp 设定的Kp
-- @param     double Ki 设定的Ki
-- @param     double Kd 设定的Kd
-- @param     double outPutMin 限幅
-- @param     double outPutMax 限幅
-- @auther    庄文标
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

/**@brief     增量式PID获取值
-- @param     Incremental_PID_TypeDef *PID PID结构体
-- @param     double Target_Value 目标值
-- @param     double Current_Value 当前值
-- @return    double Output 输出
-- @auther    庄文标
-- @date   	  2023/9/12
**/
double Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, double Error)
{
	double P = 0.0, I=0.0 , D=0.0;
	static double Increment = 0;//增量
	
	PID->Error = Error;//获取误差
		
	P = PID->Error - PID->Error_Last;//当前误差-上一次误差
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
	//限幅
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
