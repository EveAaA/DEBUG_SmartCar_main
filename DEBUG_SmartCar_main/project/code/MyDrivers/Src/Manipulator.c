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
    .Init_Angle = 40,//�Ƕȴ�����
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
    .Init_Angle = 85,//�Ƕȴ�˳ʱ��ת,85��ɫ��ǰ,175��ɫ��ǰ,265��ɫ��ǰ,355��ɫ��ǰ��
                     //185��ɫ�� 275��ɫ�� 5��ɫ�� 95��ɫ��
    .Servo_Time = 0,
};
Servo_Handle Door_Servo = //�ŵ��
{
    .Pin = PWM2_MODULE3_CHB_D3,
    .Init_Angle = 50,//�Ƕȴ��� ���ŽǶ�130  ���ŽǶ�50
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
void Set_Servo_Angle(Servo_Handle Servo,uint16 Angle)
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
void Manipulator_PutDown()
{
    static uint8 PuDowm_State = 0;
    switch (PuDowm_State)
    {
        case 0://�ȷ���̧�ֶ��
            Electromagnet_On;//�򿪵����
            Set_Servo_Angle(Raise_Servo,0);
#ifdef Servo_Slow
            PuDowm_State = 1;
            Stretch_Servo.Servo_Time = 1;
#else
        PuDowm_State = 2;
#endif
        break;
        case 1://����׶�
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

/**@brief   ��е��̧��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/5
**/
void Manipulator_PutUp()
{
    static uint8 PutUp_State = 0;
    switch (PutUp_State)
    {
        case 0:
            Set_Servo_Angle(Stretch_Servo,120);//��̧���
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
            if(Raise_Servo.Servo_Time == 0)
            {
                Raise_Servo.Servo_Time = 1;
            }
            if(Raise_Servo.Servo_Time >= 50)
            {
                Raise_Servo.Servo_Time = 0;
                PutUp_State = 0;
                Servo_Flag.Put_Up = true;
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
    static uint8 Pickup_State = 0;
    // printf("Pickup_State = %d\r\n",Pickup_State);
    switch(Pickup_State)
    {
        case 0:
            if(!Servo_Flag.Put_Down)//��е�۷���
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
            if(Servo_Flag.Put_Up == false)//�ѿ�Ƭ������
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

/**@brief   ����ת�̶��ת���ĸ��ط�
-- @param   Rotaryservo_Handle RotaryServo ѡ����ɫ
-- @param   bool Door �Ƿ���ж��
-- @author  ׯ�ı�
-- @date    2024/5/6
**/
void Rotary_Switch(Rotaryservo_Handle RotaryServo,bool Door)
{
    switch (RotaryServo)
    {
        case White:
            if(Door == false)//��ǰ��
            {
                Set_Servo_Angle(Rotary_Servo,85);
            }
            else
            {
                Set_Servo_Angle(Rotary_Servo,185);
            }
        break;
        case Black:
            if(Door == false)//��ǰ��
            {
                Set_Servo_Angle(Rotary_Servo,175);
            }
            else
            {
                Set_Servo_Angle(Rotary_Servo,275);
            }
        break;
        case Red:
            if(Door == false)//��ǰ��
            {
                Set_Servo_Angle(Rotary_Servo,265);
            }
            else
            {
                Set_Servo_Angle(Rotary_Servo,5);
            }
        break;
        case Yellow:
            if(Door == false)//��ǰ��
            {
                Set_Servo_Angle(Rotary_Servo,355);
            }
            else
            {
                Set_Servo_Angle(Rotary_Servo,95);
            }
        break;
    }
}

/**@brief   �ѿ�Ƭ����ָ���ֿ�
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/4/30
**/
void Put_Depot()
{
    static uint8 Depot_State = 0;
    static uint8 Card_Class = 0;
    switch(Depot_State)
    {
        case 0:
            Card_Class = Receivedata.Servo_Rotary;
            Rotary_Switch(Card_Class,false);
            if(Rotary_Servo.Servo_Time == 0)
            {
                Rotary_Servo.Servo_Time = 1;
            }
            if(Rotary_Servo.Servo_Time >= 250)
            {
                Rotary_Servo.Servo_Time = 0;
                Depot_State = 1;
            }
        break;
        case 1:
            Set_Servo_Angle(Stretch_Servo,30);//���ֱ�ӷ���
            Stretch_Servo.Servo_Time = 1;
            Depot_State = 2;
        break;
        case 2:
            if(Stretch_Servo.Servo_Time>=50)
            {    
                Stretch_Servo.Servo_Time = 0;
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
            Electromagnet_On;
            Set_Servo_Angle(Raise_Servo,140);
            Out_State = 1;
        break;
        case 1://����Ƭ
            Set_Servo_Angle(Stretch_Servo,15);
            Stretch_Servo.Servo_Time = 1;
            Out_State = 2;
        break;
        case 2://�ѿ�Ƭ�ó���
            if(Stretch_Servo.Servo_Time >= 100)
            {
                Set_Servo_Angle(Stretch_Servo,120);
                Stretch_Servo.Servo_Time = 1;
                Out_State = 3;
            }
        break;
        case 3://�ѿ�Ƭ������
            if(Stretch_Servo.Servo_Time >= 100)
            {
                Stretch_Servo.Servo_Time = 0;
                Set_Servo_Angle(Raise_Servo,0);
                Raise_Servo.Servo_Time = 1;
                Out_State = 4;
            }
        break;
        case 4://�ѿ�Ƭ����
            if(Raise_Servo.Servo_Time >= 75)
            {
                Raise_Servo.Servo_Time = 0;
                Stretch_Servo.Servo_Time = 1;
                Set_Servo_Angle(Stretch_Servo,150);
                Out_State = 5;
            }
        break;
        case 5://���������
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