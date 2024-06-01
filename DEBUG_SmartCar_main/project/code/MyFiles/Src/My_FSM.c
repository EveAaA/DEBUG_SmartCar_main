/**
  ******************************************************************************
  * @file    My_FSM.c
  * @author  ׯ�ı�
  * @brief   ��״̬��
  *
    @verbatim
	��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "My_FSM.h"

/* Define\Declare ------------------------------------------------------------*/
FSM_Handle MyFSM = {
    .CurState = Line_Patrol,
    .Line_Board_State = Find,
    .Static_Angle = 0,
    .Board_Dir = -1,
};

uint16 wait_time = 0;
#define Static_Time 100 //�ȴ���ֹ��ʱ�䣬��Լ0.5��
// #define debug_switch  //�Ƿ����

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   ɢ���������ԵĿ�Ƭ״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/5/10
**/
void Line_BoardFsm()
{
    switch (MyFSM.Line_Board_State)
    {
        case Find://�ҵ���Ƭ
            #ifdef debug_switch
                printf("Find\r\n");    
            #endif 
            if(Turn.Finish == false)
            {
                if(MyFSM.Board_Dir == LEFT)
                {
                    Turn_Angle(-90);//ת����ʮ��
                }
                else if(MyFSM.Board_Dir == RIGHT)
                {
                    Turn_Angle(90);//ת����ʮ��
                }
            }
            else
            {
                MyFSM.Line_Board_State = Wait_Data;//�ȴ��ƶ����ݵķ���
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Turn.Finish = false;
            }
        break;
        case Wait_Data://�ȴ��������ݻش�
            #ifdef debug_switch
                printf("Wait_Data\r\n");    
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            if((FINETUNING_DATA.dx!=0) || (FINETUNING_DATA.dy!=0))
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Line_Board_State = Move;//��ʼ�ƶ�����Ƭǰ��
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                if(MyFSM.Board_Dir == LEFT)
                {
                    Car_run_X(3.0f);//�����ƶ�һ���ֹ��������Ƭ
                }
                else if(MyFSM.Board_Dir == RIGHT)
                {
                    Car_run_X(-3.0f);//�����ƶ�һ���ֹ��������Ƭ
                }
            }
        break;
        case Move://�ƶ�����Ƭǰ��
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Line_Board_State = Classify;//ʶ�����
            }
        break;
        case Confirm://ȷ���Ƿ��ƶ���λ
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)
            {
                MyFSM.Line_Board_State = Move;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Line_Board_State = Classify;//ʶ�����
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Classify:
            #ifdef debug_switch
                printf("Classify\r\n");    
            #endif 
            if(CLASSIFY_DATA.type != None)//ʶ���˷���
            {
                MyFSM.Line_Board_State = Pick;//����Ƭ
                MyFSM.Big_Board = CLASSIFY_DATA.type;//��¼����
                printf("class = %d\r\n",CLASSIFY_DATA.type);
                CLASSIFY_DATA.type = None;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Set_Beepfreq(MyFSM.Big_Board+1);
                Rotary_Switch(MyFSM.Big_Board,false);//ת��ת��
                MyFSM.Big_Board = None;
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//�������ݣ����շ�������
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Pick://����Ƭ
            #ifdef debug_switch
                printf("Pick\r\n");    
            #endif
            if(Servo_Flag.Pick_End == false)
            {
                if(wait_time != 1)
                {
                    if(Bufcnt(true,200))//�ȴ�һ���ó�ͣ����
                    {
                        wait_time = 1;
                    }
                }

                if(wait_time == 1)
                {
                    Pick_Card();
                }
            }
            else
            {
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
                wait_time = 0;
                MyFSM.Line_Board_State = Return_Line;//��������
            } 
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Return_Line://��������
            #ifdef debug_switch
                printf("Return\r\n");    
            #endif
            if(Turn.Finish == false)
            {
                if(MyFSM.Board_Dir == LEFT)
                {
                    Turn_Angle(90);
                }
                else if(MyFSM.Board_Dir == RIGHT)
                {
                    Turn_Angle(-90);
                }
            }
            else
            {
                Turn.Finish = false;
                MyFSM.Line_Board_State = Find;
                MyFSM.CurState = Line_Patrol;
            }
        break;
    }
}

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/4/27
**/
void FSM_main()
{
    switch (MyFSM.CurState)
    {
        case Line_Patrol://Ѳ��״̬
            if(Start == 1)
            {
                #ifdef debug_switch
                    printf("Line\r\n");    
                #endif 
                Car_run();
                if(FINDBORDER_DATA.FINDBORDER_FLAG == true)
                {
                    Forward_Speed = 3;
                }
                else
                {
                    Forward_Speed = 5;
                }
                if((FINDBORDER_DATA.dir == LEFT) || (FINDBORDER_DATA.dir == RIGHT))
                {
                    MyFSM.Board_Dir = FINDBORDER_DATA.dir;
                    MyFSM.CurState = Line_Board;//��߿�Ƭ
                }
            }
            else
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Line_Board://����ɢ�俨Ƭ
            Line_BoardFsm();
        break;
    }
}

