#ifndef _MOTOR_H
#define _MOTOR_H

#include "zf_common_headfile.h"

typedef enum
{
  LMotor_F,
  LMotor_B,
  RMotor_F,
  RMotor_B,
}MotorHandle;

void Motor_Init();
void Set_Motor_Speed(MotorHandle Motor, float PWMDuty);

#endif