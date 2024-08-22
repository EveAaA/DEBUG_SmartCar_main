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
    .Ring_Board_State = Find_Ring,
    .Static_Angle = 0,
    .Board_Dir = -1,
    .Stop_Flag = false,
    .Big_Count[0] = 0,
    .Big_Count[1] = 0,
    .Big_Count[2] = 0,
    .Small_Count = 0,
    .Same_Type = 0,
    .Pick_Count = 0,
    .Small_Depot_Count = 0,
    .Same_Board_Flag = false,
    .Cross_Flag_ = false,
    .Unload_Count = 0,
    .Big_Pos_Count = 0,
    .Depot_Pos = White,
    .Take_Board_Out = true,//true为逐张拿出方案
    .Big_Pos[0] = RIGHT,
    .Big_Pos[1] = RIGHT,
    .Big_Pos[2] = RIGHT,
    .Ring_Flag = false,
    .Simple_Flag = 1,
    .Big_Point = 0,
};

ATB_t BigWare[3];
uint8 After_Big[10] = {0,0,0};
WareState_t smallPlaceWare =
{
    .isSame = false,
    .currWareNum = 0,
    .notEmptyNum = 0,
    .isWareUsed = {false, false, false, false, false},
};
GetCard_t *currCard;

uint16 wait_time = 0;
#define Static_Time 100 // 等待静止的时间，大约0.5秒
// #define debug_switch  //是否调试
// #define Classify_debug //是否查看分类
// #define Final_Change //是否放置区域在道路两边
/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
 **/
/**
 * @brief: 重置仓库
 * @param：WareState_t结构体指针
 * @return:None
 */
void resetWare(WareState_t *Ware)
{
    uint8 i;
    Ware->notEmptyNum = 0;
    Ware->currWareNum = 0;
    for (i = 0; i < 5; i++)
    {
        Ware->isWareUsed[i] = false;
        Ware->list[i].CardName = nil;
        Ware->list[i].cardNum = 0;
        Ware->list[i].WareIndex = 0;
    }
}

/**
 * @brief : 将卡片放置到对应的仓库
 *
 * @param Ware
 * @param place
 * @param Depot
 * @return None
 */
void putCardIntoWare(WareState_t *Ware, Place_t place, Rotaryservo_Handle *Depot)
{
    uint8 i;
    Ware->isSame = false;
    // 在已有的仓库里查找是否有相同类别的卡片存在仓库内
    for (i = 0; i < Ware->currWareNum; i++)
    {
        // 如果有卡片存在
        if (place == Ware->list[i].CardName)
        {
            Ware->list[i].cardNum++;          // 当前卡片仓库卡片数量加一
            *Depot = Ware->list[i].WareIndex; // 将对应位置下标赋值给Depot
            // printf("当前卡片种类: %s, 当前仓库对应种类: %s, 当前仓库下标: %d 当前仓库对应卡片数量: %d \r\n", PLACE_TABLE_STR[place], PLACE_TABLE_STR[Ware->list[i].CardName], Ware->list[i].WareIndex, Ware->list[i].cardNum);
            Ware->isSame = true;
            break;
        }
    }
    // 如果没有与识别的类别相同的仓库， 则开辟一个新的仓库
    if (Ware->isSame == false)
    {
        uint8 j;
        Ware->notEmptyNum++; // 已用仓库加一 对应仓库闲置表
        // 如果创建的仓库大于5个直接退出循环
        if (Ware->notEmptyNum == 6)
        {
            // printf("类别超过5个\r\n");
            return;
        }
        Ware->list[Ware->currWareNum].CardName = place; // 记录当前仓库对应的卡片种类
        Ware->list[Ware->currWareNum].cardNum = 1;      // 假定卡片已经捡起
        // 在剩余的仓库里寻找一个闲置的仓库
        for (j = 0; j < 5; j++)
        {
            // 找到了闲置的仓库
            if (Ware->isWareUsed[j] == false)
            {
                Ware->list[Ware->currWareNum].WareIndex = j; // 将这个闲置仓库的下标赋值给list结构体，便于后续如果有相同卡片直接获取仓库下标位置
                Ware->isWareUsed[j] = true;                  // 将该仓库置为有类别状态
                *Depot = j;                                  // 将最新的下标赋值给Depot
                // printf("index: %d  :%d %d %d %d %d \r\n",j, Ware->isWareUsed[0], Ware->isWareUsed[1], Ware->isWareUsed[2], Ware->isWareUsed[3], Ware->isWareUsed[4]);
                break;
            }
        }
        Ware->currWareNum++; // 当前已用仓库数量加一 对应仓库存储单元
        // printf("新建仓库\r\n");
        // printf("当前卡片种类: %s, 当前仓库对应种类: %s, 当前仓库下标: %d 当前仓库对应卡片数量: %d \r\n", PLACE_TABLE_STR[place], PLACE_TABLE_STR[Ware->list[i].CardName], Ware->list[i].WareIndex, Ware->list[i].cardNum);
    }
}

/**
 * @brief 转动转盘到待拿出的仓库
 *
 * @param ware
 * @param place
 * @return Rotaryservo_Handle
 */
Rotaryservo_Handle takeOutFromWare(WareState_t *ware, Place_t place, uint8 *num, GetCard_t* currWare)
{
    *num = 0;
    for (uint8 i = 0; i < 5; i++)
    {
        if (place == ware->list[i].CardName)
        {
            *num = ware->list[i].cardNum;
            currWare = &(ware->list[i]);
            return ware->list[i].WareIndex;
        }
    }
    return 0;
}

void checkBigPlaceWare(WareState_t *ware)
{
    for (uint8_t i = 0; i < 5; ++i)
    {
        switch (ware->list[i].WareIndex)
        {
        case 0:
            if (ware->list[i].cardNum != 0)
            {
                BigWare[0].cardNum = ware->list[i].cardNum;
            }
            break;
        case 1:
            if (ware->list[i].cardNum != 0)
            {
                BigWare[1].cardNum = ware->list[i].cardNum;
            }
            break;
        case 2:
            if (ware->list[i].cardNum != 0)
            {
                BigWare[2].cardNum = ware->list[i].cardNum;
            }
            break;
        default:
            break;
        }
    }
}

