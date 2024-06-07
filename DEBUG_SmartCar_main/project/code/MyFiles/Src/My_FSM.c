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
    .CurState = Depart,
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

/**@brief   ����״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/6/5
**/
static void Depart_Fsm()
{
    if(Start == 1)
    {
        if(Navigation.Finish_Flag == false)
        {
            Navigation_Process_Y(30);
        }
        else
        {
            Navigation.Finish_Flag = false;
            MyFSM.CurState = Line_Patrol;
            Car.Image_Flag = true;
        }
    }
    else
    {
        Car.Speed_X = 0;
        Car.Speed_Y = 0;
        Car.Speed_Z = 0;
    }
}

/**@brief    Ѳ��״̬��
-- @param    ��
-- @verbatim �ڴ�״̬����ת��������Ƭ��״̬��
-- @author   ׯ�ı�
-- @date     2024/6/5
**/
static void Line_PatrolFsm()
{
    #ifdef debug_switch
        printf("Line_Patrol\r\n");    
    #endif 
    // if(FINDBORDER_DATA.FINDBORDER_FLAG == true)
    // {
    //     Forward_Speed = 3;
    // }
    // else
    // {
        Forward_Speed = 5;
    // }
    Car_run(Forward_Speed);
    // if((FINDBORDER_DATA.dir == LEFT) || (FINDBORDER_DATA.dir == RIGHT))
    // {
    //     MyFSM.Board_Dir = FINDBORDER_DATA.dir;
    //     MyFSM.CurState = Line_Board;//ɢ�俨Ƭ
    // }
}

/**@brief   ɢ���������ԵĿ�Ƭ״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/5/10
**/
static void Line_BoardFsm()
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
            }
            else
            {
                if(MyFSM.Board_Dir == LEFT)
                {
                    Car.Speed_X = 2.0f;//�����ƶ�һ���ֹ��������Ƭ
                }
                else
                {
                    Car.Speed_X = -2.0f;//�����ƶ�һ���ֹ��������Ƭ
                }
            }
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move://�ƶ�����Ƭǰ��
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Line_Board_State = Confirm;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm://ȷ���Ƿ��ƶ���λ
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 2)
            {
                MyFSM.Line_Board_State = Move;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Line_Board_State = Move_Y;//Y���ƶ�
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Move_Y:
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f\r\n",FINETUNING_DATA.dy/10.f);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Y(FINETUNING_DATA.dy/10.f);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Line_Board_State = Confirm_Y;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm_Y:
            #ifdef debug_switch
                printf("Confirm_Y\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Line_Board_State = Move_Y;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//�������ݣ����շ�������
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
                Dodge_Board();
                MyFSM.Line_Board_State = Pick;//����Ƭ
                MyFSM.Big_Board = CLASSIFY_DATA.type;//��¼����
                CLASSIFY_DATA.type = None;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Set_Beepfreq(MyFSM.Big_Board+1);
                Car.Depot_Pos = MyFSM.Big_Board;
                MyFSM.Big_Board = None;
            }
            else
            {
                if(Bufcnt(true,3000))//3s����һ��
                {
                    UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//�������ݣ����շ�������
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Pick://����Ƭ
            #ifdef debug_switch
                printf("Pick\r\n");    
            #endif
            if(Servo_Flag.Pick_End == false)
            {
                Pick_Card();
            }
            else
            {
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
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
        case Depart://����״̬
            Depart_Fsm();
        break;
        case Line_Patrol://Ѳ��״̬
            Line_PatrolFsm();
        break;
        case Line_Board://����ɢ�俨Ƭ
            Line_BoardFsm();
        break;
    }
}

