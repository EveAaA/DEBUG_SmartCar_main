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
    .Cross_Board_State = Find_Cross,
    .Unload_State = Find_Zebra,
    .Static_Angle = 0,
    .Board_Dir = -1,
    .Stop_Flag = false,
    .Big_Count[0] = 0,
    .Big_Count[1] = 0,
    .Big_Count[2] = 0,
    .Small_Count = 0,
    .Cross_Flag_ = false,
    .Unload_Count = 0,
    .Big_Pos_Count = 0,
    .Depot_Pos = White,
    .Big_Pos[0] = RIGHT,
    .Big_Pos[1] = RIGHT,
    .Big_Pos[2] = RIGHT,
};

uint16 wait_time = 0;
#define Static_Time 100 //�ȴ���ֹ��ʱ�䣬��Լ0.5��
#define debug_switch  //�Ƿ����

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
            Navigation_Process_Y(0,30);
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
    if(FINDBORDER_DATA.FINDBORDER_FLAG == true)
    {
        Forward_Speed = 3;
    }
    else
    {
        Forward_Speed = 5;
    }
    Car_run(Forward_Speed);
    Dodge_Carmar();
    if((FINDBORDER_DATA.dir == LEFT) || (FINDBORDER_DATA.dir == RIGHT))
    {
        MyFSM.Board_Dir = FINDBORDER_DATA.dir;
        Car.Image_Flag = false;
        MyFSM.CurState = Line_Board;//ɢ�俨Ƭ
    }
    else if(Image_Flag.Cross_Fill == 2)
    {
        MyFSM.CurState = Cross_Board;//ʮ�ֻػ�״̬��
        MyFSM.Cross_Dir = Image_Flag.Cross_Type;
        MyFSM.Cross_Flag_ = true;
        Set_Beepfreq(1);
    }
    else if(Bufcnt(Image_Flag.Zerba,500))
    {
        Car.Image_Flag = false;
        Image_Flag.Zerba = false;
        MyFSM.CurState = Unload;//ж��
    }
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
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Line_Board_State = Move;//��ʼ�ƶ�����Ƭǰ��
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
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
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
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
                Navigation_Process_Y(0,FINETUNING_DATA.dy/10.f);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
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
                MyFSM.Big_Count[MyFSM.Big_Board]+=1;
                CLASSIFY_DATA.type = None;
                Set_Beepfreq(MyFSM.Big_Board+1);
                MyFSM.Depot_Pos = MyFSM.Big_Board;
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
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
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
                Car.Image_Flag = true;
                Turn.Finish = false;
                MyFSM.Line_Board_State = Find;
                MyFSM.CurState = Line_Patrol;
            }
        break;
    }
}

