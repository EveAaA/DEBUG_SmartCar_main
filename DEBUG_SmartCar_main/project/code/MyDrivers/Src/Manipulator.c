/**
  ******************************************************************************
  * @file    Manipulator.c
  * @author  庄文标
  * @brief   机械臂驱动
  * @date    11/5/2023
    @verbatim
    包含两个舵机和电磁铁
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Manipulator.h"
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define Servo_FREQ 330

#define Set_180Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 90.0))//设置180度舵机角度转为具体占空比
#define Set_360Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 180.0))//设置360度舵机角度转为具体占空比
#define Electromagnet_On gpio_set_level(D27,1)
#define Electromagnet_Off gpio_set_level(D27,0)


Servo_Handle Stretch_Servo = //抬臂舵机
{
    .Pin = PWM2_MODULE0_CHA_C6,
    .Init_Angle = 150,//角度小往下
    .Servo_Time = 0,
};
Servo_Handle Raise_Servo = //抬手舵机
{
    .Pin = PWM1_MODULE3_CHB_B11,
    .Init_Angle = 50,//角度小往下
    .Servo_Time = 0,
};
Servo_Handle Rotary_Servo = //旋转舵机
{
    .Pin = PWM1_MODULE3_CHA_D0,
    .Init_Angle = 65,//角度小往下
};
Servo_Flag_Handle Servo_Flag = {false,false};

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   机械臂初始化
-- @param   无
-- @auther  庄文标
-- @date    2023/11/5
**/
void Manipulator_Init()
{
    pwm_init(Stretch_Servo.Pin,Servo_FREQ,Set_180Servo_Angle(Stretch_Servo.Init_Angle));
    pwm_init(Raise_Servo.Pin,Servo_FREQ,Set_180Servo_Angle(Raise_Servo.Init_Angle));
    pwm_init(Rotary_Servo.Pin,Servo_FREQ,Set_360Servo_Angle(Rotary_Servo.Init_Angle));
    // pwm_init(PWM2_MODULE2_CHA_C10,Servo_FREQ,6000);
    gpio_init(D27,GPO,0,GPO_PUSH_PULL);
}

/**@brief   设置舵机角度,180度舵机
-- @param   ServoHandle Servo 选择舵机
-- @param   float Angle 设置的角度
-- @auther  庄文标
-- @date    2023/11/5
**/
void Set_Servo_Angle(Servo_Handle Servo,float Angle)
{
    Servo.Set_Angle = Angle;
    // if(Angle > Servo.Max_Angle)//限幅
    // {
    //     Servo.Set_Angle = Servo.Max_Angle;
    // }
    // else if(Angle < Servo.Min_Angle)
    // {
    //     Servo.Set_Angle = Servo.Min_Angle;
    // }

    pwm_set_duty(Servo.Pin,Set_180Servo_Angle(Servo.Set_Angle));
}

/**@brief   机械臂放下拿卡片动作
-- @param   无
-- @auther  庄文标
-- @date    2024/11/5
**/
void Manipulator_PutDown()
{
    static uint8 PuDowm_State = 0;
    switch (PuDowm_State)
    {
        case 0://先放下抬手舵机
            Set_Servo_Angle(Raise_Servo,110);
            Electromagnet_On;//打开电磁铁
#ifdef Servo_Slow
            PuDowm_State = 1;
            Stretch_Servo.Servo_Time = 1;
#else
        PuDowm_State = 2;
#endif
        break;
        case 1://缓冲阶段
            Set_Servo_Angle(Stretch_Servo,30);
            // printf("%d\r\n",timer_get(GPT_TIM_1));     
            if(Stretch_Servo.Servo_Time >= 100)
            {
                PuDowm_State = 2;
                Stretch_Servo.Servo_Time = 0;
            }
        break;
        case 2:
            Set_Servo_Angle(Stretch_Servo,5);
            PuDowm_State = 0;
            Servo_Flag.Put_Down = true;
        break;
    }
}

/**@brief   机械臂抬起
-- @param   无
-- @auther  庄文标
-- @date    2023/11/5
**/
void Manipulator_PutUp()
{
    static uint8 PutUp_State = 0;
    switch (PutUp_State)
    {
        case 0:
            Set_Servo_Angle(Stretch_Servo,60);//先抬大臂
            PutUp_State = 1;
        break;
        case 1:
            Set_Servo_Angle(Raise_Servo,180);
            Electromagnet_Off;
            PutUp_State = 0;
            Servo_Flag.Put_Up = true;
        break;
    }
}

extern float Pick_Angle;

/**@brief   机械臂捡起卡片
-- @param   无
-- @auther  庄文标
-- @date    2023/11/5
**/
void Pick_Card()
{
    static uint8 Pickup_State = 0;
    Set_Car_Speed(0,0,Angle_Control(Pick_Angle));
    switch(Pickup_State)
    {
        case 0:
            if(!Servo_Flag.Put_Down)
            {
                Manipulator_PutDown();
            }
            else
            {
                Pickup_State = 1;
                Raise_Servo.Servo_Time = 1;
            }
        break;
        case 1:
            if(Raise_Servo.Servo_Time > 200)
            {
                Raise_Servo.Servo_Time = 0;
                Pickup_State = 2;
            }
        break;
        case 2:
            Manipulator_PutUp();
            Pickup_State = 0;
        break;
    }

}