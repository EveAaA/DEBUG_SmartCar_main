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
    .Pin = PWM2_MODULE0_CHA_C6,
    .Init_Angle = 150,//�Ƕ�С����
    .Servo_Time = 0,
};
Servo_Handle Raise_Servo = //̧�ֶ��
{
    .Pin = PWM1_MODULE3_CHB_B11,
    .Init_Angle = 50,//�Ƕ�С����
    .Servo_Time = 0,
};
Servo_Handle Rotary_Servo = //��ת���
{
    .Pin = PWM1_MODULE3_CHA_D0,
    .Init_Angle = 65,//�Ƕ�С����
};
Servo_Flag_Handle Servo_Flag = {false,false};

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��е�۳�ʼ��
-- @param   ��
-- @auther  ׯ�ı�
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

/**@brief   ���ö���Ƕ�,180�ȶ��
-- @param   ServoHandle Servo ѡ����
-- @param   float Angle ���õĽǶ�
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void Set_Servo_Angle(Servo_Handle Servo,float Angle)
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

    pwm_set_duty(Servo.Pin,Set_180Servo_Angle(Servo.Set_Angle));
}

/**@brief   ��е�۷����ÿ�Ƭ����
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/11/5
**/
void Manipulator_PutDown()
{
    static uint8 PuDowm_State = 0;
    switch (PuDowm_State)
    {
        case 0://�ȷ���̧�ֶ��
            Set_Servo_Angle(Raise_Servo,110);
            Electromagnet_On;//�򿪵����
#ifdef Servo_Slow
            PuDowm_State = 1;
            Stretch_Servo.Servo_Time = 1;
#else
        PuDowm_State = 2;
#endif
        break;
        case 1://����׶�
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

/**@brief   ��е��̧��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void Manipulator_PutUp()
{
    static uint8 PutUp_State = 0;
    switch (PutUp_State)
    {
        case 0:
            Set_Servo_Angle(Stretch_Servo,60);//��̧���
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

/**@brief   ��е�ۼ���Ƭ
-- @param   ��
-- @auther  ׯ�ı�
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