/**@brief   ʮ�ֻػ���Ƭ״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/5/10
**/
static void Cross_BoardFsm()
{
    static float Curanglg = 0;
    static float Lastanglg = 0;
    static float Angle_Offest = 0;
    switch (MyFSM.Cross_Board_State)
    {
        case Find_Cross://�ҵ�ʮ��
            #ifdef debug_switch
                printf("Find_Cross\r\n");    
            #endif 
            Car_run(5);
            if(Bufcnt(true,1500))
            {
                MyFSM.Cross_Board_State = Find;
                Car.Image_Flag = false;
            }
        break;
        case Find://�ҵ�ʮ�ֺ�׼��ת��
            if(MyFSM.Cross_Dir == RIGHT)
            {
                if(Turn.Finish == false)
                {
                    Turn_Angle(90);
                }
                else
                {
                    Turn.Finish = false;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    MyFSM.Cross_Board_State = Wait_Data;
                }
            }
            else if(MyFSM.Cross_Dir == LEFT)
            {
                if(Turn.Finish == false)
                {
                    Turn_Angle(-90);
                }
                else
                {
                    Turn.Finish = false;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    MyFSM.Cross_Board_State = Wait_Data;
                }                    
            }
        break;
        case Wait_Data://�ȴ��������ݻش�
            #ifdef debug_switch
                printf("Cross_Wait_Data\r\n");    
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,800))
                {
                    MyFSM.Cross_Board_State = Move;//��ʼ�ƶ�����Ƭǰ��
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                }
                Car.Speed_X = 0;
            }
            else
            {
                if(MyFSM.Cross_Dir == LEFT)
                {
                    Car.Speed_X = 2;//�����ƶ�һ���ֹ��������Ƭ
                }
                else
                {
                    Car.Speed_X = -2;//�����ƶ�һ���ֹ��������Ƭ
                }
            }
            Car.Speed_Y = 2;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move://�ƶ�����Ƭǰ��
            #ifdef debug_switch
                printf("Cross_Move:%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm://ȷ���Ƿ��ƶ���λ
            #ifdef debug_switch
                printf("Cross_Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 2)
            {
                MyFSM.Cross_Board_State = Move;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Cross_Board_State = Move_Y;//Y���ƶ�
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Move_Y:
            #ifdef debug_switch
                printf("Cross_Move_Y:%f\r\n",FINETUNING_DATA.dy/10.f);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Y(0,FINETUNING_DATA.dy/10.f);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm_Y;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm_Y:
            #ifdef debug_switch
                printf("Cross_Confirm_Y\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//��������
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Cross_Board_State = Move_Y;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//�������ݣ����շ�������
                MyFSM.Cross_Board_State = Classify;//ʶ�����
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Classify:
            #ifdef debug_switch
                printf("Cross_Classify\r\n");    
            #endif 
            if(CLASSIFY_DATA.place != nil)//ʶ���˷���
            {
                Dodge_Board();
                MyFSM.Cross_Board_State = Pick;//����Ƭ
                MyFSM.Small_Board[MyFSM.Small_Count] = CLASSIFY_DATA.place;//��¼����
                CLASSIFY_DATA.place = nil;
                Set_Beepfreq(1);
                if(MyFSM.Small_Count == 4)
                {
                    MyFSM.Depot_Pos = MyFSM.Small_Count - 1;//���òֿ�λ��
                }
                else
                {
                    MyFSM.Depot_Pos = MyFSM.Small_Count;
                }
                MyFSM.Small_Count+=1;
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
                printf("Cross_Pick\r\n");    
            #endif
            if(Servo_Flag.Pick_End == false)
            {
                Pick_Card();
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                if(MyFSM.Small_Count<=3)
                {
                    MyFSM.Cross_Board_State = Classify;//����ʶ��
                    Servo_Flag.Put_Up = false;
                    Servo_Flag.Put_Down = false;
                    Servo_Flag.Pick_End = false;
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                }
                else
                {
                    if(Turn.Finish==false)
                    {
                        Turn_Angle(160);
                    }
                    else
                    {
                        Turn.Finish = false;
                        Servo_Flag.Put_Up = false;
                        Servo_Flag.Put_Down = false;
                        Servo_Flag.Pick_End = false;
                        MyFSM.Cross_Board_State = Find_Place;//Ѱ�ҷ���λ��
                    }
                }
            } 
        break;
        case Find_Place://Ѱ�ҷ���λ��
            #ifdef debug_switch
                printf("Find_Place:%d,%f\r\n",MyFSM.Unload_Count,Angle_Offest);    
            #endif 
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//��ȡ�����������
            Dodge_Carmar();
            Curanglg = Gyro_YawAngle_Get();
            if (Lastanglg == 0)
            {
                Lastanglg = Curanglg;
            }
            Angle_Offest += Curanglg - Lastanglg;
            if(MyFSM.Cross_Dir == RIGHT)
            {
                Car_run_X(2);
            }
            else
            {
                Car_run_X(-2);
            }
            Lastanglg = Curanglg;
            if(MyFSM.Unload_Count==0&&FINDBORDER_DATA.FINDBIGPLACE_FLAG == true)
            {
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Cross_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
            }
            else if(FINDBORDER_DATA.FINDBIGPLACE_FLAG == true && (fabs(Angle_Offest) >= 35))
            {
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Cross_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
            }
        break;
        case Wait_PlaceData://�ȴ����ڴ��ط����������Ϣ
            #ifdef debug_switch
                printf("Cross_Wait_PlaceData\r\n");    
            #endif 
            Dodge_Carmar();
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//���ͻ�ȡ����������Ϣ
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Cross_Board_State = Move_Place;//��ʼ�ƶ�����������ǰ��
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move_Place://�ƶ�����Ƭǰ��
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("Cross_Place:%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm_Place;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm_Place://ȷ���Ƿ��ƶ���λ
            #ifdef debug_switch
                printf("Cross_Confirm_Place\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//��������
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == 0)
            {
                MyFSM.Cross_Board_State = Move_Place;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Cross_Board_State = Move_PlaceY;//Y���ƶ�
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Move_PlaceY:
            #ifdef debug_switch
                printf("Move_PlaceY:%f\r\n",FINETUNING_DATA.dy/10.f);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Y(0,FINETUNING_DATA.dy/10.f);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm_PlaceY;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm_PlaceY:
            #ifdef debug_switch
                printf("Confirm_PlaceY\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//���ְ�΢��
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Cross_Board_State = Move_PlaceY;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE);//�������ݣ����մ����������
                MyFSM.Cross_Board_State = Classify_Place;//ʶ�����
                Servo_Flag.Depot_End = false;//������
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Classify_Place:
            #ifdef debug_switch
                printf("Classify_Place\r\n");    
            #endif 
            if(SMALL_PLACE_DATA.place != nil)//ʶ���˷���
            {
                Dodge_Board();
                Servo_Flag.Depot_End = true;
                for(uint8 i = 0;i <= MyFSM.Small_Count - 1;i++)
                {
                    if(MyFSM.Small_Board[i] == SMALL_PLACE_DATA.place)
                    {
                        MyFSM.Depot_Pos = i;
                        MyFSM.Unload_Count+=1;
                        Set_Beepfreq(1);
                        break;
                    }
                }
                SMALL_PLACE_DATA.place = nil;
                MyFSM.Cross_Board_State = Unload_Board;
            }
            else
            {
                if(Bufcnt(true,3000))//3s����һ��
                {
                    UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE);//�������ݣ�����С���������
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Unload_Board:
            #ifdef debug_switch
                printf("Cross_Unload_Board\r\n");    
            #endif
            if(Servo_Flag.Depot_End)
            {
                if(!Servo_Flag.Put_Out)
                {
                    Take_Card_Out();
                }
                else
                {
                    Servo_Flag.Put_Out = false;
                    if(MyFSM.Unload_Count==MyFSM.Small_Count)
                    {
                        MyFSM.Cross_Board_State  = Return_Line;
                    }
                    else
                    {
                        MyFSM.Cross_Board_State = Ready_Find_Next;
                    }
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Ready_Find_Next:
            if(MyFSM.Cross_Dir==RIGHT)
            {
                if(Navigation.Finish_Flag == false)
                {
                    Navigation_Process_Y(0,-50);
                }
                else
                {
                    MyFSM.Cross_Board_State = Find_Place;
                }
            }
            else
            {
                if(Navigation.Finish_Flag == false)
                {
                    Navigation_Process_Y(0,-50);
                }
                else
                {
                    MyFSM.Cross_Board_State = Find_Place;
                }                
            }
        break;
        case Return_Line:
            if(MyFSM.Cross_Dir==RIGHT)
            {
                if(Turn.Finish == false)
                {
                    Turn_Angle(90);
                }
                else
                {
                    Turn.Finish = false;
                    MyFSM.Cross_Board_State = Find_Cross;
                    MyFSM.CurState = Line_Patrol;
                    Car.Image_Flag = true;
                    UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//������ȡ��·�Կ�Ƭ
                }
            }
            else
            {
                if(Turn.Finish == false)
                {
                    Turn_Angle(-90);
                }
                else
                {
                    Turn.Finish = false;
                    MyFSM.Cross_Board_State = Find_Cross;
                    MyFSM.CurState = Line_Patrol;
                    Car.Image_Flag = true;
                    UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//������ȡ��·�Կ�Ƭ
                }                
            }
        break;   
    }
}

/**@brief   �յ�ǰж��״̬��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/6/9
**/
static void Unload_Fsm()
{
    switch(MyFSM.Unload_State)
    {
        case Find_Zebra:
            #ifdef debug_switch
                printf("Find_Zebra\r\n");    
            #endif 
            if(Turn.Finish == false)
            {
                if(MyFSM.Big_Pos[0] == RIGHT)
                {
                    Turn_Angle(90);
                }
                else if(MyFSM.Big_Pos[0] == LEFT)
                {
                    Turn_Angle(-90);
                }
            }
            else 
            {
                Turn.Finish = false;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Unload_State = Wait_Big_Data;
            }
        break;
        case Wait_Big_Data:
            #ifdef debug_switch
                printf("Wait_Big_Data\r\n");    
            #endif 
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//��ȡ�����������
            if(FINDBORDER_DATA.FINDBIGPLACE_FLAG == true)
            {
                MyFSM.Unload_State = Wait_Data;
            }
            Car.Speed_X = 2;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Wait_Data:
            #ifdef debug_switch
                printf("Wait_Data\r\n");    
            #endif 
            Dodge_Carmar();
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//���Ͷ����ְ�΢����Ϣ
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Unload_State = Move;//��ʼ�ƶ�����Ƭǰ��
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                }
            }
            Car.Speed_X = 0;
            if(MyFSM.Big_Pos_Count == 0)
            {
                Car.Speed_Y = 2;
            }
            else
            {
                Car.Speed_Y = 0;
            }
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move://�ƶ�����Ƭǰ��
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Unload_State = Confirm;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm://ȷ���Ƿ��ƶ���λ
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//��������
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == 0)
            {
                MyFSM.Unload_State = Move;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Unload_State = Move_Y;//Y���ƶ�
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
                Navigation_Process(0,FINETUNING_DATA.dy/10.f);//�ƶ�
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Unload_State = Confirm_Y;//ȷ���Ƿ��ƶ���λ
            }
        break;
        case Confirm_Y:
            #ifdef debug_switch
                printf("Confirm_Y\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//���ְ�΢��
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Unload_State = Move_Y;//�ƶ�
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_BIGPLACE);//�������ݣ����մ����������
                MyFSM.Unload_State = Classify;//ʶ�����
                Servo_Flag.Depot_End = false;//������
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Classify:
            #ifdef debug_switch
                printf("Classify\r\n");    
            #endif 
            if(BIG_PLACE_DATA.Big_Place != None)//ʶ���˷���
            {
                Dodge_Board();
                MyFSM.Big_Board = BIG_PLACE_DATA.Big_Place;//��¼����
                BIG_PLACE_DATA.Big_Place = None;
                Set_Beepfreq(MyFSM.Big_Board+1);
                MyFSM.Depot_Pos = MyFSM.Big_Board;//ת��ת��
                MyFSM.Big_Pos_Count +=1;//��¼ȥ�˼�������ж����
                MyFSM.Unload_Count = MyFSM.Big_Count[MyFSM.Big_Board];
                MyFSM.Big_Board = None;
                MyFSM.Unload_State = Unload_Board;
            }
            else
            {
                if(Bufcnt(true,300))//3s����һ��
                {
                    UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_BIGPLACE);//�������ݣ����մ����������
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Unload_Board:
            #ifdef debug_switch
                printf("Unload_Board\r\n");    
            #endif
            if(MyFSM.Unload_Count == 0)
            {
                MyFSM.Unload_State = Unload_Next;
                Servo_Flag.Depot_End = false;
            }
            if(Servo_Flag.Depot_End)
            {
                if(!Servo_Flag.Put_Out)
                {
                    Take_Card_Out();
                }
                else
                {
                    MyFSM.Unload_Count -=1;   
                    Servo_Flag.Put_Out = false;
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Unload_Next:
            #ifdef debug_switch
                printf("Unload_Next\r\n");    
            #endif
            if(MyFSM.Big_Pos_Count < 3)
            {
                if((MyFSM.Big_Pos[0]!=MyFSM.Big_Pos[1]) || (MyFSM.Big_Pos[1]!=MyFSM.Big_Pos[2]))
                {
                    if(Turn.Finish == false)
                    {
                        Turn_Angle(180);
                    }
                    else
                    {
                        Turn.Finish = false;
                        MyFSM.Unload_State = Wait_Big_Data;
                        MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    }
                }
                else
                {
                    if(Navigation.Finish_Flag == false)
                    {
                        Navigation_Process(60,0);
                    }
                    else
                    {
                        MyFSM.Unload_State = Wait_Data;
                        Navigation.Finish_Flag = false;
                    }
                }
            } 
            else
            {
                if(Turn.Finish == false)
                {
                    Turn_Angle(-100);
                }
                else
                {
                    Turn.Finish = false;
                    Image_Flag.Zerba = false;
                    MyFSM.Unload_State = Return_Line;
                    Car.Image_Flag = true;
                }
            }
        break;
        case Return_Line:
            #ifdef debug_switch
                printf("Return_Line = %d %d\r\n",Image_Flag.Zerba,MyFSM.Stop_Flag);    
            #endif
            Dodge_Carmar();
            if(Bufcnt(Image_Flag.Zerba,1700))
            {
                MyFSM.Stop_Flag = true;
            }

            if(MyFSM.Stop_Flag)
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                Car_run(5);
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
        case Cross_Board://ʮ�ֻػ���Ƭ
            Cross_BoardFsm();
        break;
        case Unload://�յ�ǰж��
            Unload_Fsm();
        break;
    }
}

