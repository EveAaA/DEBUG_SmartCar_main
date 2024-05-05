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
    .LeftBoard_State = Find,
    .RightBoard_State = Find,
};

uint16 wait_time = 0;
float Staic_Angle = 0;
#define Static_Time 100 //�ȴ���ֹ��ʱ�䣬��Լһ��
// #define debug_switch  //�Ƿ����

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   ��߿�Ƭ״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/4/27
**/
void Left_BoardFsm()
{
    switch (MyFSM.LeftBoard_State)
    {
        case Find://�ҵ���Ƭ
            #ifdef debug_switch
                printf("Find\r\n");    
            #endif 
            if(Turn.Finish == false)
            {
                Turn_Angle(-90);//ת����ʮ��
            }
            else
            {
                MyFSM.LeftBoard_State = Wait_Data;//�ȴ��ƶ����ݵķ���
                Staic_Angle = Gyro_YawAngle_Get();
                Turn.Finish = false;
            }
        break;
        case Wait_Data://�ȴ��������ݻش�
            #ifdef debug_switch
                printf("Wait_Data\r\n");    
            #endif 
            if(FINETUNING_DATA.dx!=0 || FINETUNING_DATA.dy!=0)
            {
                if(wait_time==0)
                {
                    wait_time = 1;
                }
                if(wait_time >= Static_Time)
                {
                    wait_time = 0; 
                    MyFSM.LeftBoard_State = Move;//��ʼ�ƶ�����Ƭǰ��
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            else
            {
                Car.Speed_X = 3;//����ǰ�ƶ�һ���ֹ��������Ƭ
                Car.Speed_Y = 0;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            // UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
        break;
        case Move://�ƶ�����Ƭǰ��
            #ifdef debug_switch
                printf("Move\r\n");    
            #endif 
            // printf("%f,%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f,0);
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(0,0);//�ƶ�
            }
            else
            {
                Set_Beeptime(500);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                Staic_Angle = Gyro_YawAngle_Get();
                MyFSM.LeftBoard_State = Pick;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm://ȷ���Ƿ��ƶ���λ
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            // UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            // printf("%f,%f\r\n",FINETUNING_DATA.dx,FINETUNING_DATA.dy);
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)//û���ƶ���λ
            {
                if(FINETUNING_DATA.dx!=0 || FINETUNING_DATA.dy!=0)//ȷ��������
                {
                    
                    MyFSM.LeftBoard_State = Move;
                }
            }
            else
            {
                MyFSM.LeftBoard_State = Pick;//����Ƭ
                Staic_Angle = Gyro_YawAngle_Get();
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Pick://����Ƭ
            #ifdef debug_switch
                printf("Pick\r\n");    
            #endif
            if(wait_time==0)
            {
                wait_time = 1;
            }
            if(Servo_Flag.Put_Depot == false)
            {
                if(wait_time >= Static_Time)
                {
                    Pick_Card();
                }
            }
            else
            {
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Put_Depot = false;
                wait_time = 0;
                MyFSM.LeftBoard_State = Return_Line;
            } 
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Return_Line:
            #ifdef debug_switch
                printf("Return\r\n");    
            #endif
            if(Turn.Finish == false)
            {
                Turn_Angle(90);
            }
            else
            {
                Turn.Finish = false;
                MyFSM.LeftBoard_State = Find;
                MyFSM.CurState = Line_Patrol;
            }
        break;
    }
}

/**@brief   �ұ߿�Ƭ״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/4/27
**/
void Right_BoardFsm()
{
    switch (MyFSM.RightBoard_State)
    {
        case Find://�ҵ���Ƭ
            #ifdef debug_switch
                printf("Find\r\n");    
            #endif 
            if(Turn.Finish == false)
            {
                Turn_Angle(90);//ת����ʮ��
            }
            else
            {
                MyFSM.RightBoard_State = Wait_Data;//�ȴ��ƶ����ݵķ���
                Staic_Angle = Gyro_YawAngle_Get();
                Turn.Finish = false;
            }
        break;
        case Wait_Data://�ȴ��������ݻش�
            #ifdef debug_switch
                printf("Wait_Data\r\n");    
            #endif 
            if(FINETUNING_DATA.dx!=0 || FINETUNING_DATA.dy!=0)
            {
                if(wait_time==0)
                {
                    wait_time = 1;
                }
                if(wait_time >= Static_Time)
                {
                    MyFSM.RightBoard_State = Move;//��ʼ�ƶ�����Ƭǰ��
                    wait_time = 0;
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            else
            {
                Car.Speed_X = -3;//����ǰ�ƶ�һ���ֹ��������Ƭ
                Car.Speed_Y = 0;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            // UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
        break;
        case Move://�ƶ�����Ƭǰ��
            #ifdef debug_switch
                printf("Move\r\n");    
            #endif 
            // printf("%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(0,0);//�ƶ�
            }
            else
            {
                Set_Beeptime(500);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                Staic_Angle = Gyro_YawAngle_Get();
                MyFSM.RightBoard_State = Pick;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm://ȷ���Ƿ��ƶ���λ
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            // UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            // printf("%f,%f\r\n",FINETUNING_DATA.dx,FINETUNING_DATA.dy);
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)//û���ƶ���λ
            {
                if(FINETUNING_DATA.dx!=0 || FINETUNING_DATA.dy!=0)
                {
                    MyFSM.RightBoard_State = Move;
                }
            }
            else
            {
                MyFSM.RightBoard_State = Pick;//����Ƭ
                Staic_Angle = Gyro_YawAngle_Get();
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Pick://����Ƭ
            #ifdef debug_switch
                printf("Pick\r\n");    
            #endif
            if(wait_time==0)
            {
                wait_time = 1;
            }
            if(Servo_Flag.Put_Depot == false)
            {
                if(wait_time >= 50)//�ȴ�һ���ó�ͣ����
                {
                    Pick_Card();
                }
            }
            else
            {
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Put_Depot = false;
                wait_time = 0;
                MyFSM.RightBoard_State = Return_Line;//��������
            } 
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Return_Line://��������
            #ifdef debug_switch
                printf("Return\r\n");    
            #endif
            if(Turn.Finish == false)
            {
                Turn_Angle(-90);
            }
            else
            {
                Turn.Finish = false;
                MyFSM.RightBoard_State = Find;
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
                if(FINDBORDER_DATA.dir == LEFT)
                {
                    MyFSM.CurState = Line_Left_Board;//��߿�Ƭ
                    //Staic_Angle = Gyro_YawAngle_Get();
                }
                else if(FINDBORDER_DATA.dir == RIGHT)
                {
                    MyFSM.CurState = Line_Right_Board;//�ұ߿�Ƭ
                }
            }
            else
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Line_Left_Board:
            Left_BoardFsm();
        break;
        case Line_Right_Board:
            Right_BoardFsm();
        break;
    }
    Set_Car_Speed(Car.Speed_X,Car.Speed_Y,Car.Speed_Z);//�����ٶȵ��߳�
}