/**@brief   发车状态机
-- @param   无
-- @author  庄文标
-- @date    2024/6/5
**/
static void Depart_Fsm()
{
    if (Start == 1)
    {
        UART_SendByte(&_UART_FINDBORDER, 0xce); // 发送获取放置区域信息

        if (Navigation.Finish_Flag == false)
        {
            Navigation_Process_Y(0, 30);
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
    if (HARDBORDER_DATA.FINDBORDER_FLAG == true && !Image_Flag.Roadblock && !Image_Flag.Ramp)
    {
        Forward_Speed = 4;
    }
    else
    {
        if (All_Stright() && !Image_Flag.Ramp)
        {
            Forward_Speed = 8;
            Car.Speed = true;
        }
        else if(Image_Flag.Ramp)
        {
            if(Image_Flag.Ramp == 1)
            {
                Forward_Speed = 6;
                Car.Speed = true;
            }
            else if(Image_Flag.Ramp == 2)
            {
                Forward_Speed = 4;
                Car.Speed = true;
            }
        }
        else
        {
            Forward_Speed = 6;
            Car.Speed = true;
        }
    }
    Car_run(Forward_Speed);
    Dodge_Carmar();

    if ((Image_Flag.Cross_Fill == 2) && (MyFSM.Simple_Flag !=0)) // 十字回环
    {
        MyFSM.CurState = Cross_Board; // 十字回环状态机
        Car.Speed = false;
        Set_Beepfreq(1);
    }
    else if (((Image_Flag.Right_Ring) || (Image_Flag.Left_Ring)) && (MyFSM.Simple_Flag !=0)) // 圆环卡片
    {
        MyFSM.CurState = Ring_Board; // 圆环状态机
        Car.Speed = false;
        if (Image_Flag.Left_Ring)
        {
            MyFSM.Ring_Dir = LEFT;
        }
        else if (Image_Flag.Right_Ring)
        {
            MyFSM.Ring_Dir = RIGHT;
        }
        Set_Beepfreq(2);
    }
    else if (Bufcnt(Image_Flag.Zerba, 100)) // 终点前卸货
    {
        Car.Image_Flag = false;
        Car.Speed = false;
        Image_Flag.Zerba = false;
        MyFSM.CurState = Unload; // 卸货
        Set_Beepfreq(3);
    }
    else if (((HARDBORDER_DATA.dir == LEFT) || (HARDBORDER_DATA.dir == RIGHT)) && !Image_Flag.Roadblock && !Image_Flag.Ramp) // 散落卡片
    {
        MyFSM.Board_Dir = HARDBORDER_DATA.dir;
        HARDBORDER_DATA.FINDBORDER_FLAG = false;
        HARDBORDER_DATA.dir = STRAIGHT;
        Car.Speed = false;
        Car.Image_Flag = false;
        MyFSM.CurState = Line_Board; // 散落卡片
    }
}

/**@brief   散落在赛道旁的卡片状态机
-- @param   无
-- @author  庄文标
-- @date    2024/5/10
**/
static void Line_BoardFsm()
{
    static bool Return_End = false;
    switch (MyFSM.Line_Board_State)
    {
        case Find: // 找到卡片
    #ifdef debug_switch
            printf("Find\r\n");
    #endif
            Car.Speed = false;
            if (Turn.Finish == false)
            {
                if (MyFSM.Board_Dir == LEFT)
                {
                    Turn_Angle(-80); // 转动九十度
                }
                else if (MyFSM.Board_Dir == RIGHT)
                {
                    Turn_Angle(80); // 转动九十度
                }
            }
            else
            {
                MyFSM.Line_Board_State = Wait_Data; // 等待移动数据的发回
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Turn.Finish = false;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                Light_On;//开灯
            }
        break;
        case Wait_Data: // 等待串口数据回传
    #ifdef debug_switch
         printf("Wait_Data\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, START_FINETUNING_BESIDE); // 发送数据
            if (UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (Bufcnt(true, 100))
                {
                    if (FINETUNING_DATA.dy == -999 || FINETUNING_DATA.dx == -999)
                    {
                        MyFSM.Line_Board_State = Return_Line; // 不是目标板返回赛道
                        FINETUNING_DATA.dx = 0;
                        FINETUNING_DATA.dy = 0;
                        Light_Off;//关灯
                    }
                    else if (FINETUNING_DATA.IS_BORDER_ALIVE == false)
                    {
                        MyFSM.Line_Board_State = Return_Line; // 不是目标板返回赛道
                        FINETUNING_DATA.dx = 0;
                        FINETUNING_DATA.dy = 0;
                        Light_Off;//关灯
                    }
                    else
                    {
                        MyFSM.Line_Board_State = Move; // 开始移动到卡片前面
                        MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                        MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                    }
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Move: // 移动到卡片前面
    #ifdef debug_switch
            printf("%f,%f\r\n", FINETUNING_DATA.dx / 10.f, FINETUNING_DATA.dy / 10.f);
    #endif
            CLASSIFY_DATA.type = None;
            CLASSIFY_DATA.place = nil;
            UART_SendByte(&_UART_FINDBORDER, START_FINETUNING_BESIDE); //发送数据
            if (Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X, MyFSM.Target_Pos_Y); // 移动
            }
            else
            {
                UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
                Set_Beeptime(200);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
                MyFSM.Line_Board_State = Classify; // 识别
            }
        break;
        case Classify:
    #ifdef debug_switch
            printf("Classify\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            if (CLASSIFY_DATA.type != None) // 识别到了分类
            {
                // Dodge_Board();
                Light_Off;//关灯
                MyFSM.Line_Board_State = Pick; // 捡起卡片
                Servo_Flag.Put_Down_End = false;
                MyFSM.Pick_Count = 9;
                Servo_Flag.Depot_End = true;
                MyFSM.Big_Board = CLASSIFY_DATA.type; // 记录分类
                #ifdef Classify_debug
                printf("line=%s\r\n", PLACE_TABLE_STR[CLASSIFY_DATA.place]);
                #endif
                MyFSM.Big_Count[MyFSM.Big_Board] += 1;
                CLASSIFY_DATA.type = None;
                CLASSIFY_DATA.place = nil;
                Set_Beepfreq(MyFSM.Big_Board + 1);
                MyFSM.Depot_Pos = MyFSM.Big_Board;
                MyFSM.Big_Board = None;
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC); // 发送数据，接收分类数据
                if (Bufcnt(true, 5000))
                {
                    MyFSM.Line_Board_State = Time_Out;//超时
                    Light_Off;//关灯
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Pick: // 捡起卡片
    #ifdef debug_switch
            printf("Pick\r\n");
    #endif
            CLASSIFY_DATA.type = None;
            CLASSIFY_DATA.place = nil;
            if (Servo_Flag.Pick_End == false)
            {
                Pick_Card();
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            // else
            // {
            //     Servo_Flag.Put_Up = false;
            //     Servo_Flag.Put_Down = false;
            //     // Servo_Flag.Pick_End = false;
            //     // MyFSM.Line_Board_State = Return_Line; // 返回赛道
            // }

            if(Servo_Flag.Put_Down_End)
            {
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Pick_Count = 0;
                if (Turn.Finish == false)
                {
                    if (MyFSM.Board_Dir == LEFT)
                    {
                        Turn_Angle(80);
                    }
                    else if (MyFSM.Board_Dir == RIGHT)
                    {
                        Turn_Angle(-80);
                    }
                    Car.Image_Flag = true;
                }
                else
                {
                    Return_End = true;
                    Servo_Flag.Put_Down_End = false;
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                }
            }

            if(Return_End && Servo_Flag.Pick_End)
            {
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Put_Down_End = false;
                Turn.Finish = false;
                Return_End = false;
                Servo_Flag.Pick_End = false;
                RightRing.Ring_State = Ring_Front;
                RightRing.Ring_Front_Flag = 0; 
                LeftRing.Ring_State = Ring_Front;
                LeftRing.Ring_Front_Flag = 0; 
                MyFSM.Line_Board_State = Return_Line;
            }
        break;
        case Time_Out:
            if (Turn.Finish == false)
            {
                if (MyFSM.Board_Dir == LEFT)
                {
                    Turn_Angle(80);
                }
                else if (MyFSM.Board_Dir == RIGHT)
                {
                    Turn_Angle(-80);
                }
                Car.Image_Flag = true;
            }
            else
            {
                Turn.Finish = false;
                MyFSM.Line_Board_State = Return_Line;
            }            
        break;
        case Return_Line: // 返回赛道
    #ifdef debug_switch
            printf("Return\r\n");
    #endif
            FINETUNING_DATA.dx = 0;
            FINETUNING_DATA.dy = 0;
            MyFSM.Pick_Count = 0;
            RightRing.Ring_State = Ring_Front;
            RightRing.Ring_Front_Flag = 0; 
            LeftRing.Ring_State = Ring_Front;
            LeftRing.Ring_Front_Flag = 0; 
            if (MyFSM.Board_Dir == LEFT)
            {
                if (No_Get_Line() && (!Return_End))
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 3;
                }
                else
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                    MyFSM.Line_Board_State = Finsh_Return;
                }
            }

            if (MyFSM.Board_Dir == RIGHT)
            {
                if (No_Get_Line() && (!Return_End))
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = -3;
                }
                else
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                    MyFSM.Line_Board_State = Finsh_Return;
                }
            }
        break;
        case Finsh_Return:
    #ifdef debug_switch
            printf("Finsh_Return\r\n");
    #endif
            if (Bufcnt(true,500))
            {
                MyFSM.Line_Board_State = Find;
                MyFSM.CurState = Line_Patrol;
                MyFSM.Pick_Count = 0;
            }
            Car_run(4);   
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
    static uint8 Cross = 0;
    switch (MyFSM.Cross_Board_State)
    {
        case Find_Cross: // 找到十字
        #ifdef debug_switch
                printf("Find_Cross\r\n");
        #endif
            Car.Speed = false;
            Car_run(5);
            if (Bufcnt((Image_Flag.Cross_Fill == false), 1400))
            {
                if ((Points_R[Data_Stastics_R][0] + Points_L[Data_Stastics_L - 1][0]) / 2 >= Image_W / 2)
                {
                    MyFSM.Cross_Dir = RIGHT;
                }
                else
                {
                    MyFSM.Cross_Dir = LEFT;
                }
                MyFSM.Cross_Board_State = Find;
                Car.Image_Flag = false;
            }
        break;
        case Find: // 找到十字后准备转向
            if (Turn.Finish == false)
            {
                if (MyFSM.Cross_Dir == RIGHT)
                {
                    Turn_Angle(80);
                }
                else
                {
                    Turn_Angle(-80);
                }
            }
            else
            {
                if(Bufcnt(true,500))
                {
                    Turn.Finish = false;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    MyFSM.Cross_Board_State = Forward_;
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                    FINETUNING_DATA.IS_BORDER_ALIVE = true;
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Forward_:
            if(Bufcnt(true,600))
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;   
                MyFSM.Cross_Board_State = Wait_Data;  
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Wait_Data: // 等待串口数据回传
    #ifdef debug_switch
            // printf("cross1:%d,%d\r\n", UnpackFlag.FINETUNING_DATA_FLAG, FINETUNING_DATA.IS_BORDER_ALIVE);
            printf("Cross_Move+!!!!:%f,%f\r\n", FINETUNING_DATA.dx / 10.f, FINETUNING_DATA.dy / 10.f);
    #endif
            Light_On;//开灯
            UART_SendByte(&_UART_FINDBORDER, START_FINETUNING); // 发送数据
            if (UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (FINETUNING_DATA.IS_BORDER_ALIVE)
                {
                    if (Bufcnt(true, 500))
                    {
                        MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                        MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                        MyFSM.Cross_Board_State = Move; // 开始移动到卡片前面
                    }
                }
                else
                {
                    Light_Off;//关灯
                    if (MyFSM.Pick_Count > 7)
                    {
                        MyFSM.Cross_Board_State = Ready_Find_Place;
                    }
                    else
                    {
                        MyFSM.Cross_Board_State = No_Board_Return;
                    }

                    if(Bufcnt(true,5000))//超时退出
                    {
                        MyFSM.Cross_Board_State = No_Board_Return;
                    }
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Move: // 移动到卡片前面
    #ifdef debug_switch
            printf("Cross_Move:%f,%f\r\n", FINETUNING_DATA.dx / 10.f, FINETUNING_DATA.dy / 10.f);
    #endif
            UART_SendByte(&_UART_FINDBORDER, START_FINETUNING); // 发送数据
            if (Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X, MyFSM.Target_Pos_Y); // 移动
            }
            else
            {
                Set_Beeptime(200);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                resetWare(&smallPlaceWare); // 重置仓库
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                MyFSM.Cross_Board_State = Classify; // 识别
                CLASSIFY_DATA.IS_CLASSIFY = true;
                MyFSM.Pick_Count = 0;
                UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            }
        break;
        case Classify:
    #ifdef debug_switch
            // printf("Cross_Classify\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            if (UnpackFlag.FINETUNING_DATA_FLAG) // 接收到数据了
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (CLASSIFY_DATA.place != nil) // 识别到了分类
                {
                    Dodge_Board();
                    MyFSM.Cross_Board_State = Pick;                                          // 捡起卡片
                    putCardIntoWare(&smallPlaceWare, CLASSIFY_DATA.place, &MyFSM.Depot_Pos); // 将卡片放入对应仓库
                    After_Big[MyFSM.Depot_Pos] += 1;
                    #ifdef Classify_debug
                    printf("Cross=%s\r\n", PLACE_TABLE_STR[CLASSIFY_DATA.place]);
                    #endif
                    CLASSIFY_DATA.place = nil;
                    CLASSIFY_DATA.type = None;
                    Servo_Flag.Depot_End = true;
                    Set_Beepfreq(1);
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                }
                else if (!CLASSIFY_DATA.IS_CLASSIFY) // 没有卡片
                {
                    MyFSM.Cross_Board_State = Ready_Find_Place;
                    Light_Off;//关灯
                    MyFSM.Pick_Count = 0;
                }
            }
            else // 没有接受到数据
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC); // 发送数据，接收分类数据
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Pick: // 捡起卡片
    #ifdef debug_switch
                //  printf("Cross_Pick\r\n");
    #endif
            CLASSIFY_DATA.type = None;
            CLASSIFY_DATA.place = nil;
            if (Servo_Flag.Pick_End == false)
            {
                Pick_Card();
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                MyFSM.Pick_Count += 1;
                if (MyFSM.Pick_Count > 7) // 出现大于7次捡起卡片说明卡片飞了或者是见不到，直接去放卡片
                {
                    MyFSM.Cross_Board_State = Ready_Find_Place;
                }
                else
                {
                    MyFSM.Cross_Board_State = Classify; // 继续识别
                }
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Ready_Find_Place: // 准备找第一个放卡片的位置
            if (Turn.Finish == false)
            {
                Turn_Angle(180);
            }
            else
            {
                Turn.Finish = false;
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
                MyFSM.Cross_Board_State = Ring_First_Place; // 寻找放置位置
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Ring_First_Place: // 直接往前走
    #ifdef debug_switch
            printf("Cross_First_Place\r\n");
    #endif
            Dodge_Carmar();
            Light_On;//开灯
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送获取放置区域信息
            if (UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (Bufcnt(true,700))
                {
                    MyFSM.Cross_Board_State = Move_Place; // 开始移动到放置区域前面
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                Car.Speed_X = 1;
                Car.Speed_Y = 3;
                Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
            }
        break;
        case Move_Place: // 移动到卡片前面
        #ifdef debug_switch
        //  printf("Move\r\n");
        //  printf("Cross_Place:%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);
        #endif
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送获取放置区域信息
            if (Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X, MyFSM.Target_Pos_Y); // 移动
            }
            else
            {
                Set_Beepfreq(1);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Classify_Place; // 识别放置区域
                UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            }
        break;
        case Classify_Place:
    #ifdef debug_switch
            // printf("Classify_Place\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            if (SMALL_PLACE_DATA.place != nil) // 识别到了分类
            {
                #ifdef Classify_debug
                printf("%s\r\n", PLACE_TABLE_STR[SMALL_PLACE_DATA.place]);
                #endif
                MyFSM.Unload_Count += 1;
                Dodge_Board();
                Light_Off;//开灯
                Servo_Flag.Depot_End = true;
                MyFSM.Depot_Pos = takeOutFromWare(&smallPlaceWare, SMALL_PLACE_DATA.place, &MyFSM.Small_Count, currCard);
                Set_Beepfreq(1);
                SMALL_PLACE_DATA.place = nil;
                if (MyFSM.Small_Count != 0)
                {
                    MyFSM.Cross_Board_State = Unload_Board;
                }
                else if ((MyFSM.Small_Count == 0) && (MyFSM.Unload_Count !=1)) // 没有卡片且不是第一个卸货点
                {
                    MyFSM.Cross_Board_State = Ready_Find_Next;
                }
                else if ((MyFSM.Small_Count == 0) && (MyFSM.Unload_Count ==1)) // 没有卡片且是第一个卸货点
                {
                    MyFSM.Cross_Board_State = Ready_Find_Next_First;
                }
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE); // 发送数据，接收小类分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Unload_Board:
        #ifdef debug_switch
                printf("Cross_Unload_Board:%d\r\n", MyFSM.Unload_Count);
        #endif
            SMALL_PLACE_DATA.place = nil;
            if (Servo_Flag.Depot_End)
            {
                if (!Servo_Flag.Put_Out)
                {
                    Take_Card_Out();
                }
                else
                {
                    Servo_Flag.Put_Out = false;
                    MyFSM.Small_Count -= 1;
                    After_Big[MyFSM.Depot_Pos] -= 1;
                    // printf("Place: %s\r\n", PLACE_TABLE_STR[currCard->CardName]);
                    // printf("Num: %d\r\n", currCard->cardNum);
                    if ((MyFSM.Small_Count == 0) && (MyFSM.Unload_Count !=1)) // 卡片卸完且不到一个卸货点
                    {
                        MyFSM.Cross_Board_State = Ready_Find_Next;
                        MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    }
                    else if (MyFSM.Small_Count != 0) // 还有卡片
                    {
                        MyFSM.Cross_Board_State = Unload_Board;
                    }
                    else if ((MyFSM.Unload_Count ==1) && (MyFSM.Small_Count == 0)) // 卡片卸完且是第一个卸货点
                    {
                        MyFSM.Cross_Board_State = Ready_Find_Next_First;
                        MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    }
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Ready_Find_Next_First:
        #ifdef debug_switch
                printf("Ready_Find_Next_First\r\n");
        #endif
            MyFSM.Depot_Pos = 0;
            SMALL_PLACE_DATA.place = nil;
            if (Bufcnt(true, 1100))
            {
                if (MyFSM.Unload_Count >= 5)
                {
                    MyFSM.Cross_Board_State = Adjust_Line;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                }
                else
                {
                    MyFSM.Cross_Board_State = Find_Place;
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }   
            // if(MyFSM.Cross_Dir == RIGHT)
            // {
            //     Car.Speed_X = 1.5;
            // }
            // else
            // {
            //     Car.Speed_X = -1.5;
            // }
            Car.Speed_X = 0;
            Car.Speed_Y = -3.5;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Ready_Find_Next:
            SMALL_PLACE_DATA.place = nil;
            MyFSM.Depot_Pos = 0;
            if (Bufcnt(true, 800))
            {
                if (MyFSM.Unload_Count >= 5)
                {
                    MyFSM.Cross_Board_State = Adjust_Line;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                }
                else
                {
                    MyFSM.Cross_Board_State = Find_Place;
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }   
            Car.Speed_X = 0;
            Car.Speed_Y = -3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Find_Place: // 寻找放置位置
        #ifdef debug_switch
            //  printf("Find_Place\r\n");
        #endif
            MyFSM.Depot_Pos = 0;
            Light_On;//开灯
            SMALL_PLACE_DATA.place = nil;
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETSMALLPLACE); // 获取小类放置区域
            Dodge_Carmar();
            Curanglg = Gyro_YawAngle_Get();
            if (Lastanglg == 0)
            {
                Lastanglg = Curanglg;
            }
            Angle_Offest += (Curanglg - Lastanglg);
            if (MyFSM.Cross_Dir == RIGHT)
            {
                Car_run_X(2.5);
            }
            else
            {
                Car_run_X(-2.5);
            }
            Lastanglg = Curanglg;
            if (MyFSM.Unload_Count == 0 && FINDBORDER_DATA.FINDBIGPLACE_FLAG == true)
            {
                Set_Beepfreq(1);
                Light_On;//开灯
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Cross_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
            else if (FINDBORDER_DATA.FINDBIGPLACE_FLAG == true && (fabs(Angle_Offest) >= 30))
            {
                Set_Beepfreq(1);
                Light_On;//开灯
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Cross_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
        break;
        case Wait_PlaceData: // 等待串口传回放置区域的信息
    #ifdef debug_switch
        //printf("Cross_Wait_PlaceData\r\n");
    #endif
            FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
            Dodge_Carmar();
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送获取放置区域信息
            if (UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if (Bufcnt(true, 500))
                {
                    MyFSM.Cross_Board_State = Move_Place; // 开始移动到放置区域前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                }
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 3;
                Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
            }
        break;
        case Adjust_Line://调整
            if(MyFSM.Cross_Dir == RIGHT)
            {
                Car.Speed_X = 5;
            }
            else
            {
                Car.Speed_X = -5;
            }
            Car.Speed_Y = 0;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
            if(Bufcnt(true,1260))
            {
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Cross_Board_State = Return_Line;
                // checkBigPlaceWare(&smallPlaceWare);
                MyFSM.Big_Count[0] += After_Big[0];
                MyFSM.Big_Count[1] += After_Big[1];
                MyFSM.Big_Count[2] += After_Big[2];
                After_Big[0] = 0;
                After_Big[1] = 0;
                After_Big[2] = 0;
            }
        break;
        case Return_Line:
    #ifdef debug_switch
            printf("Return_Line");
    #endif
            resetWare(&smallPlaceWare);
            MyFSM.Unload_Count = 0;
            SMALL_PLACE_DATA.place = nil;
            if (Turn.Finish == false)
            {
                if (MyFSM.Cross_Dir == RIGHT)
                {
                    Turn_Angle(90);
                }
                else
                {
                    Turn_Angle(-90);
                }
                Car.Image_Flag = true;
            }
            else
            {
                if(No_Get_Line())
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    if(MyFSM.Cross_Dir == RIGHT)
                    {
                        Car.Speed_Z = 3;
                    }
                    else
                    {
                        Car.Speed_Z = -3;
                    }
                }
                else
                {
                    Turn.Finish = false;
                    MyFSM.Cross_Board_State = Finsh_Return;
                }
                // if(Bufcnt(true,2000))
                // {
                //     Car.Speed_X = 0;
                //     Car.Speed_Y = 0;
                //     Car.Speed_Z = 0;
                // }
                // Car.Speed_X = 0;
                // Car.Speed_Y = 5;
                // Car.Speed_Z = 0;
            }
            Dodge_Carmar();
            resetWare(&smallPlaceWare);
        break;
        case Ready_Ring:
            if(No_Get_Line())
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                if(Image_Flag.R_Find == false)
                {
                    Car.Speed_Z = 2;
                }
                else if(Image_Flag.L_Find == false)
                {
                    Car.Speed_Z = -2;
                }
            }
            else
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
                MyFSM.Cross_Board_State = Finsh_Return;
            }
        break;
        case Finsh_Return: 
            Car_run(5);
            if (Bufcnt(true,1000))
            {
                FINDBORDER_DATA.FINDBORDER_FLAG = false;
                Turn.Finish = false;
                MyFSM.Cross_Board_State = Find_Cross;
                MyFSM.CurState = Line_Patrol;
            }
        break;
        case No_Board_Return: // 元素中没有卡片直接走
            Dodge_Carmar();
            if (Turn.Finish == false)
            {
                if (MyFSM.Cross_Dir == RIGHT)
                {
                    Turn_Angle(-90);
                }
                else
                {
                    Turn_Angle(90);
                }
                Car.Image_Flag = true;
            }
            else
            {
                Car_run(6);
                if (Image_Flag.Cross_Fill == 2 || MyFSM.Cross_Flag_)
                {
                    MyFSM.Cross_Flag_ = true;
                    if (Bufcnt(MyFSM.Cross_Flag_, 2000))
                    {
                        FINDBORDER_DATA.FINDBORDER_FLAG = false;
                        MyFSM.Cross_Flag_ = false;
                        Turn.Finish = false;
                        MyFSM.Cross_Board_State = Find_Cross;
                        MyFSM.CurState = Line_Patrol;
                    }
                }
                else
                {
                    if (Bufcnt(MyFSM.Cross_Flag_, 10000))//超时退出
                    {
                        FINDBORDER_DATA.FINDBORDER_FLAG = false;
                        MyFSM.Cross_Flag_ = false;
                        Turn.Finish = false;
                        MyFSM.Cross_Board_State = Find_Cross;
                        MyFSM.CurState = Line_Patrol;
                    }
                }
            }
        break;
    }
}

/**@brief   圆环卡片状态机
-- @param   无
-- @author  庄文标
-- @date    2024/7/2
**/
static void Ring_BoardFsm()
{
    static float Curanglg = 0;
    static float Lastanglg = 0;
    static float Angle_Offest = 0;
    switch (MyFSM.Ring_Board_State)
    {
        case Find_Ring: // 找到出环口
    #ifdef debug_switch
            printf("Find_Ring\r\n");
    #endif
            Car.Speed = false;
            Car_run(4);

            if (RightRing.Ring_State == Leave_Ring_First)
            {
                MyFSM.Ring_Board_State = Ready_Ring;
            }
            else if (LeftRing.Ring_State == Leave_Ring_First)
            {
                MyFSM.Ring_Board_State = Ready_Ring;
            }

            if(Bufcnt(true,5000))//超时保护
            {
                MyFSM.Ring_Board_State = No_Board_Ring;
                if (MyFSM.Ring_Dir == RIGHT)
                {
                    RightRing.Ring_State = Ring_Front;
                    Image_Flag.Left_Ring = false;
                }
                else
                {
                    LeftRing.Ring_State = Ring_Front;
                    Image_Flag.Right_Ring = false;
                }
            }
        break;
        case Ready_Ring:
    #ifdef debug_switch
            printf("Ready_Ring\r\n");
    #endif
            Car_run(3);
            if ((FINDBORDER_DATA.dir == LEFT) || (FINDBORDER_DATA.dir == RIGHT))
            {
                MyFSM.Ring_Board_State = Find;
                Car.Image_Flag = false;
            }

            if (Bufcnt(true,1200))
            {
                MyFSM.Ring_Board_State = Find;
                Car.Image_Flag = false;
            }
            break;
        case Find: // 找卡片
    #ifdef debug_switch
            printf("Ring_Find\r\n");
    #endif
            FINETUNING_DATA.IS_BORDER_ALIVE = true;
            UnpackFlag.FINETUNING_DATA_FLAG = false;
            if (Turn.Finish == false)
            {
                if (MyFSM.Ring_Dir == RIGHT)
                {
                    Turn_Angle(77);
                }
                else
                {
                    Turn_Angle(-77);
                }
            }
            else
            {
                Turn.Finish = false;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Ring_Board_State = Wait_Data;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                Light_On;//开灯
            }
            break;
        case Wait_Data: // 等待串口数据回传
    #ifdef debug_switch
            printf("Ring_Wait_Data\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, START_FINETUNING); // 发送数据
            if (UnpackFlag.FINETUNING_DATA_FLAG)                 // 接收到串口数据
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (FINETUNING_DATA.IS_BORDER_ALIVE) // 如果有卡片
                {
                    if (Bufcnt(true, 100))
                    {
                        MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                        MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                        MyFSM.Ring_Board_State = Move; // 开始移动到卡片前面
                    }
                }
                // else
                // {
                //     Light_Off;//关灯
                //     if (MyFSM.Pick_Count > 6)
                //     {
                //         MyFSM.Ring_Board_State = Ready_Find_Place;
                //     }
                //     else
                //     {
                //         MyFSM.Ring_Board_State = No_Board_Return;
                //     }
                // }
            }
            break;
        case Move: // 移动到卡片前面
    #ifdef debug_switch
            printf("Ring_Move:%f,%d\r\n", FINETUNING_DATA.dx / 10.f, FINETUNING_DATA.FINETUNING_FINISH_FLAG);
    #endif
            CLASSIFY_DATA.place = nil;
            CLASSIFY_DATA.type = None;
            UART_SendByte(&_UART_FINDBORDER, START_FINETUNING); // 发送数据
            if (Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X, MyFSM.Target_Pos_Y); // 移动
            }
            else
            {
                Set_Beeptime(200);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                resetWare(&smallPlaceWare); // 重置仓库
                CLASSIFY_DATA.IS_CLASSIFY = true;
                MyFSM.Pick_Count = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                MyFSM.Ring_Board_State = Classify; // 识别
                UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            }
            break;
        case Classify:
    #ifdef debug_switch
            printf("Ring_Classify\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            if (UnpackFlag.FINETUNING_DATA_FLAG) // 接收到数据了
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (CLASSIFY_DATA.place != nil) // 识别到了分类
                {
                    Dodge_Board();
                    MyFSM.Ring_Board_State = Pick;// 捡起卡片
                    putCardIntoWare(&smallPlaceWare, CLASSIFY_DATA.place, &MyFSM.Depot_Pos); // 将卡片放入对应仓库
                    After_Big[MyFSM.Depot_Pos] += 1;
                    #ifdef Classify_debug
                    printf("ring=%s\r\n", PLACE_TABLE_STR[CLASSIFY_DATA.place]);
                    #endif
                    Servo_Flag.Depot_End = true;
                    CLASSIFY_DATA.place = nil;
                    CLASSIFY_DATA.type = None;
                    Set_Beepfreq(1);
                }
                else if (!CLASSIFY_DATA.IS_CLASSIFY) // 没有卡片
                {
                    MyFSM.Ring_Board_State = Ready_Find_Place; // 准备找第一个放置位置
                    MyFSM.Pick_Count = 0;
                    Light_Off;//关灯
                }
            }
            else // 没有接受到数据
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC); // 发送数据，接收分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
            break;
        case Pick: // 捡起卡片
    #ifdef debug_switch
                //  printf("Ring_Pick\r\n");
    #endif
            CLASSIFY_DATA.type = None;
            CLASSIFY_DATA.place = nil;
            if (Servo_Flag.Pick_End == false)
            {
                Pick_Card();
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                MyFSM.Pick_Count += 1;
                if (MyFSM.Pick_Count > 6) // 出现大于6次捡起卡片说明卡片飞了或者是见不到，直接去放置点
                {
                    MyFSM.Ring_Board_State = Ready_Find_Place;
                }
                else
                {
                    MyFSM.Ring_Board_State = Classify; // 继续识别
                }
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
            break;
        case Ready_Find_Place: // 准备找第一个放卡片的位置
    #ifdef debug_switch
            printf("Ring_Ready_Find_Place\r\n");
    #endif
            if (Turn.Finish == false)
            {
                Turn_Angle(180);
            }
            else
            {
                Turn.Finish = false;
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
                MyFSM.Ring_Board_State = Ring_First_Place; // 圆环第一个放置点
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                Light_On;//开灯
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            break;
        case Ring_First_Place: // 直接往前走
    #ifdef debug_switch
            printf("Ring_First_Place\r\n");
    #endif
            // MyFSM.Depot_Pos = 0;
            Dodge_Carmar();
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送获取放置区域信息
            if (UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (Bufcnt(true,450))
                {
                    MyFSM.Ring_Board_State = Move_Place; // 开始移动到放置区域前面
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                }
            }
            else
            {
                Car.Speed_X = -0.5f;
                Car.Speed_Y = 3;
                Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
            }
        break;
        case Move_Place:// 移动到放置区域前面
            Light_On;//开灯
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送获取放置区域信息
            if (Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X, MyFSM.Target_Pos_Y); // 移动
            }
            else
            {
                Set_Beepfreq(1);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Ring_Board_State = Classify_Place; // 识别放置区域
                Light_On;//开灯
                UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            }
        break;
        case Classify_Place: // 识别放置区域
    #ifdef debug_switch
        //  printf("RingClassify_Place\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            if (SMALL_PLACE_DATA.place != nil && SMALL_PLACE_DATA.IS_PLACE) // 识别到了分类
            {
                #ifdef Classify_debug
                printf("%s\r\n", PLACE_TABLE_STR[SMALL_PLACE_DATA.place]);
                #endif
                Light_Off;//开灯
                MyFSM.Unload_Count += 1;
                Dodge_Board();
                Servo_Flag.Depot_End = true;
                MyFSM.Depot_Pos = takeOutFromWare(&smallPlaceWare, SMALL_PLACE_DATA.place, &MyFSM.Small_Count, currCard);
                Set_Beepfreq(1);
                SMALL_PLACE_DATA.place = nil;
                SMALL_PLACE_DATA.IS_PLACE = false;
                if (MyFSM.Small_Count != 0)
                {
                    MyFSM.Ring_Board_State = Unload_Board;
                }
                else if ((MyFSM.Unload_Count == 1) && (MyFSM.Small_Count == 0)) // 没有卡片且是第一个卸货点
                {
                    MyFSM.Ring_Board_State = Ready_Find_Next_First;
                    // printf("111Ready_Find_Next_First:%s\r\n", PLACE_TABLE_STR[SMALL_PLACE_DATA.place]);
                }
                else if ((MyFSM.Small_Count == 0) && (MyFSM.Unload_Count < 5) && (MyFSM.Unload_Count > 1)) // 没有卡片且不是第一、五个卸货点
                {
                    MyFSM.Ring_Board_State = Ready_Find_Next;
                    // printf("111Ready_Find_Next:%s\r\n", PLACE_TABLE_STR[SMALL_PLACE_DATA.place]);
                }
                else if ((MyFSM.Unload_Count >= 5) && (MyFSM.Small_Count == 0)) // 没有卡片且是第五个卸货点
                {
                    MyFSM.Ring_Board_State = Return_Line;
                }
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE); // 发送数据，接收小类分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Unload_Board: // 卸货
    #ifdef debug_switch
            printf("Ring_Unload_Board\r\n");
    #endif
            SMALL_PLACE_DATA.place = nil;
            if (Servo_Flag.Depot_End)
            {
                if (!Servo_Flag.Put_Out)
                {
                    Take_Card_Out();
                }
                else
                {
                    Servo_Flag.Put_Out = false;
                    MyFSM.Small_Count -= 1;
                    After_Big[MyFSM.Depot_Pos] -= 1;
                    if ((MyFSM.Small_Count == 0) && (MyFSM.Unload_Count < 5) && (MyFSM.Unload_Count > 1)) // 卡片卸完且不是一、五个卸货点
                    {
                        MyFSM.Ring_Board_State = Ready_Find_Next;
                        MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    }
                    else if (MyFSM.Small_Count != 0) // 还有卡片
                    {
                        MyFSM.Ring_Board_State = Unload_Board;
                    }
                    else if ((MyFSM.Unload_Count >= 5) && (MyFSM.Small_Count == 0)) // 卡片卸完且是五个卸货点
                    {
                        MyFSM.Ring_Board_State = Return_Line;
                        MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    }
                    else if ((MyFSM.Unload_Count == 1) && (MyFSM.Small_Count == 0)) // 卡片卸完且是第一个卸货点
                    {
                        MyFSM.Ring_Board_State = Ready_Find_Next_First;
                        MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    }
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Ready_Find_Next_First: // 准备找第二个放置点
    #ifdef debug_switch
            printf("Ready_Find_Next_First\r\n");
    #endif
            // MyFSM.Depot_Pos = 0;
            SMALL_PLACE_DATA.place = nil;
            if (Navigation.Finish_Flag == false)
            {
                if (Bufcnt(true, 1500))
                {
                    Navigation.Finish_Flag = true;
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                }
                if(MyFSM.Ring_Dir == RIGHT)
                {
                    Car.Speed_X = -2;
                }
                else
                {
                    Car.Speed_X = 2; 
                }
                Car.Speed_Y = -5;
                Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
            }
            else
            {
                if (Turn.Finish == false)
                {
                    if (MyFSM.Ring_Dir == RIGHT)
                    {
                        Turn_Angle(-90);
                    }
                    else
                    {
                        Turn_Angle(90);
                    }
                }
                else
                {
                    Turn.Finish = false;
                    Navigation.Finish_Flag = false;
                    MyFSM.Ring_Board_State = Ring_First_Place;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    Light_On;//开灯
                }
            }
        break;
        case Find_Place: // 寻找放置位置
    #ifdef debug_switch
            printf("Find_Place\r\n");
    #endif
            SMALL_PLACE_DATA.place = nil;
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETSMALLPLACE); // 获取小类放置区域
            Dodge_Carmar();
            Curanglg = Gyro_YawAngle_Get();
            if (Lastanglg == 0)
            {
                Lastanglg = Curanglg;
            }
            Angle_Offest += (Curanglg - Lastanglg);
            if (MyFSM.Ring_Dir == RIGHT)
            {
                Car_run_X(-2.5);
            }
            else
            {
                Car_run_X(2.5);
            }
            Lastanglg = Curanglg;
            if (MyFSM.Unload_Count == 1 && FINDBORDER_DATA.FINDBIGPLACE_FLAG == true)
            {
                Light_On;//开灯
                Set_Beepfreq(1);
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Ring_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
            else if (FINDBORDER_DATA.FINDBIGPLACE_FLAG == true && (fabs(Angle_Offest) >= 30))
            {
                Light_On;//开灯 
                Set_Beepfreq(1);
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Ring_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
            break;
        case Wait_PlaceData: // 等待串口传回放置区域的信息，环内
    #ifdef debug_switch
            printf("Cross_Wait_PlaceData\r\n");
    #endif
            FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
            Dodge_Carmar();
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送获取放置区域信息
            if (UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if (Bufcnt(true, 100))
                {
                    MyFSM.Ring_Board_State = Move_Place; // 开始移动到放置区域前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                }
            }
            else
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 3;
                Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
            }
        break;
        case Ready_Find_Next: // 环内的准备找下一个放置点
    #ifdef debug_switch
            printf("Ready_Find_Next\r\n");
    #endif
            MyFSM.Depot_Pos = 0;
            SMALL_PLACE_DATA.place = nil;
            if (MyFSM.Unload_Count == 2)//第二个放置点
            {
                if (Navigation.Finish_Flag == false)
                {
                    if (MyFSM.Ring_Dir == RIGHT)
                    {
                        if (Bufcnt(true, 850))
                        {
                            Navigation.Finish_Flag = true;
                            Car.Speed_X = 0;
                            Car.Speed_Y = 0;
                            Car.Speed_Z = 0;
                        }
                        Car.Speed_X = -0.5;
                        Car.Speed_Y = -3.5;
                        Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
                    }
                    else if (MyFSM.Ring_Dir == LEFT)
                    {
                        if (Bufcnt(true, 850))
                        {
                            Navigation.Finish_Flag = true;
                            Car.Speed_X = 0;
                            Car.Speed_Y = 0;
                            Car.Speed_Z = 0;
                        }
                        Car.Speed_X = 0.5;
                        Car.Speed_Y = -3.5;
                        Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
                    }
                }
                else
                {
                    Navigation.Finish_Flag = false;
                    MyFSM.Ring_Board_State = Find_Place;
                }
            }
            else
            {
                if (Navigation.Finish_Flag == false)
                {
                    if (Bufcnt(true, 850))
                    {
                        Navigation.Finish_Flag = true;
                        Car.Speed_X = 0;
                        Car.Speed_Y = 0;
                        Car.Speed_Z = 0;
                    }
                    Car.Speed_X = 0;
                    Car.Speed_Y = -3;
                    Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
                }
                else
                {
                    Navigation.Finish_Flag = false;
                    MyFSM.Ring_Board_State = Find_Place;
                }
            }
        break;
        case Adjust_Line:
            Car_run_X(0);
            if(Bufcnt(true,500))
            {
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Ring_Board_State = Find_Place;
            }
        break;
        case Return_Line:
    #ifdef debug_switch
            printf("Return_Line\r\n");
    #endif
            MyFSM.Unload_Count = 0;
            SMALL_PLACE_DATA.place = nil;
            Dodge_Carmar();
            resetWare(&smallPlaceWare);
            if (Turn.Finish == false)
            {
                if (MyFSM.Ring_Dir == RIGHT)
                {
                    if (Bufcnt(true,1500))
                    {            
                        Turn.Finish = true;
                        Car.Speed_X = 0;
                        Car.Speed_Y = 0;
                        Car.Speed_Z = 0;
                    }     
                    Car.Speed_X = -3;
                    Car.Speed_Y = 1;
                    Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
                }
                else
                {
                    if (Bufcnt(true,1500))
                    {            
                        Turn.Finish = true;
                        Car.Speed_X = 0;
                        Car.Speed_Y = 0;
                        Car.Speed_Z = 0;
                    }     
                    Car.Speed_X = 3;
                    Car.Speed_Y = 1;
                    Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);          
                }
                Car.Image_Flag = true;
            }
            else
            {
                if(No_Get_Line())
                {
                    if(MyFSM.Ring_Dir == RIGHT)
                    {
                        Car.Speed_X = 0;
                        Car.Speed_Y = 0;
                        Car.Speed_Z = -2;
                    }
                    else
                    {
                        Car.Speed_X = 0;
                        Car.Speed_Y = 0;
                        Car.Speed_Z = 2;
                    }
                }
                else
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                    MyFSM.Ring_Board_State = Finsh_Return;
                    Turn.Finish = false; 
                }
            }
        break;
        case Finsh_Return:
            Car_run(5);
            if (Bufcnt(true, 500))
            {
                Turn.Finish = false;
                RightRing.Ring_State = Ring_Front;
                LeftRing.Ring_State = Ring_Front;
                MyFSM.Ring_Board_State = Find_Ring;
                MyFSM.CurState = Line_Patrol;
                Image_Flag.Right_Ring = false;
                Image_Flag.Left_Ring = false;
                // MyFSM.Ring_Flag = true;
                FINDBORDER_DATA.FINDBORDER_FLAG = false;
                MyFSM.Big_Count[0]+=After_Big[0];
                MyFSM.Big_Count[1]+=After_Big[1];
                MyFSM.Big_Count[2]+=After_Big[2];
                After_Big[0] = 0;
                After_Big[1] = 0;
                After_Big[2] = 0;
            }
        break;
        case No_Board_Return: // 元素里没有卡片直接走
            if (Turn.Finish == false)
            {
                if (MyFSM.Ring_Dir == RIGHT)
                {
                    Turn_Angle(-90);
                }
                else
                {
                    Turn_Angle(90);
                }
            }
            else
            {
                if (MyFSM.Ring_Dir == RIGHT)
                {
                    RightRing.Ring_State = Leave_Ring;
                    Car.Image_Flag = true;
                    MyFSM.Ring_Board_State = No_Board_Ring;
                }
                else if (MyFSM.Ring_Dir == LEFT)
                {
                    LeftRing.Ring_State = Leave_Ring;
                    Car.Image_Flag = true;
                    MyFSM.Ring_Board_State = No_Board_Ring;
                }
                Turn.Finish = false;
            }
        break;
        case No_Board_Ring:
            Car_run(5);
            // UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBORDER); // 继续获取道路旁卡片
            if (MyFSM.Ring_Dir == RIGHT)
            {
                if (Image_Flag.Right_Ring == false)
                {
                    MyFSM.Ring_Board_State = Find_Ring;
                    MyFSM.CurState = Line_Patrol;
                    Turn.Finish = false;
                }
            }
            else
            {
                if (Image_Flag.Left_Ring == false)
                {
                    MyFSM.Ring_Board_State = Find_Ring;
                    MyFSM.CurState = Line_Patrol;
                    Turn.Finish = false;
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
    static bool MOVE_Flag = false;
    switch (MyFSM.Unload_State)
    {
        case Find_Zebra:
    #ifdef debug_switch
            printf("Find_Zebra\r\n");
    #endif
            Car.Speed = false;
            if (Turn.Finish == false)
            {
                Turn_Angle(77);
            }
            else
            {
                if(Bufcnt(true,200))
                {
                    Turn.Finish = false;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    MyFSM.Unload_State = Find_First_Point_R;
                }
            }
        break;
        case Find_First_Point_R://找第一个点
            if(Bufcnt(true,Menu.Zebra_First_Dis))//原700
            {
                MyFSM.Unload_State = Wait_Big_Data;
                // MyFSM.Unload_State = Wait_Big_Data_Left;
            }
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE); // 获取大类放置区域
            Car_run_X(6);
            // Car.Speed_X = 6;
            // Car.Speed_Y = 0;
            // Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Wait_Big_Data_Left:
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Wait_Big_Data:
    #ifdef debug_switch
            printf("Wait_Big_Data\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBIGPLACE); // 获取大类放置区域
            if(Bufcnt(true,500))
            {
                MOVE_Flag = 1;
            }
            if(MOVE_Flag)
            {
                if (FINDBORDER_DATA.FINDBIGPLACE_FLAG)
                {
                    MyFSM.Big_Point+=1;
                    FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                    MyFSM.Big_Pos[0] = RIGHT;
                    Light_On;//开灯
                    MyFSM.Unload_State = Wait_Data;
                    MOVE_Flag = false;
                }
                else
                {
                    MOVE_Flag = false;
                    MyFSM.Big_Point+=1;
                    if(MyFSM.Big_Point!=3)
                    {
                        MyFSM.Unload_State = No_Board_Next;
                    }
                    else
                    {
                        MyFSM.Unload_State = Diff_Dir_Unload;
                    }
                }
            }

            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Diff_Dir_Unload://有不同方向的卸货点
            MyFSM.Big_Pos[2] = LEFT;
            if (Turn.Finish == false)
            {
                Turn_Angle(170);
            }
            else
            {
                if(Bufcnt(true,200))
                {
                    Turn.Finish = false;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                    MyFSM.Unload_State = Wait_Big_Data;
                }
            }
        break;
        case No_Board_Next:
            FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
            if(MyFSM.Big_Point < 3)
            {
                Car_run_X(6);
                if (Bufcnt(true, 900))
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                    MyFSM.Unload_State = Wait_Big_Data;
                }
            }
            else
            {
                Car_run_X(6);
                if (Bufcnt(true, 900))
                {
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                    MyFSM.Unload_State = Wait_Big_Data;
                }
            }
        break;
        case Wait_Data:
    #ifdef debug_switch
            printf("Wait_Data\r\n");
    #endif
            Dodge_Carmar();
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送对数字板微调信息
            if (UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if (FINETUNING_DATA.IS_BORDER_ALIVE)
                {
                    if (Bufcnt(true,300))
                    {
                        MyFSM.Unload_State = Move; // 开始移动到卡片前面
                        MyFSM.Target_Pos_X = FINETUNING_DATA.dx / 10.0f;
                        MyFSM.Target_Pos_Y = FINETUNING_DATA.dy / 10.0f;
                    }
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move: // 移动到卡片前面
    #ifdef debug_switch
            printf("%f,%d\r\n", FINETUNING_DATA.dx / 10.f, FINETUNING_DATA.FINETUNING_FINISH_FLAG);
    #endif
            BIG_PLACE_DATA.Big_Place = None;
            BIG_PLACE_DATA.place = nil;
            UART_SendByte(&_UART_FINDBORDER, UART_STARTFINETUNING_PLACE); // 发送对数字板微调信息    
            if (Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X, MyFSM.Target_Pos_Y); // 移动
            }
            else
            {
                Set_Beepfreq(1);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Unload_State = Classify; // 识别
                UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            }
        break;
        case Classify:
    #ifdef debug_switch
            printf("Classify\r\n");
    #endif
            UART_SendByte(&_UART_FINDBORDER, UART_SLEEP_FLAG); //发送数据
            if (BIG_PLACE_DATA.Big_Place != None) // 识别到了分类
            {
                Servo_Flag.Depot_End = true;
                Dodge_Board();
                MyFSM.Big_Board = BIG_PLACE_DATA.Big_Place; // 记录分类
                BIG_PLACE_DATA.Big_Place = None;
                BIG_PLACE_DATA.place = nil;
                Set_Beepfreq(MyFSM.Big_Board + 1);
                MyFSM.Depot_Pos = MyFSM.Big_Board;
                MyFSM.Big_Pos_Count += 1; // 记录去了几个大类卸货点
                MyFSM.Unload_Count = MyFSM.Big_Count[MyFSM.Big_Board];
                MyFSM.Big_Count[MyFSM.Big_Board] = 0;
                MyFSM.Big_Board = None;
                MyFSM.Unload_State = Unload_Board;
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_BIGPLACE); // 发送数据，接收大类分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Unload_Board:
    #ifdef debug_switch
            printf("Unload_Board:%d,%d\r\n", Servo_Flag.Depot_End, Servo_Flag.Open_Door);
    #endif
            BIG_PLACE_DATA.Big_Place = None;
            BIG_PLACE_DATA.place = nil;
            if (MyFSM.Unload_Count == 0)
            {
                if(MyFSM.Big_Point==3)
                {
                    MyFSM.Unload_State = Unload_Next;
                }
                else
                {
                    MyFSM.Unload_State = Ready_Find_Next;
                }
                Servo_Flag.Depot_End = false;
            }
            if (Servo_Flag.Depot_End)
            {
                if (!Servo_Flag.Put_Out)
                {
                    Take_Card_Out();
                }
                else
                {
                    MyFSM.Unload_Count -= 1;
                    Servo_Flag.Put_Out = false;
                }
            }
        break;
        case Ready_Find_Next:
            if (Bufcnt(true, 800))
            {
                MyFSM.Unload_State = Unload_Next;
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }   
            Car.Speed_X = 0;
            Car.Speed_Y = -3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Unload_Next:
    #ifdef debug_switch
            printf("Unload_Next\r\n");
    #endif
            BIG_PLACE_DATA.Big_Place = None;
            BIG_PLACE_DATA.place = nil;
            FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
            if ((MyFSM.Big_Pos_Count < 3) && ((MyFSM.Big_Count[0] != 0) || (MyFSM.Big_Count[1] != 0) || (MyFSM.Big_Count[2] != 0)))
            {
                if(MyFSM.Big_Point==3)
                {
                    MyFSM.Unload_State = Diff_Dir_Unload;
                    MyFSM.Big_Pos[2] = LEFT;
                }
                else if (Navigation.Finish_Flag == false)
                {
                    if(MyFSM.Big_Pos[MyFSM.Big_Pos_Count-1] == RIGHT)
                    {
                        Car_run_X(6);
                        if (Bufcnt(true, 900))
                        {
                            Car.Speed_X = 0;
                            Car.Speed_Y = 0;
                            Car.Speed_Z = 0;
                            Navigation.Finish_Flag = true;
                        }
                    }
                }
                else
                {
                    MyFSM.Unload_State = Wait_Big_Data;
                    Navigation.Finish_Flag = false;
                }
            }
            else // 三个卸货点都去了
            {
                if (Turn.Finish == false)
                {
                    if(MyFSM.Big_Pos[2]==RIGHT)
                    {
                        Turn_Angle(-80);
                    }
                    else
                    {
                        Turn_Angle(80);
                    }
                }
                else
                {
                    Turn.Finish = false;
                    Image_Flag.Zerba = false;
                    MyFSM.Unload_State = Return_Line;
                    Light_Off;//关灯
                    Car.Image_Flag = true;                    
                }
            }
        break;
        case Return_Line:
    #ifdef debug_switch
            printf("Return_Line = %d %d\r\n", Image_Flag.Zerba, MyFSM.Stop_Flag);
    #endif
            Dodge_Carmar();
            if (Bufcnt(Image_Flag.Zerba, 900))
            {
                MyFSM.Stop_Flag = true;
            }

            if (MyFSM.Stop_Flag)
            {
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                Car.Speed = true;
                Car_run(8);
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
        case Depart: // 发车状态
            Depart_Fsm();
        break;
        case Line_Patrol: // 巡线状态
            Line_PatrolFsm();
        break;
        case Line_Board: // 赛道散落卡片
            Line_BoardFsm();
        break;
        case Cross_Board: // 十字回环卡片
            Cross_BoardFsm();
        break;
        case Ring_Board: // 圆环卡片
            Ring_BoardFsm();
        break;
        case Unload: // 终点前卸货
            Unload_Fsm();
        break;
    }
}