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
    .CurState = Line_Patrol,
    .Line_Board_State = Find,
    .Static_Angle = 0,
    .Board_Dir = -1,
};

uint16 wait_time = 0;
#define Static_Time 100 //等待静止的时间，大约0.5秒
// #define debug_switch  //是否调试

/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   散落在赛道旁的卡片状态机
-- @param   无
-- @author  庄文标
-- @date    2024/5/10
**/
void Line_BoardFsm()
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
            if((FINETUNING_DATA.dx!=0) || (FINETUNING_DATA.dy!=0))
            {
                if(Bufcnt(true,500))
                {
                    MyFSM.Line_Board_State = Move;//开始移动到卡片前面
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                if(MyFSM.Board_Dir == LEFT)
                {
                    Car_run_X(3.0f);//往右移动一点防止看不到卡片
                }
                else if(MyFSM.Board_Dir == RIGHT)
                {
                    Car_run_X(-3.0f);//往左移动一点防止看不到卡片
                }
            }
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                // printf("Move\r\n");    
                printf("%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);
            #endif 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Line_Board_State = Classify;//识别分类
            }
        break;
        case Confirm://确认是否移动到位
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)
            {
                MyFSM.Line_Board_State = Move;//移动
            }
            else
            {
                Set_Beeptime(200);
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
                MyFSM.Line_Board_State = Pick;//捡起卡片
                MyFSM.Big_Board = CLASSIFY_DATA.type;//记录分类
                printf("class = %d\r\n",CLASSIFY_DATA.type);
                CLASSIFY_DATA.type = None;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Set_Beepfreq(MyFSM.Big_Board+1);
                Rotary_Switch(MyFSM.Big_Board,false);//转动转盘
                MyFSM.Big_Board = None;
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//发送数据，接收分类数据
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
                if(wait_time != 1)
                {
                    if(Bufcnt(true,200))//等待一会让车停下来
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
                Turn.Finish = false;
                MyFSM.Line_Board_State = Find;
                MyFSM.CurState = Line_Patrol;
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
        case Line_Patrol://巡线状态
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
                    MyFSM.CurState = Line_Board;//左边卡片
                }
            }
            else
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Line_Board://赛道散落卡片
            Line_BoardFsm();
        break;
    }
}

