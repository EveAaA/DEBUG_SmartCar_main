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
    .Init_Angle = 140,//角度小往下
    .Servo_Time = 0,
};
Servo_Handle Raise_Servo = //抬手舵机
{
    .Pin = PWM2_MODULE3_CHA_D2,
    .Init_Angle = 120,//角度小往下150
    .Servo_Time = 0,
};
Servo_Handle Rotary_Servo = //旋转舵机
{
    .Pin = PWM1_MODULE3_CHB_B11,
    .Init_Angle = 55,//角度大顺时针转,85白色在前,175黑色在前,265红色在前,355黄色在前，前三个分别代表大类123
                     //185白色门 275黑色门 5红色门 95黄色门 
    .Servo_Time = 0,
};
Servo_Handle Door_Servo = //门电机
{
    .Pin = PWM2_MODULE3_CHB_D3,
    .Init_Angle = 45,//角度大开门 开门角度130  关门角度50
}; 
Servo_Flag_Handle Servo_Flag = {false,false,false,false,false};

/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   设置舵机角度,180度舵机
-- @param   ServoHandle Servo 选择舵机
-- @param   float Angle 设置的角度
-- @author  庄文标
-- @date    2023/11/5
**/
static void Set_Servo_Angle(Servo_Handle Servo,uint16 Angle)
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
static void Manipulator_PutDown()
{
    volatile static uint8 PuDowm_State = 0;
    switch (PuDowm_State)
    {
        case 0://先放下抬手舵机
            Electromagnet_On;//打开电磁铁
            Set_Servo_Angle(Raise_Servo,1);
#ifdef Servo_Slow
            PuDowm_State = 1;
            // Stretch_Servo.Servo_Time = 1;
#else
        PuDowm_State = 2;
#endif
        break;
        case 1://缓冲阶段
            Set_Servo_Angle(Stretch_Servo,60);//120
            if(Bufcnt(true,500))
            {
                PuDowm_State = 2;
            }     
        break;
        case 2:
            Set_Servo_Angle(Stretch_Servo,30);//150
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
static void Manipulator_PutUp()
{
    static uint8 PutUp_State = 0;
    switch (PutUp_State)
    {
        case 0:
            Set_Servo_Angle(Stretch_Servo,60);//先抬大臂 120
            PutUp_State = 1;
        break;
        case 1:
            if(Bufcnt(true,400))//这是为了留给机械臂运转的时间
            {
                Set_Servo_Angle(Raise_Servo,135);//把卡片翻起来
                PutUp_State = 2;
            }
        break;
        case 2:
            if(Bufcnt(true,400))
            {
                Set_Servo_Angle(Stretch_Servo,138);//大臂直接放入 30
                PutUp_State = 3;
            }
        break;
        case 3:
            if(Bufcnt(true,400))
            {    
                PutUp_State = 0;
                Electromagnet_Off;
                Servo_Flag.Put_Up = true;                
                Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle);
                Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle);
            }
        break;
    }
    // printf("%d\r\n",PutUp_State);
}


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
    pwm_init(Door_Servo.Pin,Servo_FREQ,Set_180Servo_Angle(Door_Servo.Init_Angle));
    gpio_init(D27,GPO,0,GPO_PUSH_PULL);
}



/**@brief   机械臂捡起卡片
-- @param   无
-- @author  庄文标
-- @date    2023/11/5
**/
void Pick_Card()
{
    volatile static uint8 Pickup_State = 0;
    // printf("PS = %d\r\n",Pickup_State);
    switch(Pickup_State)
    {
        case 0:
            if(!Servo_Flag.Put_Down)//机械臂放下
            {
                Manipulator_PutDown(); 
            }
            else
            {
                if(Bufcnt(true,500))
                {
                    Pickup_State = 1;
                }
            }
        break;
        case 1:
            if(Servo_Flag.Put_Up == false)//把卡片放入仓库
            {
                Manipulator_PutUp();
            }
            else
            {
                Servo_Flag.Pick_End = true;
                Pickup_State = 0;
            }
        break;
    }
}

/**@brief   躲避卡片
-- @param   无
-- @author  庄文标
-- @date    2024/6/2
**/
void Dodge_Board()
{
    Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle - 30);//抬起机械臂以免卡到卡片
    Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle - 30);
}

/**@brief   躲避卡片
-- @param   无
-- @author  庄文标
-- @date    2024/6/2
**/
void Dodge_Carmar()
{
    Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle);//抬起机械臂以免卡到卡片
    Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle);
}

