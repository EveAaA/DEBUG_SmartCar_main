/**
  ******************************************************************************
  * @file    Manipulator.c
  * @author  ׯ�ı�
  * @brief   ��е������
  * @date    11/5/2023
    @verbatim
    ������������͵����
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Manipulator.h"
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define Servo_FREQ 330

#define Set_180Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 90.0))//����180�ȶ���Ƕ�תΪ����ռ�ձ�
#define Set_360Servo_Angle(angle) ((float)PWM_DUTY_MAX / (1000.0 / (float)Servo_FREQ) * (0.5 + (float)(angle) / 180.0))//����360�ȶ���Ƕ�תΪ����ռ�ձ�
#define Electromagnet_On gpio_set_level(D27,1)
#define Electromagnet_Off gpio_set_level(D27,0)


Servo_Handle Stretch_Servo = //̧�۶��
{
    .Pin = PWM1_MODULE3_CHA_D0,
    .Init_Angle = 140,//�Ƕ�С����
    .Servo_Time = 0,
};
Servo_Handle Raise_Servo = //̧�ֶ��
{
    .Pin = PWM2_MODULE3_CHA_D2,
    .Init_Angle = 120,//�Ƕ�С����150
    .Servo_Time = 0,
};
Servo_Handle Rotary_Servo = //��ת���
{
    .Pin = PWM1_MODULE3_CHB_B11,
    .Init_Angle = 55,//�Ƕȴ�˳ʱ��ת,85��ɫ��ǰ,175��ɫ��ǰ,265��ɫ��ǰ,355��ɫ��ǰ��ǰ�����ֱ�������123
                     //185��ɫ�� 275��ɫ�� 5��ɫ�� 95��ɫ�� 
    .Servo_Time = 0,
};
Servo_Handle Door_Servo = //�ŵ��
{
    .Pin = PWM2_MODULE3_CHB_D3,
    .Init_Angle = 45,//�Ƕȴ��� ���ŽǶ�130  ���ŽǶ�50
}; 
Servo_Flag_Handle Servo_Flag = {false,false,false,false,false};

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   ���ö���Ƕ�,180�ȶ��
-- @param   ServoHandle Servo ѡ����
-- @param   float Angle ���õĽǶ�
-- @author  ׯ�ı�
-- @date    2023/11/5
**/
static void Set_Servo_Angle(Servo_Handle Servo,uint16 Angle)
{
    Servo.Set_Angle = Angle;
    // if(Angle > Servo.Max_Angle)//�޷�
    // {
    //     Servo.Set_Angle = Servo.Max_Angle;
    // }
    // else if(Angle < Servo.Min_Angle)
    // {
    //     Servo.Set_Angle = Servo.Min_Angle;
    // }
    if(Servo.Pin == Rotary_Servo.Pin)//�����360�ȶ��
    {
        pwm_set_duty(Servo.Pin,Set_360Servo_Angle(Servo.Set_Angle));
    }
    else
    {
        pwm_set_duty(Servo.Pin,Set_180Servo_Angle(Servo.Set_Angle));
    }   
}

/**@brief   ��е�۷����ÿ�Ƭ����
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/11/5
**/
static void Manipulator_PutDown()
{
    volatile static uint8 PuDowm_State = 0;
    switch (PuDowm_State)
    {
        case 0://�ȷ���̧�ֶ��
            Electromagnet_On;//�򿪵����
            Set_Servo_Angle(Raise_Servo,1);
#ifdef Servo_Slow
            PuDowm_State = 1;
            // Stretch_Servo.Servo_Time = 1;
#else
        PuDowm_State = 2;
#endif
        break;
        case 1://����׶�
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

/**@brief   ��е��̧��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/5
**/
static void Manipulator_PutUp()
{
    static uint8 PutUp_State = 0;
    switch (PutUp_State)
    {
        case 0:
            Set_Servo_Angle(Stretch_Servo,60);//��̧��� 120
            PutUp_State = 1;
        break;
        case 1:
            if(Bufcnt(true,400))//����Ϊ��������е����ת��ʱ��
            {
                Set_Servo_Angle(Raise_Servo,135);//�ѿ�Ƭ������
                PutUp_State = 2;
            }
        break;
        case 2:
            if(Bufcnt(true,400))
            {
                Set_Servo_Angle(Stretch_Servo,138);//���ֱ�ӷ��� 30
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
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��е�۳�ʼ��
-- @param   ��
-- @author  ׯ�ı�
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



/**@brief   ��е�ۼ���Ƭ
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/5
**/
void Pick_Card()
{
    volatile static uint8 Pickup_State = 0;
    // printf("PS = %d\r\n",Pickup_State);
    switch(Pickup_State)
    {
        case 0:
            if(!Servo_Flag.Put_Down)//��е�۷���
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
            if(Servo_Flag.Put_Up == false)//�ѿ�Ƭ����ֿ�
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

/**@brief   ��ܿ�Ƭ
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/6/2
**/
void Dodge_Board()
{
    Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle - 30);//̧���е�����⿨����Ƭ
    Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle - 30);
}

/**@brief   ��ܿ�Ƭ
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/6/2
**/
void Dodge_Carmar()
{
    Set_Servo_Angle(Stretch_Servo,Stretch_Servo.Init_Angle);//̧���е�����⿨����Ƭ
    Set_Servo_Angle(Raise_Servo,Raise_Servo.Init_Angle);
}

/**@brief   ����ת�̶��ת���ĸ��ط�
-- @param   Rotaryservo_Handle RotaryServo ѡ����ɫ
-- @param   uint16 Rotary_Speed �ٶ�
-- @author  ׯ�ı�
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

/**@brief   �ѿ�Ƭ����ָ���ֿ�
-- @param   ��
-- @author  ׯ�ı�
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
            Set_Servo_Angle(Stretch_Servo,150);//���ֱ�ӷ��� 30
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

/**@brief   �ѿ�Ƭ�Ӳֿ������ó���
-- @param   ��
-- @author  ׯ�ı�
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
        case 4://����Ƭ
            Set_Servo_Angle(Stretch_Servo,150);//15
            Out_State = 5;           
        break;
        case 5://�ѿ�Ƭ�ó���
            if(Bufcnt(true,500))
            {
                Set_Servo_Angle(Raise_Servo,135);
                Out_State = 6;
            }            
        break;
        case 6://�ѿ�Ƭ�ó���
            if(Bufcnt(true,500))
            {
                Set_Servo_Angle(Stretch_Servo,60);//120
                Out_State = 7;
            }
        break;
        case 7://�ѿ�Ƭ������
            if(Bufcnt(true,500))
            {
                Set_Servo_Angle(Raise_Servo,0);
                Out_State = 8;
            }
        break;
        case 8://�ѿ�Ƭ����
            if(Bufcnt(true,375))
            {
                Set_Servo_Angle(Stretch_Servo,30);//150
                Out_State = 9;
            }
        break;
        case 9://���������
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