// /**
//   ******************************************************************************
//   * @file    User_FSM.c
//   * @author  庄文标
//   * @brief   主状态机
//   *
//     @verbatim
// 	无
//     @endverbatim
//   * @{
// **/

// /* Includes ------------------------------------------------------------------*/
// #include "User_FSM.h"

// /* Define\Declare ------------------------------------------------------------*/
// FSM_t *CURRENT_FSM;//当前运行状态机
// FSM_t Line_FSM;//巡线状态机
// FSM_t Left_Board_FSM;//卡片状态机_赛道旁
// FSM_t Right_Board_FSM;//卡片状态机_赛道旁

// /**
//  ******************************************************************************
//  *  @defgroup 内部调用
//  *  @brief
//  *
//  **/

// /**@brief   发车选择
// -- @auther  庄文标
// -- @date    2024/3/13
// **/
// bool Depart_Select()
// {
//     if(Start == 1)
//     {
//         return true;
//     }
//     return false;
// }

// /**@brief   找到卡片
// -- @auther  庄文标
// -- @date    2024/3/13
// **/
// bool Board_Find()
// {
//     if(FINDBORDER_DATA.FINDBORDER_FLAG == true)
//     {
//         Forward_Speed = 3;
//     }
//     else
//     {
//         Forward_Speed = 5;
//     }
//     if(FINDBORDER_DATA.dir == LEFT)
//     {
//         CURRENT_FSM = &Left_Board_FSM;//切换到卡片状态机
//     }
//     else if(FINDBORDER_DATA.dir == RIGHT)
//     {
//         CURRENT_FSM = &Right_Board_FSM;//切换到卡片状态机
//     }
//     return false;
// }

// /**@brief   找到卡片
// -- @auther  庄文标
// -- @date    2024/3/13
// **/
// bool TurnAction_Finish()
// {
//     if(Turn_Finsh)
//     {
//         Navigation.Start_Angle = Gyro_YawAngle_Get();//当前角度，用于下一步移动的角度闭环 
//         // printf("board_finishy\r\n");
//         return true;
//     }
//     return false;
// }

// /**@brief   移动结束
// -- @auther  庄文标
// -- @date    2024/4/4
// **/
// bool Move_Finish_()
// {
//     if(Navigation.Finish_Flag)
//     {
//         return true;
//     }
//     return false;
// }

// float Pick_Angle = 0;

// /**@brief   确认移动到位
// -- @auther  庄文标
// -- @date    2024/4/4
// **/
// bool Confirm_OK()
// {
//     if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)
//     {
//         Reset_Navigation();
//         Left_Board_FSM.CurState = Move;//如果移动没到位，就再进行一次调整
//         return false;
//     }
//     Pick_Angle = Gyro_YawAngle_Get();
//     return true;
// }

// /**@brief   确认移动到位右边
// -- @auther  庄文标
// -- @date    2024/4/4
// **/
// bool Confirm_OK_Right()
// {
//     if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)
//     {
//         Reset_Navigation();
//         Right_Board_FSM.CurState = Move;//如果移动没到位，就再进行一次调整
//         return false;
//     }
//     return true;
// }

// /**@brief   确认移动是否收到串口发的数据
// -- @auther  庄文标
// -- @date    2024/4/4
// **/
// bool Data_Receive()
// {
//     if(FINETUNING_DATA.dx!=0 && FINETUNING_DATA.dy!=0)
//     {
//         return true;
//     }
//     return false;
// }

// /**@brief   等待转向结束
// -- @auther  庄文标
// -- @date    2024/4/4
// **/
// bool Wait()
// {
//     if(Turn_Finsh)
//     {
//         CURRENT_FSM = &Line_FSM;//切换到卡片状态机
//         // printf("board_finishy\r\n");
//     }
//     return false;
// }

// bool Pick_Card_Finish()
// {
//     if(Servo_Flag.Put_Up)
//     {
//         Servo_Flag.Put_Up = false;
//         Servo_Flag.Put_Down = false;
//         return true;
//     }
//     return false;
// }

// /**
//  ******************************************************************************
//  *  @defgroup 状态表
//  *  @brief
//  *
//  **/

// FSMTable_t LineTable[] =
// {
//     //当前的状态		当前状态执行函数		跳转事件条件函数		下一个状态
// 	{Depart,           Motor_Disable,         Depart_Select,         LinePatrol    },
//     {LinePatrol,       Car_run,               Board_Find,            FindBoard     },
// };

// FSMTable_t LeftBoardTable[] =
// {
//     //当前的状态		当前状态执行函数		跳转事件条件函数		下一个状态
// 	{Find,              Change_Direction,     TurnAction_Finish,      Wait_Data    },
//     {Wait_Data,         Car_Stop_Wait_Data_L, Data_Receive,           Move         },
//     {Move,              Move_Action,          Move_Finish_,           Confirm      },
//     {Confirm,           Car_Stop_Wait_Data_L, Confirm_OK,             Pick         },
//     {Pick,              Pick_Card,            Pick_Card_Finish,       Return_Line  },
//     {Return_Line,       Return_Action,        Wait,                   Finish       },
// };

// FSMTable_t RightBoardTable[] =
// {
//     //当前的状态		当前状态执行函数		跳转事件条件函数		下一个状态
// 	{Find,              Change_Right,         TurnAction_Finish,      Wait_Data     },
//     {Wait_Data,         Car_Stop_Wait_Data_R, Data_Receive,           Move          },
//     {Move,              Move_Action,          Move_Finish_,           Confirm       },
//     {Confirm,           Car_Stop_Wait_Data_R, Confirm_OK_Right,       Pick          },
//     {Pick,              Pick_Card,            Pick_Card_Finish,       Return_Line   },
//     {Return_Line,       Return_Right,         Wait,                   Finish        },
// };

// /**
//  ******************************************************************************
//  *  @defgroup 外部调用
//  *  @brief
//  *
//  **/

// /**@brief   状态机初始化
// -- @auther  庄文标
// -- @date    2024/3/13
// **/
// void My_FSM_Init()
// {
//     CURRENT_FSM = &Line_FSM;
//     FSMInit(&Line_FSM,Depart,LineTable,0);
//     FSMInit(&Left_Board_FSM,Find,LeftBoardTable,1);
//     FSMInit(&Right_Board_FSM,Find,RightBoardTable,2);
// }
