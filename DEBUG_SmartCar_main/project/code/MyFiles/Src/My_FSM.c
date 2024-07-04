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
    .Big_Pos[0] = RIGHT,
    .Big_Pos[1] = RIGHT,
    .Big_Pos[2] = RIGHT,
};

WareState_t smallPlaceWare = 
{
    .isSame = false,
    .currWareNum = 0,
    .notEmptyNum = 0,
    .isWareUsed = {false, false, false, false, false},
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
            Ware->list[i].cardNum++;   // 当前卡片仓库卡片数量加一
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
                Ware->isWareUsed[j] = true;   // 将该仓库置为有类别状态
                *Depot = j; // 将最新的下标赋值给Depot
                // printf("index: %d  :%d %d %d %d %d \r\n",j, Ware->isWareUsed[0], Ware->isWareUsed[1], Ware->isWareUsed[2], Ware->isWareUsed[3], Ware->isWareUsed[4]);
                break;
            }
        }
        Ware->currWareNum++;  // 当前已用仓库数量加一 对应仓库存储单元
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
Rotaryservo_Handle takeOutFromWare(WareState_t* ware , Place_t place,uint8* num)
{
    *num = 0;
    for (uint8 i = 0; i < 5; i++)
    {
        if (place == ware->list[i].CardName)
        {
            *num = ware->list[i].cardNum;
            return ware->list[i].WareIndex;
        }
    }
    return 0;
}

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

    if((FINDBORDER_DATA.dir == LEFT) || (FINDBORDER_DATA.dir == RIGHT))//散落卡片
    {
        MyFSM.Board_Dir = FINDBORDER_DATA.dir;
        Car.Image_Flag = false;
        MyFSM.CurState = Line_Board;//散落卡片
    }
    else if(Image_Flag.Cross_Fill == 2)//十字回环
    {
        Image_Flag.Cross_Fill = 0;
        MyFSM.CurState = Cross_Board;//十字回环状态机
        MyFSM.Cross_Dir = Image_Flag.Cross_Type;
        Set_Beepfreq(1);
    }
    else if((Image_Flag.Right_Ring) || (Image_Flag.Left_Ring))//圆环卡片
    {
        MyFSM.CurState = Ring_Board;//圆环状态机
        if(Image_Flag.Left_Ring)
        {
            MyFSM.Ring_Dir = LEFT;
        }
        else if(Image_Flag.Right_Ring)
        {
            MyFSM.Ring_Dir = RIGHT;
        }
        Set_Beepfreq(1);        
    }
    else if(Bufcnt(Image_Flag.Zerba,200))//终点前卸货
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
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
        break;
        case Wait_Data://等待串口数据回传
            #ifdef debug_switch
                // printf("Wait_Data\r\n");
                // printf("%f,%f\r\n",FINETUNING_DATA.dx,FINETUNING_DATA.dy);    
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING_BESIDE);//发送数据
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if(Bufcnt(true,500))
                {
                    if(FINETUNING_DATA.dy== -999 || FINETUNING_DATA.dx == -999)
                    {
                        MyFSM.Line_Board_State = Return_Line;//不是目标板返回赛道
                        FINETUNING_DATA.dx = 0;
                        FINETUNING_DATA.dy = 0;
                    }
                    else if(FINETUNING_DATA.IS_BORDER_ALIVE == false)
                    {
                        MyFSM.Line_Board_State = Return_Line;//不是目标板返回赛道
                        FINETUNING_DATA.dx = 0;
                        FINETUNING_DATA.dy = 0;                        
                    }
                    else
                    {
                        MyFSM.Line_Board_State = Move;//开始移动到卡片前面
                        MyFSM.Target_Pos_X = FINETUNING_DATA.dx/10.0f;
                        MyFSM.Target_Pos_Y = FINETUNING_DATA.dy/10.0f;
                    }
                }
            }
            // else
            // {
            //     if(MyFSM.Board_Dir == LEFT)
            //     {
            //         Car.Speed_X = 2.0f;//往右移动一点防止看不到卡片
            //     }
            //     else
            //     {
            //         Car.Speed_X = -2.0f;//往左移动一点防止看不到卡片
            //     }
            // }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                printf("%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING_BESIDE);//发送数据
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X,MyFSM.Target_Pos_Y);//移动
            }
            else
            {
                Set_Beeptime(200);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
                MyFSM.Line_Board_State = Classify;//识别
            }
        break;
        case Classify:
            #ifdef debug_switch
                printf("Classify\r\n");    
            #endif 
            if(CLASSIFY_DATA.type != None)//识别到了分类
            {
                // Dodge_Board();
                MyFSM.Line_Board_State = Pick;//捡起卡片
                MyFSM.Pick_Count = 9;
                MyFSM.Big_Board = CLASSIFY_DATA.type;//记录分类
                MyFSM.Big_Count[MyFSM.Big_Board]+=1;
                CLASSIFY_DATA.type = None;
                CLASSIFY_DATA.place = nil;
                Set_Beepfreq(MyFSM.Big_Board+1);
                MyFSM.Depot_Pos = MyFSM.Big_Board;
                MyFSM.Big_Board = None;
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//发送数据，接收分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Pick://捡起卡片
            #ifdef debug_switch
                printf("Pick\r\n");    
            #endif
            CLASSIFY_DATA.type = None;
            CLASSIFY_DATA.place = nil;
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
            Car.Speed_Z = 0;
        break;
        case Return_Line://返回赛道
            #ifdef debug_switch
                printf("Return\r\n");    
            #endif
            FINETUNING_DATA.dx = 0;
            FINETUNING_DATA.dy = 0;
            MyFSM.Pick_Count = 0;
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
                if(Bufcnt(true,600))
                {
                    Turn.Finish = false;
                    MyFSM.Line_Board_State = Find;
                    MyFSM.CurState = Line_Patrol;
                }
                Car_run(4);
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
            if(Turn.Finish == false)
            {
                if(MyFSM.Cross_Dir == RIGHT)
                {
                    Turn_Angle(90);
                }
                else
                {
                    Turn_Angle(-90);
                }
            }
            else
            {
                Turn.Finish = false;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Cross_Board_State = Wait_Data;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
        break;
        case Wait_Data://等待串口数据回传
            #ifdef debug_switch
                printf("cross\r\n");            
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if(FINETUNING_DATA.IS_BORDER_ALIVE)
                {
                    if(Bufcnt(true,500))
                    {
                        MyFSM.Target_Pos_X = FINETUNING_DATA.dx/10.0f;
                        MyFSM.Target_Pos_Y = FINETUNING_DATA.dy/10.0f;
                        MyFSM.Cross_Board_State = Move;//开始移动到卡片前面
                    }
                }
                else
                {
                    MyFSM.Cross_Board_State = No_Board_Return;
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                printf("Cross_Move:%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X,MyFSM.Target_Pos_Y);//移动
            }
            else
            {
                Set_Beeptime(200);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                resetWare(&smallPlaceWare); // 重置仓库
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                MyFSM.Cross_Board_State = Classify;//识别
                CLASSIFY_DATA.IS_CLASSIFY = true;
                MyFSM.Pick_Count = 0;
            }
        break;
        case Classify:
            #ifdef debug_switch
                // printf("Cross_Classify\r\n");    
            #endif 
            if(UnpackFlag.FINETUNING_DATA_FLAG)//接收到数据了
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if(CLASSIFY_DATA.place != nil)//识别到了分类
                {
                    printf("Cross_Classify\r\n");    
                    Dodge_Board();
                    MyFSM.Cross_Board_State = Pick;//捡起卡片
                    putCardIntoWare(&smallPlaceWare, CLASSIFY_DATA.place, &MyFSM.Depot_Pos); // 将卡片放入对应仓库
                    printf("now=%s\r\n",PLACE_TABLE_STR[CLASSIFY_DATA.place]);
                    CLASSIFY_DATA.place = nil;
                    CLASSIFY_DATA.type = None;
                    Set_Beepfreq(1);                  
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0;
                }
                else if(!CLASSIFY_DATA.IS_CLASSIFY)//没有卡片
                {
                    MyFSM.Cross_Board_State = Ready_Find_Place;
                    MyFSM.Pick_Count = 0;
                }
            }
            else//没有接受到数据
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//发送数据，接收分类数据
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Pick://捡起卡片
            #ifdef debug_switch
                // printf("Cross_Pick\r\n");    
            #endif
            CLASSIFY_DATA.type = None;
            CLASSIFY_DATA.place = nil;
            if(Servo_Flag.Pick_End == false)
            {
                Pick_Card();
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                MyFSM.Pick_Count+=1;
                MyFSM.Cross_Board_State = Classify;//继续识别
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Ready_Find_Place://准备找第一个放卡片的位置
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
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Find_Place://寻找放置位置
            #ifdef debug_switch
                // printf("Find_Place\r\n");    
            #endif 
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETSMALLPLACE);//获取小类放置区域
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
                Set_Beepfreq(1);
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Cross_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
            else if(FINDBORDER_DATA.FINDBIGPLACE_FLAG == true && (fabs(Angle_Offest) >= 45))
            {
                Set_Beepfreq(1);
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Cross_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
        break;
        case Wait_PlaceData://等待串口传回放置区域的信息
            #ifdef debug_switch
                // printf("Cross_Wait_PlaceData\r\n");    
            #endif 
            FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
            Dodge_Carmar();
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送获取放置区域信息
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,800))
                {
                    MyFSM.Cross_Board_State = Move_Place;//开始移动到放置区域前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx/10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy/10.0f;                
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move_Place://移动到卡片前面
            #ifdef debug_switch
                // printf("Move\r\n");    
                // printf("Cross_Place:%f,%f\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);
            #endif
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送获取放置区域信息 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X,MyFSM.Target_Pos_Y);//移动
            }
            else
            {
                Set_Beepfreq(1);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Cross_Board_State = Classify_Place;//识别放置区域
            }
        break;
        case Classify_Place:
            #ifdef debug_switch
                // printf("Classify_Place\r\n");    
            #endif 
            if(SMALL_PLACE_DATA.place != nil)//识别到了分类
            {
                printf("%s\r\n",PLACE_TABLE_STR[SMALL_PLACE_DATA.place]);
                MyFSM.Unload_Count+=1;
                Dodge_Board();
                Servo_Flag.Depot_End = true;
                MyFSM.Depot_Pos = takeOutFromWare(&smallPlaceWare,SMALL_PLACE_DATA.place,&MyFSM.Small_Count);
                Set_Beepfreq(1);
                SMALL_PLACE_DATA.place = nil;
                if(MyFSM.Small_Count!=0)
                {
                    MyFSM.Cross_Board_State = Unload_Board;
                }
                else if((MyFSM.Small_Count==0)&&(MyFSM.Unload_Count<5))//没有卡片且不是第五个卸货点
                {
                    MyFSM.Cross_Board_State = Ready_Find_Next;
                }
                else if((MyFSM.Unload_Count>=5)&&(MyFSM.Small_Count==0))//没有卡片且是第五个卸货点
                {
                    MyFSM.Cross_Board_State  = Return_Line;
                }
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE);//发送数据，接收小类分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Unload_Board:
            #ifdef debug_switch
                printf("Cross_Unload_Board:%d\r\n",MyFSM.Unload_Count);    
            #endif
            SMALL_PLACE_DATA.place = nil;
            if(Servo_Flag.Depot_End)
            {
                if(!Servo_Flag.Put_Out)
                {
                    Take_Card_Out();
                }
                else
                {
                    Servo_Flag.Put_Out = false;
                    MyFSM.Small_Count-=1;
                    if((MyFSM.Small_Count == 0) && (MyFSM.Unload_Count<5))//卡片卸完且不到五个卸货点
                    {
                        MyFSM.Cross_Board_State = Ready_Find_Next;
                    }
                    else if(MyFSM.Small_Count != 0)//还有卡片
                    {
                        MyFSM.Cross_Board_State = Unload_Board;
                    }
                    else if((MyFSM.Unload_Count>=5) && (MyFSM.Small_Count == 0))//卡片卸完且是五个卸货点
                    {
                        MyFSM.Cross_Board_State = Return_Line;
                    }
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0; 
        break;
        case Ready_Find_Next:
            SMALL_PLACE_DATA.place = nil;
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Y(0,-20);
            }
            else
            {
                Navigation.Finish_Flag =false;
                MyFSM.Cross_Board_State = Find_Place;
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Return_Line:
            #ifdef debug_switch
                printf("Return_Line");
            #endif
            resetWare(&smallPlaceWare);
            MyFSM.Unload_Count = 0;
            SMALL_PLACE_DATA.place = nil;
            if(Turn.Finish == false)
            {
                if(MyFSM.Cross_Dir==RIGHT)
                {
                    Turn_Angle(100);
                }
                else
                {
                    Turn_Angle(-100);
                }
                Car.Image_Flag = true;
            }
            else
            {
                Car_run(5);
                UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBORDER);//继续获取道路旁卡片
                if(Bufcnt(true,3000))
                {
                    FINDBORDER_DATA.FINDBORDER_FLAG = false;
                    Turn.Finish = false;
                    MyFSM.Cross_Board_State = Find_Cross;
                    MyFSM.CurState = Line_Patrol;
                }
            }                
            Dodge_Carmar();
            resetWare(&smallPlaceWare);
        break;   
        case No_Board_Return://元素中没有卡片直接走
            Dodge_Carmar();
            if(Turn.Finish == false)
            {
                if(MyFSM.Cross_Dir==RIGHT)
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
                Car_run(5);
                UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBORDER);//继续获取道路旁卡片
                if(Image_Flag.Cross_Fill==2)
                {
                    MyFSM.Cross_Flag_ = true;
                }
                if(Bufcnt(MyFSM.Cross_Flag_,1000))
                {
                    FINDBORDER_DATA.FINDBORDER_FLAG = false;
                    MyFSM.Cross_Flag_ = false;
                    Turn.Finish = false;
                    MyFSM.Cross_Board_State = Find_Cross;
                    MyFSM.CurState = Line_Patrol;
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
        case Find_Ring://找到出环口
            #ifdef debug_switch
                printf("Find_Ring\r\n");    
            #endif
            Car_run(4);
            if(RightRing.Ring_State==Leave_Ring_First)
            {
                MyFSM.Ring_Board_State = Ready_Ring;
                Car.Image_Flag = false;
            }
            else if(LeftRing.Ring_State==Leave_Ring_First)
            {
                MyFSM.Ring_Board_State = Ready_Ring;
                Car.Image_Flag = false;
            } 
        break;
        case Ready_Ring:
            Car_run(4);
            if(Bufcnt(true,300))
            {
                MyFSM.Ring_Board_State = Find;
            }
        break;
        case Find://找卡片
            #ifdef debug_switch
                printf("Ring_Find\r\n");    
            #endif
            if(Turn.Finish == false)
            {
                if(MyFSM.Ring_Dir == RIGHT)
                {
                    Turn_Angle(85);
                }
                else
                {
                    Turn_Angle(-85);
                }
            }
            else
            {
                Turn.Finish = false;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                MyFSM.Ring_Board_State = Wait_Data;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
        break;
        case Wait_Data://等待串口数据回传
            #ifdef debug_switch
                printf("Ring_Wait_Data\r\n");    
            #endif
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(UnpackFlag.FINETUNING_DATA_FLAG)//接收到串口数据
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if(FINETUNING_DATA.IS_BORDER_ALIVE)//如果有卡片
                {
                    if(Bufcnt(true,500))
                    {
                        MyFSM.Target_Pos_X = FINETUNING_DATA.dx/10.0f;
                        MyFSM.Target_Pos_Y = FINETUNING_DATA.dy/10.0f;
                        MyFSM.Ring_Board_State = Move;//开始移动到卡片前面
                    }
                    Car.Speed_X = 0;
                    Car.Speed_Y = 0;
                    Car.Speed_Z = 0; 
                }
                else
                {
                    MyFSM.Ring_Board_State = No_Board_Return; 
                }
            }
        break;
        case Move://移动到卡片前面
            #ifdef debug_switch
                printf("Ring_Move:%f,%d\r\n",FINETUNING_DATA.dx/10.f,FINETUNING_DATA.FINETUNING_FINISH_FLAG);
            #endif 
            UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);//发送数据
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X,MyFSM.Target_Pos_Y);//移动
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
                MyFSM.Ring_Board_State = Classify;//识别
            }
        break;
        case Classify:
            #ifdef debug_switch
                printf("Ring_Classify\r\n");    
            #endif 
            if(UnpackFlag.FINETUNING_DATA_FLAG)//接收到数据了
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if(CLASSIFY_DATA.place != nil)//识别到了分类
                {
                    Dodge_Board();
                    MyFSM.Ring_Board_State = Pick;//捡起卡片
                    putCardIntoWare(&smallPlaceWare, CLASSIFY_DATA.place, &MyFSM.Depot_Pos); // 将卡片放入对应仓库
                    printf("now=%s\r\n",PLACE_TABLE_STR[CLASSIFY_DATA.place]);
                    CLASSIFY_DATA.place = nil;
                    CLASSIFY_DATA.type = None;
                    Set_Beepfreq(1);                  
                }
                else if(!CLASSIFY_DATA.IS_CLASSIFY)//没有卡片
                {
                    MyFSM.Ring_Board_State = Ready_Find_Place;//准备找第一个放置位置
                    MyFSM.Pick_Count = 0;
                }
            }
            else//没有接受到数据
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_PIC);//发送数据，接收分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Pick://捡起卡片
            #ifdef debug_switch
                // printf("Ring_Pick\r\n");    
            #endif
            CLASSIFY_DATA.type = None;
            CLASSIFY_DATA.place = nil;
            if(Servo_Flag.Pick_End == false)
            {
                Pick_Card();
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
            else
            {
                MyFSM.Pick_Count+=1;
                MyFSM.Ring_Board_State = Classify;//继续识别
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Ready_Find_Place://准备找第一个放卡片的位置
            #ifdef debug_switch
                printf("Ring_Ready_Find_Place\r\n");    
            #endif
            if(Turn.Finish==false)
            {
                Turn_Angle(180);
            }
            else
            {
                Turn.Finish = false;
                Servo_Flag.Put_Up = false;
                Servo_Flag.Put_Down = false;
                Servo_Flag.Pick_End = false;
                MyFSM.Ring_Board_State = Ring_First_Place;//圆环第一个放置点
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                Car.Speed_X = 0;
                Car.Speed_Y = 0;
                Car.Speed_Z = 0;
            }
        break;
        case Ring_First_Place://直接往前走
            #ifdef debug_switch
                printf("Ring_First_Place\r\n");    
            #endif 
            Dodge_Carmar();
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送获取放置区域信息
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if(Bufcnt(true,500))
                {
                    MyFSM.Ring_Board_State = Move_Place;//开始移动到放置区域前面
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx/10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy/10.0f;                
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Find_Place://寻找放置位置
            #ifdef debug_switch
                printf("Find_Place\r\n");    
            #endif 
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETSMALLPLACE);//获取小类放置区域
            Dodge_Carmar();
            Curanglg = Gyro_YawAngle_Get();
            if (Lastanglg == 0)
            {
                Lastanglg = Curanglg;
            }
            Angle_Offest += Curanglg - Lastanglg;
            if(MyFSM.Ring_Dir == RIGHT)
            {
                Car_run_X(-2);
            }
            else
            {
                Car_run_X(2);
            }
            Lastanglg = Curanglg;
            if(MyFSM.Unload_Count==1&&FINDBORDER_DATA.FINDBIGPLACE_FLAG == true)
            {
                Set_Beepfreq(1);
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Ring_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
            else if(FINDBORDER_DATA.FINDBIGPLACE_FLAG == true && (fabs(Angle_Offest) >= 45))
            {
                Set_Beepfreq(1);
                FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
                MyFSM.Ring_Board_State = Wait_PlaceData;
                MyFSM.Static_Angle = Gyro_YawAngle_Get();
                Angle_Offest = 0;
                Lastanglg = 0;
                UnpackFlag.FINETUNING_DATA_FLAG = false;
            }
        break;
        case Wait_PlaceData://等待串口传回放置区域的信息，环内
            #ifdef debug_switch
                printf("Cross_Wait_PlaceData\r\n");    
            #endif 
            FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
            Dodge_Carmar();
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送获取放置区域信息
            if(UnpackFlag.FINETUNING_DATA_FLAG)
            {
                if(Bufcnt(true,300))
                {
                    MyFSM.Ring_Board_State = Move_Place;//开始移动到放置区域前面
                    UnpackFlag.FINETUNING_DATA_FLAG = false;
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx/10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy/10.0f;                
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 3;
            Car.Speed_Z = Angle_Control(MyFSM.Static_Angle);
        break;
        case Move_Place://移动到放置区域前面
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送获取放置区域信息 
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X,MyFSM.Target_Pos_Y);//移动
            }
            else
            {
                Set_Beepfreq(1);
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Ring_Board_State = Classify_Place;//识别放置区域
            }
        break;
        case Classify_Place://识别放置区域
            #ifdef debug_switch
                // printf("RingClassify_Place\r\n");    
            #endif 
            if(SMALL_PLACE_DATA.place != nil)//识别到了分类
            {
                printf("%s\r\n",PLACE_TABLE_STR[SMALL_PLACE_DATA.place]);
                MyFSM.Unload_Count+=1;
                Dodge_Board();
                Servo_Flag.Depot_End = true;
                MyFSM.Depot_Pos = takeOutFromWare(&smallPlaceWare,SMALL_PLACE_DATA.place,&MyFSM.Small_Count);
                Set_Beepfreq(1);
                SMALL_PLACE_DATA.place = nil;
                if(MyFSM.Small_Count!=0)
                {
                    MyFSM.Ring_Board_State = Unload_Board;
                }
                else if((MyFSM.Unload_Count==1) && (MyFSM.Small_Count == 0))//没有卡片且是第一个卸货点
                {
                    MyFSM.Ring_Board_State = Ready_Find_Next_First;
                }
                else if((MyFSM.Small_Count==0)&&(MyFSM.Unload_Count<5)&&(MyFSM.Unload_Count>1))//没有卡片且不是第一、五个卸货点
                {
                    MyFSM.Ring_Board_State = Ready_Find_Next;
                }
                else if((MyFSM.Unload_Count>=5)&&(MyFSM.Small_Count==0))//没有卡片且是第五个卸货点
                {
                    MyFSM.Ring_Board_State  = Return_Line;
                }
            }
            else
            {
                UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_SMALLPLACE);//发送数据，接收小类分类数据
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Unload_Board://卸货
            #ifdef debug_switch
                printf("Ring_Unload_Board\r\n");    
            #endif
            SMALL_PLACE_DATA.place = nil;
            if(Servo_Flag.Depot_End)
            {
                if(!Servo_Flag.Put_Out)
                {
                    Take_Card_Out();
                }
                else
                {
                    Servo_Flag.Put_Out = false;
                    MyFSM.Small_Count-=1;
                    if((MyFSM.Small_Count == 0) && (MyFSM.Unload_Count<5) && (MyFSM.Unload_Count>1))//卡片卸完且不是一、五个卸货点
                    {
                        MyFSM.Ring_Board_State = Ready_Find_Next;
                    }
                    else if(MyFSM.Small_Count != 0)//还有卡片
                    {
                        MyFSM.Ring_Board_State = Unload_Board;
                    }
                    else if((MyFSM.Unload_Count>=5) && (MyFSM.Small_Count == 0))//卡片卸完且是五个卸货点
                    {
                        MyFSM.Ring_Board_State = Return_Line;
                    }
                    else if((MyFSM.Unload_Count==1) && (MyFSM.Small_Count == 0))//卡片卸完且是第一个卸货点
                    {
                        MyFSM.Ring_Board_State = Ready_Find_Next_First;
                    }
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;             
        break;
        case Ready_Find_Next_First://准备找第二个放置点
            #ifdef debug_switch
                printf("Ready_Find_Next_First\r\n");    
            #endif       
            SMALL_PLACE_DATA.place = nil;
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Y(0,-34);
            }
            else
            {
                if(Turn.Finish == false)
                {
                    if(MyFSM.Ring_Dir == RIGHT)
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
                    Navigation.Finish_Flag =false;
                    MyFSM.Ring_Board_State = Ring_First_Place;
                    MyFSM.Static_Angle = Gyro_YawAngle_Get();
                }
            }     
        break;
        case Ready_Find_Next://环内的准备找下一个放置点
            #ifdef debug_switch
                printf("Ready_Find_Next\r\n");    
            #endif
            SMALL_PLACE_DATA.place = nil;
            if(MyFSM.Unload_Count == 2)
            {
                if(Navigation.Finish_Flag == false)
                {
                    if(MyFSM.Ring_Dir == RIGHT)
                    {
                        Navigation_Process_Y_Image(-15,-20);
                    }
                    else if(MyFSM.Ring_Dir == LEFT)
                    {
                        Navigation_Process_Y_Image(15,-20);
                    }
                }
                else
                {
                    Navigation.Finish_Flag =false;
                    MyFSM.Ring_Board_State = Find_Place;
                }                
            }
            else
            {
                if(Navigation.Finish_Flag == false)
                {
                    Navigation_Process_Y_Image(0,-20);
                }
                else
                {
                    Navigation.Finish_Flag =false;
                    MyFSM.Ring_Board_State = Find_Place;
                }
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
            if(Navigation.Finish_Flag == false)
            {
                if(MyFSM.Ring_Dir == RIGHT)
                {
                    Navigation_Process(-30,30);
                }
                else
                {
                    Navigation_Process(30,30);
                }
            }
            else
            {
                Car.Image_Flag = true;
                UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBORDER);//继续获取道路旁卡片
                FINDBORDER_DATA.FINDBORDER_FLAG = false;
                Navigation.Finish_Flag = false;
                RightRing.Ring_State = Ring_Front;
                LeftRing.Ring_State = Ring_Front;
                Image_Flag.Right_Ring = false;
                Image_Flag.Left_Ring = false;
                MyFSM.Ring_Board_State = Find_Ring;
                MyFSM.CurState = Line_Patrol;
            }
        break;
        case No_Board_Return://元素里没有卡片直接走
            if(Turn.Finish == false)
            {
                if(MyFSM.Ring_Dir == RIGHT)
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
                if(MyFSM.Ring_Dir == RIGHT)
                {
                    RightRing.Ring_State = Leave_Ring;
                    Car.Image_Flag = true;
                    MyFSM.Ring_Board_State = No_Board_Ring;
                }
                else if(MyFSM.Ring_Dir == LEFT)
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
            UART_SendByte(&_UART_FINDBORDER, UART_FINDBORDER_GETBORDER);//继续获取道路旁卡片
            if(MyFSM.Ring_Dir == RIGHT)
            {
                if(Image_Flag.Right_Ring==false)
                {
                    MyFSM.Ring_Board_State = Find_Ring;
                    MyFSM.CurState = Line_Patrol;
                    Turn.Finish = false;
                }
            }
            else
            {
                if(Image_Flag.Left_Ring==false)
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
                UnpackFlag.FINETUNING_DATA_FLAG = false;
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
                UnpackFlag.FINETUNING_DATA_FLAG = false;
                if(Bufcnt(true,300))
                {
                    MyFSM.Unload_State = Move;//开始移动到卡片前面
                    MyFSM.Target_Pos_X = FINETUNING_DATA.dx/10.0f;
                    MyFSM.Target_Pos_Y = FINETUNING_DATA.dy/10.0f;
                }
            }
            Car.Speed_X = 0;
            if(MyFSM.Big_Pos_Count == 0)
            {
                Car.Speed_Y = 3;
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
            UART_SendByte(&_UART_FINE_TUNING, UART_STARTFINETUNING_PLACE);//发送对数字板微调信息
            if(Navigation.Finish_Flag == false)
            {
                Navigation_Process_Image(MyFSM.Target_Pos_X,MyFSM.Target_Pos_Y);//移动
            }
            else
            {
                Navigation.Finish_Flag = false;
                FINETUNING_DATA.dx = 0;
                FINETUNING_DATA.dy = 0;
                MyFSM.Unload_State = Classify;//识别
            }
        break;
        case Classify:
            #ifdef debug_switch
                printf("Classify\r\n");    
            #endif 
            if(BIG_PLACE_DATA.Big_Place != None)//识别到了分类
            {
                Servo_Flag.Depot_End = true;
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
                if(Bufcnt(true,500))//3s发送一次
                {
                    UART_SendByte(&_UART_FINE_TUNING, UART_CLASSIFY_BIGPLACE);//发送数据，接收大类分类数据
                }
            }
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case Unload_Board:
            #ifdef debug_switch
                printf("Unload_Board\r\n");    
            #endif
            BIG_PLACE_DATA.Big_Place = None;
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
            Car.Speed_Z = 0; 
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
        case Ring_Board://圆环卡片
            Ring_BoardFsm();
        break;
        case Unload://终点前卸货
            Unload_Fsm();
        break;
    }
}