/**@brief   设置转盘舵机转到哪个地方
-- @param   Rotaryservo_Handle RotaryServo 选择颜色
-- @param   uint16 Rotary_Speed 速度
-- @author  庄文标
-- @date    2024/5/6
**/
void Rotary_Switch(Rotaryservo_Handle RotaryServo,uint16 Rotary_Speed)
{
    static uint16 Cur_Depot = 55;
    static uint16 Tar_Depot = 0;
    static uint16 Set_Angle = 55;
    static uint16 Percent = 1;
    if(RotaryServo <= 4)
    {
        Tar_Depot = RotaryServo*72 + 55;
    }
    // else if(RotaryServo == White_Door)
    // {
    //     Tar_Depot = 185;
    // }
    // else if(RotaryServo == Black_Door)
    // {
    //     Tar_Depot = 275;
    // }
    // else if(RotaryServo == Red_Door)
    // {
    //     Tar_Depot = 359;
    // }
                      

    if(Tar_Depot!=Cur_Depot)
    {
        if(abs(Tar_Depot - Cur_Depot) > 72 && abs(Tar_Depot - Cur_Depot) <= 144)
        {
            Rotary_Speed *=2;
        }
        else if(abs(Tar_Depot - Cur_Depot) > 144 && abs(Tar_Depot - Cur_Depot) <= 216)
        {
            Rotary_Speed *=3;
        }
        else if(abs(Tar_Depot - Cur_Depot) > 216)
        {
            Rotary_Speed *=4;
        }
        Set_Angle = Cur_Depot + (Tar_Depot - Cur_Depot)*(Percent/(float)Rotary_Speed);
        Percent +=1;
        Servo_Flag.Depot_End = false;
    }

    if((Percent >= Rotary_Speed) && (RotaryServo <= Yellow))
    {
        Cur_Depot = Tar_Depot;
        Percent = 1;
        Servo_Flag.Depot_End = true;
    }
    else if((Percent >= Rotary_Speed) && (RotaryServo >= White_Door))
    {
        Cur_Depot = Tar_Depot;
        Percent = 1;
        Servo_Flag.Depot_End = true;
        Set_Servo_Angle(Door_Servo,130);          
    }
    Set_Servo_Angle(Rotary_Servo,Set_Angle); 
}

/**@brief   把卡片放入指定仓库
-- @param   无
-- @author  庄文标
-- @date    2024/4/30
**/
void Put_Depot(int8 Card_Class)
{
    volatile static uint8 Depot_State = 0;
    // printf("PD = %d\r\n",Depot_State);
    switch(Depot_State)
    {
        case 0:
            // Rotary_Switch(Card_Class);
            if(Bufcnt(true,1500))
            {
                Depot_State = 1;
            }
        break;
        case 1:
            Set_Servo_Angle(Stretch_Servo,150);//大臂直接放入 30
            Depot_State = 2;
        break;
        case 2:
            if(Bufcnt(true,200))
            {    
                Depot_State = 0;
                Electromagnet_Off;
                Servo_Flag.Put_Depot = true;
                Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle);
                Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle);
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
            Set_Servo_Angle(Stretch_Servo,40);//140
            Electromagnet_On;
            Out_State = 1;
        break;
        case 1:
            if(Bufcnt(true,1000))
            {
                Set_Servo_Angle(Raise_Servo,145);
                Out_State = 4;
            }
        break;
        case 4://吸卡片
            Set_Servo_Angle(Stretch_Servo,150);//15
            Out_State = 5;           
        break;
        case 5://把卡片拿出来
            if(Bufcnt(true,500))
            {
                Set_Servo_Angle(Raise_Servo,135);
                Out_State = 6;
            }            
        break;
        case 6://把卡片拿出来
            if(Bufcnt(true,500))
            {
                Set_Servo_Angle(Stretch_Servo,60);//120
                Out_State = 7;
            }
        break;
        case 7://把卡片翻下来
            if(Bufcnt(true,500))
            {
                Set_Servo_Angle(Raise_Servo,0);
                Out_State = 8;
            }
        break;
        case 8://把卡片放下
            if(Bufcnt(true,375))
            {
                Set_Servo_Angle(Stretch_Servo,30);//150
                Out_State = 9;
            }
        break;
        case 9://电磁铁消磁
            if(Bufcnt(true,375))
            {
                Electromagnet_Off;
                Out_State = 0;
                Servo_Flag.Put_Out = true;
                Dodge_Board();
            }
        break;
    }
}