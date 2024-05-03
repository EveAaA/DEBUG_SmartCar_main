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
#include "math.h"

/* Define\Declare ------------------------------------------------------------*/


/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief     位置式PID初始化
-- @param     Pid_TypeDef *PID PID结构体
-- @param     float Kp 设定的Kp
-- @param     float Ki 设定的Ki
-- @param     float Kd 设定的Kd
-- @param     float outPutMin 限幅
-- @param     float outPutMax 限幅
-- @author    庄文标
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

/**@brief     位置式PID获取值
-- @param     Pid_TypeDef *PID PID结构体
-- @param     float error 误差值
-- @return    float outPut 输出
-- @author    庄文标
-- @date      2023/9/6
**/
float GetPIDValue(Pid_TypeDef *PID, float error)
{
	float P = 0.0f, D=0.0f;
	static float I=0.0f;

	P = error;
	if(fabs(error) <= 2)
	{
		PID->I_Out = PID->I_Out + error;
	}
	else
	{
		PID->I_Out = 0;
	}
	D = error - PID->previousError;

	PID->Output = PID->Kp * P+ PID->Ki * PID->I_Out+ PID->Kd * D;
	PID->previousError = error;
	
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
-- @param     float Kp 设定的Kp
-- @param     float Ki 设定的Ki
-- @param     float Kd 设定的Kd
-- @param     float outPutMin 限幅
-- @param     float outPutMax 限幅
-- @author    庄文标
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

/**@brief     增量式PID获取值
-- @param     Incremental_PID_TypeDef *PID PID结构体
-- @param     float Target_Value 目标值
-- @param     float Error 误差
-- @return    float Output 输出
-- @author    庄文标
-- @date   	  2023/9/12
**/
float Get_Incremental_PID_Value(Incremental_PID_TypeDef *PID, float Error)
{
	float P = 0.0f, I=0.0f, D=0.0f;
	static float Increment = 0;//增量
	
	PID->Error = Error;//获取误差
		
	P = PID->Error - PID->Error_Last;//当前误差-上一次误差
	I = PID->Error;
	D = PID->Error - 2*PID->Error_Last + PID->Error_Pre;
	
	Increment = PID->Kp*P + PID->Ki*I + PID->Kd*D;

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
