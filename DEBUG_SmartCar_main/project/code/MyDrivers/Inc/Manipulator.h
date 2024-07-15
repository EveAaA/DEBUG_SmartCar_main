#ifndef _MANIPULATOR_H
#define _MANIPULATOR_H

#include "zf_common_headfile.h"
#define Light_On gpio_set_level(D1,1)
#define Light_Off gpio_set_level(D1,0)

typedef struct 
{
    uint8_t Pin;//对应引脚
    float Max_Angle;//最大角度
    float Min_Angle;//最小角度
    float Init_Angle;//初始化角度
    float Set_Angle;//控制的时候设置的角度
    volatile uint16 Servo_Time;
}Servo_Handle;

typedef struct
{
    volatile bool Put_Down;//机械臂放下结束
    volatile bool Put_Up;//机械臂抬起结束
    volatile bool Put_Depot;
    volatile bool Put_Out;//机械臂拿出结束
    volatile bool Pick_End;//捡起结束
    volatile bool Depot_End;//转盘转动结束
    volatile bool Open_Door;//开门
    volatile bool Door_End;//开门结束
}Servo_Flag_Handle;

typedef enum
{
    No_Depot = 8,
    White = 0,
    Black = 1,
    Red = 2,
    Yellow = 3,
    White_Door,
    Black_Door,
    Red_Door,
    Yellow_Door,
}Rotaryservo_Handle;

void Manipulator_Init();
void Pick_Card();
void Dodge_Board();
void Dodge_Carmar();
void Rotary_Switch(Rotaryservo_Handle RotaryServo,uint16 Rotary_Speed);
void Put_Depot(int8 Card_Class);
void Take_Card_Out();
void Open_Door(bool Open_Or);
void Close_Door();

extern Servo_Handle Raise_Servo;
extern Servo_Handle Stretch_Servo;
extern Servo_Handle Rotary_Servo;
extern Servo_Flag_Handle Servo_Flag;
#endif