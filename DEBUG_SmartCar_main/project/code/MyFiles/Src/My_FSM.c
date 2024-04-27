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
    .LeftBoard_State = Find,
    .RightBoard_State = Find,
};

uint16 wait_time = 0;
#define Static_Time 200 //等待静止的时间，大约一秒
//#define debug_switch  //是否调试

/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   左边卡片状态机
-- @param   无
-- @auther  庄文标
-- @date    2024/4/27
**/
void Left_BoardFsm()
{
    static float Staic_Angle = 0;
    switch (MyFSM.LeftBoard_State)
    {
        case Find://找到卡片
            #ifdef debug_switch
                printf("Find\r\n");    
            #endif 
            if(Turn.Finish == false)
            {
                Turn_Angle(-90);//转动九十度
            }
            else
            {
                MyFSM.LeftBoard_State = Wait_Data;//等待移动数据的发回
                Staic_Angle = Gyro_YawAngle_Get();
                Turn.Finish = false;
            }
        break;
        case Wait_Data://等待串口数据回传
            #ifdef debug_switch
                printf("Wait_Data\r\n");    
            #endif 
            if(FINETUNING_DATA.dx!=0 && FINETUNING_DATA.dy!=0)
            {
                if(wait_time==0)
                {
                    wait_time = 1;
                }
                if(wait_time >= Static_Time)
                {
                    wait_time = 0; 
                    MyFSM.LeftBoard_State = Move;//开始移动到卡片前面
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            else
            {
                Car.Speed_X = 3;//往右前移动一点防止看不到卡片
                Car.Speed_Y = 3;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                // printf("Move");    
            #endif 
            // printf("%f,%f,%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f,Get_X_Distance(),Get_Y_Distance());
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.LeftBoard_State = Confirm;//确认是否移动到位
            }
        break;
        case Confirm://确认是否移动到位
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)//没有移动到位
            {
                if(FINETUNING_DATA.dx!=0 && FINETUNING_DATA.dy!=0)//确保有数据
                {
                    MyFSM.LeftBoard_State = Move;
                }
            }
            else
            {
                MyFSM.LeftBoard_State = Pick;//捡起卡片
                Staic_Angle = Gyro_YawAngle_Get();
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Pick://捡起卡片
            #ifdef debug_switch
                printf("Pick\r\n");    
            #endif
            if(wait_time==0)
            {
                wait_time = 1;
            }
            if(Servo_Flag.Put_Up == false)
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

/**@brief   右边卡片状态机
-- @param   无
-- @auther  庄文标
-- @date    2024/4/27
**/
void Right_BoardFsm()
{
    static float Staic_Angle = 0;
    switch (MyFSM.RightBoard_State)
    {
        case Find://找到卡片
            #ifdef debug_switch
                printf("Find\r\n");    
            #endif 
            if(Turn.Finish == false)
            {
                Turn_Angle(90);//转动九十度
            }
            else
            {
                MyFSM.RightBoard_State = Wait_Data;//等待移动数据的发回
                Staic_Angle = Gyro_YawAngle_Get();
                Turn.Finish = false;
            }
        break;
        case Wait_Data://等待串口数据回传
            #ifdef debug_switch
                printf("Wait_Data\r\n");    
            #endif 
            if(FINETUNING_DATA.dx!=0 && FINETUNING_DATA.dy!=0)
            {
                if(wait_time==0)
                {
                    wait_time = 1;
                }
                if(wait_time >= Static_Time)
                {
                    MyFSM.RightBoard_State = Move;//开始移动到卡片前面
                    wait_time = 0;
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            else
            {
                Car.Speed_X = -3;//往左前移动一点防止看不到卡片
                Car.Speed_Y = 3;
                Car.Speed_Z = Angle_Control(Staic_Angle);
            }
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                printf("Move");    
            #endif 
            // printf("%f,%f,%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f,Get_X_Distance(),Get_Y_Distance());
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process(FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.RightBoard_State = Confirm;//确认是否移动到位
            }
        break;
        case Confirm://确认是否移动到位
            #ifdef debug_switch
                printf("Confirm\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)//没有移动到位
            {
                if(FINETUNING_DATA.dx!=0 && FINETUNING_DATA.dy!=0)
                {
                    MyFSM.RightBoard_State = Move;
                }
            }
            else
            {
                MyFSM.RightBoard_State = Pick;//捡起卡片
                Staic_Angle = Gyro_YawAngle_Get();
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Pick://捡起卡片
            #ifdef debug_switch
                printf("Pick\r\n");    
            #endif
            if(wait_time==0)
            {
                wait_time = 1;
            }
            if(Servo_Flag.Put_Up == false)
            {
                if(wait_time >= 200)//等待一会让车停下来
                {
                    Pick_Card();
                }
            }
            else
            {
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                wait_time = 0;
                MyFSM.RightBoard_State = Return_Line;//返回赛道
            } 
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Return_Line://返回赛道
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
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   主状态机
-- @param   无
-- @auther  庄文标
-- @date    2024/4/27
**/
void FSM_main()
{
    switch (MyFSM.CurState)
    {
        case Line_Patrol://巡线状态
            if(Start == 1)
            {
                Car_run();
                Turn.Finish = false;
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
                    MyFSM.CurState = Line_Left_Board;//左边卡片
                }
                else if(FINDBORDER_DATA.dir == RIGHT)
                {
                    MyFSM.CurState = Line_Right_Board;//右边卡片
                }
            }
        break;
        case Line_Left_Board:
            Left_BoardFsm();
        break;
        case Line_Right_Board:
            Right_BoardFsm();
        break;
    }
    Set_Car_Speed(Car.Speed_X,Car.Speed_Y,Car.Speed_Z);//控制速度的线程
}

