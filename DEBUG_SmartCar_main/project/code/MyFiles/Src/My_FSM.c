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
    else if(Image_Flag.Cross_Fill == 2)
    {
        MyFSM.CurState = Cross_Board;//十字回环状态机
        MyFSM.Cross_Dir = Image_Flag.Cross_Type;
        MyFSM.Cross_Flag_ = true;
        Set_Beepfreq(1);
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
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
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
                Navigation_Process_Y(0,FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
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
                MyFSM.Big_Count[MyFSM.Big_Board]+=1;
                CLASSIFY_DATA.type = None;
                Set_Beepfreq(MyFSM.Big_Board+1);
                MyFSM.Depot_Pos = MyFSM.Big_Board;
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
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
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

/**@brief   十字回环卡片状态机
-- @param   无
-- @author  庄文标
-- @date    2024/5/10
**/
static void Cross_BoardFsm()
{
    static float Curanglg = 0;
    static float Lastanglg = 0;
    static float Angle_Offest = 0;
    switch (MyFSM.Cross_Board_State)
    {
        case Find_Cross://找到十字
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
        case Find://找到十字后准备转向
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
        case Wait_Data://等待串口数据回传
            #ifdef debug_switch
                printf("Cross_Wait_Data\r\n");    
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,800))
                {
                    MyFSM.Cross_Board_State = Move;//开始移动到卡片前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                }
                Car.Speed_X = 0;
            }
            else
            {
                if(MyFSM.Cross_Dir == LEFT)
                {
                    Car.Speed_X = 2;//往右移动一点防止看不到卡片
                }
                else
                {
                    Car.Speed_X = -2;//往左移动一点防止看不到卡片
                }
            }
            Car.Speed_Y = 2;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                printf("Cross_Move:%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm;//确认是否移动到位
            }
        break;
        case Confirm://确认是否移动到位
            #ifdef debug_switch
                printf("Cross_Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 2)
            {
                MyFSM.Cross_Board_State = Move;//移动
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Cross_Board_State = Move_Y;//Y轴移动
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
                Navigation_Process_Y(0,FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm_Y;//确认是否移动到位
            }
        break;
        case Confirm_Y:
            #ifdef debug_switch
                printf("Cross_Confirm_Y\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Cross_Board_State = Move_Y;//移动
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//发送数据，接收分类数据
                MyFSM.Cross_Board_State = Classify;//识别分类
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle); 
        break;
        case Classify:
            #ifdef debug_switch
                printf("Cross_Classify\r\n");    
            #endif 
            if(CLASSIFY_DATA.place != nil)//识别到了分类
            {
                Dodge_Board();
                MyFSM.Cross_Board_State = Pick;//捡起卡片
                MyFSM.Small_Board[MyFSM.Small_Count] = CLASSIFY_DATA.place;//记录分类
                CLASSIFY_DATA.place = nil;
                Set_Beepfreq(1);
                if(MyFSM.Small_Count == 4)
                {
                    MyFSM.Depot_Pos = MyFSM.Small_Count - 1;//设置仓库位置
                }
                else
                {
                    MyFSM.Depot_Pos = MyFSM.Small_Count;
                }
                MyFSM.Small_Count+=1;
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
                    MyFSM.Cross_Board_State = Classify;//继续识别
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
                        MyFSM.Cross_Board_State = Find_Place;//寻找放置位置
                    }
                }
            } 
        break;
        case Find_Place://寻找放置位置
            #ifdef debug_switch
                printf("Find_Place:%d,%f\r\n",MyFSM.Unload_Count,Angle_Offest);    
            #endif 
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//获取大类放置区域
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
        case Wait_PlaceData://等待串口传回放置区域的信息
            #ifdef debug_switch
                printf("Cross_Wait_PlaceData\r\n");    
            #endif 
            Dodge_Carmar();
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送获取放置区域信息
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Cross_Board_State = Move_Place;//开始移动到放置区域前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move_Place://移动到卡片前面
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("Cross_Place:%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm_Place;//确认是否移动到位
            }
        break;
        case Confirm_Place://确认是否移动到位
            #ifdef debug_switch
                printf("Cross_Confirm_Place\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == 0)
            {
                MyFSM.Cross_Board_State = Move_Place;//移动
            }
            else
            {
                Set_Beeptime(200);
                MyFSM.Cross_Board_State = Move_PlaceY;//Y轴移动
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
                Navigation_Process_Y(0,FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Confirm_PlaceY;//确认是否移动到位
            }
        break;
        case Confirm_PlaceY:
            #ifdef debug_switch
                printf("Confirm_PlaceY\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//数字板微调
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG != 1)
            {
                MyFSM.Cross_Board_State = Move_PlaceY;//移动
            }
            else
            {
                Set_Beeptime(200);
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE);//发送数据，接收大类分类数据
                MyFSM.Cross_Board_State = Classify_Place;//识别分类
                Servo_Flag.Depot_End = false;//先清零
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Classify_Place:
            #ifdef debug_switch
                printf("Classify_Place\r\n");    
            #endif 
            if(SMALL_PLACE_DATA.place != nil)//识别到了分类
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
                if(Bufcnt(true,3000))//3s发送一次
                {
                    UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE);//发送数据，接收小类分类数据
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
                    UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//继续获取道路旁卡片
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
                    UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE);//继续获取道路旁卡片
                }                
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
        case Move://移动到卡片前面
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,0);//移动
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
                Navigation_Process(0,FINETUNING_DATA.dy/10.f);//移动
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
                MyFSM.Depot_Pos = MyFSM.Big_Board;//转动转盘
                MyFSM.Big_Pos_Count +=1;//记录去了几个大类卸货点
                MyFSM.Unload_Count = MyFSM.Big_Count[MyFSM.Big_Board];
                MyFSM.Big_Board = None;
                MyFSM.Unload_State = Unload_Board;
            }
            else
            {
                if(Bufcnt(true,300))//3s发送一次
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
        case Cross_Board://十字回环卡片
            Cross_BoardFsm();
        break;
        case Unload://终点前卸货
            Unload_Fsm();
        break;
    }
}

