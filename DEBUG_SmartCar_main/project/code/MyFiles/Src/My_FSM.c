/**
  ******************************************************************************
  * @file    My_FSM.c
  * @author  庄文标
  * @brief   主状态机
  *
    @verbatim
	无
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "My_FSM.h"

/* Define\Declare ------------------------------------------------------------*/
FSM_Handle MyFSM = {
    .CurState = Depart,
    .Line_Board_State = Find,
    .Unload_State = Find_Zebra,
    .Static_Angle = 0,
    .Board_Dir = -1,
    .Big_Count = 0,
    .Stop_Flag = false,
    .Big_Count_1 = 0,
    .Big_Count_2 = 0,
    .Big_Count_3 = 0,
    .Unload_Count = 0,
};

uint16 wait_time = 0;
#define Static_Time 100 //等待静止的时间，大约0.5秒
#define debug_switch  //是否调试

/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   发车状态机
-- @param   无
-- @author  庄文标
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

/**@brief    巡线状态机
-- @param    无
-- @verbatim 在此状态机跳转到其他卡片的状态机
-- @author   庄文标
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
        MyFSM.CurState = Line_Board;//散落卡片
    }
    else if(Bufcnt(Image_Flag.Zerba,500))
    {
        Car.Image_Flag = false;
        Image_Flag.Zerba = false;
        MyFSM.CurState = Unload;//卸货
    }
}

/**@brief   散落在赛道旁的卡片状态机
-- @param   无
-- @author  庄文标
-- @date    2024/5/10
**/
static void Line_BoardFsm()
{
    switch (MyFSM.Line_Board_State)
    {
        case Find://找到卡片
            #ifdef debug_switch
                printf("Find\r\n");    
            #endif 
            if(Turn.Finish == false)
            {
                if(MyFSM.Board_Dir == LEFT)
                {
                    Turn_Angle(-90);//转动九十度
                }
                else if(MyFSM.Board_Dir == RIGHT)
                {
                    Turn_Angle(90);//转动九十度
                }
            }
            else
            {
                MyFSM.Line_Board_State = Wait_Data;//等待移动数据的发回
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Turn.Finish = false;
            }
        break;
        case Wait_Data://等待串口数据回传
            #ifdef debug_switch
                printf("Wait_Data\r\n");    
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Line_Board_State = Move;//开始移动到卡片前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                }
                Car.Speed_X = 0;
            }
            else
            {
                if(MyFSM.Board_Dir == LEFT)
                {
                    Car.Speed_X = 2.0f;//往右移动一点防止看不到卡片
                }
                else
                {
                    Car.Speed_X = -2.0f;//往左移动一点防止看不到卡片
                }
            }
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Line_Board_State = Confirm;//确认是否移动到位
            }
        break;
        case Confirm://确认是否移动到位
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 2)
            {
                MyFSM.Line_Board_State = Move;//移动
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Line_Board_State = Move_Y;//Y轴移动
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
                Navigation_Process_Y(FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Line_Board_State = Confirm_Y;//确认是否移动到位
            }
        break;
        case Confirm_Y:
            #ifdef debug_switch
                printf("Confirm_Y\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Line_Board_State = Move_Y;//移动
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//发送数据，接收分类数据
                MyFSM.Line_Board_State = Classify;//识别分类
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Classify:
            #ifdef debug_switch
                printf("Classify\r\n");    
            #endif 
            if(CLASSIFY_DATA.type != None)//识别到了分类
            {
                Dodge_Board();
                MyFSM.Line_Board_State = Pick;//捡起卡片
                MyFSM.Big_Board = CLASSIFY_DATA.type;//记录分类
                if(MyFSM.Big_Board == 0)
                {
                    MyFSM.Big_Count_1+=1;
                }
                else if(MyFSM.Big_Board == 1)
                {
                    MyFSM.Big_Count_2+=1;
                }
                else if(MyFSM.Big_Board == 2)
                {
                    MyFSM.Big_Count_3+=1;
                }
                CLASSIFY_DATA.type = None;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Set_Beepfreq(MyFSM.Big_Board+1);
                Car.Depot_Pos = MyFSM.Big_Board;
                MyFSM.Big_Board = None;
            }
            else
            {
                if(Bufcnt(true,3000))//3s发送一次
                {
                    UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//发送数据，接收分类数据
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Pick://捡起卡片
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
                MyFSM.Line_Board_State = Return_Line;//返回赛道
            } 
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Return_Line://返回赛道
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

/**@brief   终点前卸货状态机
-- @param   无
-- @author  庄文标
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
                if(Car.Big_Pos_1 == RIGHT)
                {
                    Turn_Angle(90);
                }
                else if(Car.Big_Pos_1 == LEFT)
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
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//获取大类放置区域
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
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送对数字板微调信息
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Unload_State = Move;//开始移动到卡片前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                }
            }
            Car.Speed_X = 0;
            if(MyFSM.Big_Count == 0)
            {
                Car.Speed_Y = 2;
            }
            else
            {
                Car.Speed_Y = 0;
            }
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Unload_State = Confirm;//确认是否移动到位
            }
        break;
        case Confirm://确认是否移动到位
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == 0)
            {
                MyFSM.Unload_State = Move;//移动
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Unload_State = Move_Y;//Y轴移动
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
                Navigation_Process_Y(FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Unload_State = Confirm_Y;//确认是否移动到位
            }
        break;
        case Confirm_Y:
            #ifdef debug_switch
                printf("Confirm_Y\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//数字板微调
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Unload_State = Move_Y;//移动
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_BIGPLACE);//发送数据，接收大类分类数据
                MyFSM.Unload_State = Classify;//识别分类
                Servo_Flag.Depot_End = false;//先清零
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Classify:
            #ifdef debug_switch
                printf("Classify\r\n");    
            #endif 
            if(BIG_PLACE_DATA.Big_Place != None)//识别到了分类
            {
                Dodge_Board();
                MyFSM.Big_Board = BIG_PLACE_DATA.Big_Place;//记录分类
                BIG_PLACE_DATA.Big_Place = None;
                Set_Beepfreq(MyFSM.Big_Board+1);
                Car.Depot_Pos = MyFSM.Big_Board;
                MyFSM.Unload_State = Unload_Board;
                MyFSM.Big_Count +=1;
                if(MyFSM.Big_Board == 0)
                {
                    MyFSM.Unload_Count = MyFSM.Big_Count_1;
                }
                else if(MyFSM.Big_Board == 1)
                {
                    MyFSM.Unload_Count = MyFSM.Big_Count_2;
                }
                else if(MyFSM.Big_Board == 2)
                {
                    MyFSM.Unload_Count = MyFSM.Big_Count_3;
                }
                MyFSM.Big_Board = None;
            }
            else
            {
                if(Bufcnt(true,3000))//3s发送一次
                {
                    UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_BIGPLACE);//发送数据，接收大类分类数据
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
        break;
        case Unload_Next:
            #ifdef debug_switch
                printf("Unload_Next\r\n");    
            #endif
            if(MyFSM.Big_Count < 3)
            {
                if(Car.Big_Pos_1!=Car.Big_Pos_2 || Car.Big_Pos_2!=Car.Big_Pos_3)
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
                        Navigation_Process(60);
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
            if(Bufcnt(Image_Flag.Zerba,1500))
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
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   主状态机
-- @param   无
-- @author  庄文标
-- @date    2024/4/27
**/
void FSM_main()
{
    switch (MyFSM.CurState)
    {
        case Depart://发车状态
            Depart_Fsm();
        break;
        case Line_Patrol://巡线状态
            Line_PatrolFsm();
        break;
        case Line_Board://赛道散落卡片
            Line_BoardFsm();
        break;
        case Unload://终点前卸货
            Unload_Fsm();
        break;
    }
}

