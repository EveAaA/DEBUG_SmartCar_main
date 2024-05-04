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
    .Pin = PWM1_MODULE3_CHA_D0,
    .Init_Angle = 40,//角度大往下
    .Servo_Time = 0,
};
Servo_Handle Raise_Servo = //抬手舵机
{
    .Pin = PWM2_MODULE3_CHA_D2,
    .Init_Angle = 150,//角度小往下
    .Servo_Time = 0,
};
Servo_Handle Rotary_Servo = //旋转舵机
{
    .Pin = PWM1_MODULE3_CHB_B11,
    .Init_Angle = 102//角度大逆时针转
};
Servo_Handle Door_Servo = //门电机
{
    .Pin = PWM2_MODULE3_CHB_D3,
    .Init_Angle = 90,//角度大逆时针转
}; 
Servo_Flag_Handle Servo_Flag = {false,false,false,false};

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   机械臂初始化
-- @param   无
-- @author  庄文标
-- @date    2023/11/5
**/
void Manipulator_Init()
{
    pwm_init(Stretch_Servo.Pin,Servo_FREQ,Set_180Servo_Angle(Stretch_Servo.Init_Angle));
    pwm_init(Raise_Servo.Pin,Servo_FREQ,Set_180Servo_Angle(Raise_Servo.Init_Angle));
    pwm_init(Rotary_Servo.Pin,Servo_FREQ,Set_360Servo_Angle(Rotary_Servo.Init_Angle));
    // pwm_init(Door_Servo.Pin,Servo_FREQ,Set_180Servo_Angle(Door_Servo.Init_Angle));
    gpio_init(D27,GPO,0,GPO_PUSH_PULL);
}

/**@brief   设置舵机角度,180度舵机
-- @param   ServoHandle Servo 选择舵机
-- @param   float Angle 设置的角度
-- @author  庄文标
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
    if(Servo.Pin == Rotary_Servo.Pin)//如果是360度舵机
    {
        pwm_set_duty(Servo.Pin,Set_360Servo_Angle(Servo.Set_Angle));
    }
    else
    {
        pwm_set_duty(Servo.Pin,Set_180Servo_Angle(Servo.Set_Angle));
    }   
}

/**@brief   机械臂放下拿卡片动作
-- @param   无
-- @author  庄文标
-- @date    2024/11/5
**/
void Manipulator_PutDown()
{
    static uint8 PuDowm_State = 0;
    switch (PuDowm_State)
    {
        case 0://先放下抬手舵机
            Set_Servo_Angle(Raise_Servo,0);
            Electromagnet_On;//打开电磁铁
#ifdef Servo_Slow
            PuDowm_State = 1;
            Stretch_Servo.Servo_Time = 1;
#else
        PuDowm_State = 2;
#endif
        break;
        case 1://缓冲阶段
            Set_Servo_Angle(Stretch_Servo,120);
            // printf("%d\r\n",timer_get(GPT_TIM_1));     
            if(Stretch_Servo.Servo_Time >= 100)
            {
                PuDowm_State = 2;
                Stretch_Servo.Servo_Time = 0;
            }
        break;
        case 2:
            Set_Servo_Angle(Stretch_Servo,150);
            PuDowm_State = 0;
            Servo_Flag.Put_Down = true;
        break;
    }
}

/**@brief   机械臂抬起
-- @param   无
-- @author  庄文标
-- @date    2023/11/5
**/
void Manipulator_PutUp()
{
    static uint8 PutUp_State = 0;
    switch (PutUp_State)
    {
        case 0:
            Set_Servo_Angle(Stretch_Servo,120);//先抬大臂
            Stretch_Servo.Servo_Time = 1;
            PutUp_State = 1;
        break;
        case 1:
            if(Stretch_Servo.Servo_Time >= 100)
            {
                Set_Servo_Angle(Raise_Servo,140);
                Stretch_Servo.Servo_Time = 0;
                PutUp_State = 2;
            }
        break;
        case 2:
            PutUp_State = 0;
            Servo_Flag.Put_Up = true;
            // Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle);
            // Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle);
        break;
    }
    // printf("%d\r\n",PutUp_State);
}


/**@brief   机械臂捡起卡片
-- @param   无
-- @author  庄文标
-- @date    2023/11/5
**/
void Pick_Card()
{
    static uint8 Pickup_State = 0;
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
            if(Raise_Servo.Servo_Time > 100)
            {
                Raise_Servo.Servo_Time = 0;
                Pickup_State = 2;
            }
        break;
        case 2:
            if(!Servo_Flag.Put_Up)
            {
                Manipulator_PutUp();
            }
            else
            {
                if(Raise_Servo.Servo_Time == 0)
                {
                    Raise_Servo.Servo_Time = 1;
                }
                if(Raise_Servo.Servo_Time >= 200)
                {
                    Raise_Servo.Servo_Time = 0;
                    Pickup_State = 3;
                }
            }
        break;
        case 3:
            Put_Depot();
            if(Servo_Flag.Put_Depot)
            {
                Pickup_State = 0;
            }
        break;
    }

}

/**@brief   把卡片放入指定仓库
-- @param   无
-- @author  庄文标
-- @date    2024/4/30
**/
void Put_Depot()
{
    static uint8 Depot_State = 0;
    switch(Depot_State)
    {
        case 0:
            Set_Servo_Angle(Stretch_Servo,30);
            Stretch_Servo.Servo_Time = 1;
            Depot_State = 1;
        break;
        case 1:
            if(Stretch_Servo.Servo_Time>=50)
            {   
                Stretch_Servo.Servo_Time = 0;
                Depot_State = 0;
                Electromagnet_Off;
                Servo_Flag.Put_Depot = true;
            }
        break;
    }
}

/**@brief   把卡片从仓库里面拿出来
-- @param   无
-- @author  庄文标
-- @date    2024/4/30
**/
void Take_Card_Out()
{
    static uint8 Out_State = 0;
    switch (Out_State)
    {
        case 0:
            Electromagnet_On;
            Set_Servo_Angle(Raise_Servo,140);
            Out_State = 1;
        break;
        case 1://吸卡片
            Set_Servo_Angle(Stretch_Servo,15);
            Stretch_Servo.Servo_Time = 1;
            Out_State = 2;
        break;
        case 2://把卡片拿出来
            if(Stretch_Servo.Servo_Time >= 100)
            {
                Set_Servo_Angle(Stretch_Servo,120);
                Stretch_Servo.Servo_Time = 1;
                Out_State = 3;
            }
        break;
        case 3://把卡片翻下来
            if(Stretch_Servo.Servo_Time >= 100)
            {
                Stretch_Servo.Servo_Time = 0;
                Set_Servo_Angle(Raise_Servo,0);
                Raise_Servo.Servo_Time = 1;
                Out_State = 4;
            }
        break;
        case 4:
            if(Raise_Servo.Servo_Time >= 75)
            {
                Raise_Servo.Servo_Time = 0;
                Stretch_Servo.Servo_Time = 1;
                Set_Servo_Angle(Stretch_Servo,150);
                Out_State = 5;
            }
        break;
        case 5:
            if(Stretch_Servo.Servo_Time >= 75)
            {
                Electromagnet_Off;
                Out_State = 0;
                Stretch_Servo.Servo_Time = 0;
                Servo_Flag.Put_Out = true;
            }
        break;
    }
}