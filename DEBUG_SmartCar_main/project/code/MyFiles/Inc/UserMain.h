#ifndef _USERMAIN_H
#define _USERMAIN_H

/*-----------------------MyDrivers---------------------------------*/
#include "Motor.h"
#include "Button.h"
#include "Gyro.h"
#include "Encoder.h"
#include "Bluetooth.h"
#include "PID.h"
#include "Manipulator.h"
#include "Beep.h"
#include "Image.h"
#include "Uart.h"
#include "Rotary.h"
#include "FSM.h"
#include "math.h"
/*-----------------------MyMiddleware------------------------------*/
#include "Car_Control.h"
#include "Menu.h"
#include "Inertial_Navigation.h"
/*-----------------------MyFiles-----------------------------------*/
#include "Call_Back_Action.h"

void User_Init();
void User_Loop();
bool Bufcnt(bool Cond,uint16 Cnt);

extern uint16 Start;
extern uint16 Time_Cnt;
#define Servo_Slow

#endif  