// /**
//   ******************************************************************************
//   * @file    User_FSM.c
//   * @author  ׯ�ı�
//   * @brief   ��״̬��
//   *
//     @verbatim
// 	��
//     @endverbatim
//   * @{
// **/

// /* Includes ------------------------------------------------------------------*/
// #include "User_FSM.h"

// /* Define\Declare ------------------------------------------------------------*/
// FSM_t *CURRENT_FSM;//��ǰ����״̬��
// FSM_t Line_FSM;//Ѳ��״̬��
// FSM_t Left_Board_FSM;//��Ƭ״̬��_������
// FSM_t Right_Board_FSM;//��Ƭ״̬��_������

// /**
//  ******************************************************************************
//  *  @defgroup �ڲ�����
//  *  @brief
//  *
//  **/

// /**@brief   ����ѡ��
// -- @auther  ׯ�ı�
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

// /**@brief   �ҵ���Ƭ
// -- @auther  ׯ�ı�
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
//         CURRENT_FSM = &Left_Board_FSM;//�л�����Ƭ״̬��
//     }
//     else if(FINDBORDER_DATA.dir == RIGHT)
//     {
//         CURRENT_FSM = &Right_Board_FSM;//�л�����Ƭ״̬��
//     }
//     return false;
// }

// /**@brief   �ҵ���Ƭ
// -- @auther  ׯ�ı�
// -- @date    2024/3/13
// **/
// bool TurnAction_Finish()
// {
//     if(Turn_Finsh)
//     {
//         Navigation.Start_Angle = Gyro_YawAngle_Get();//��ǰ�Ƕȣ�������һ���ƶ��ĽǶȱջ� 
//         // printf("board_finishy\r\n");
//         return true;
//     }
//     return false;
// }

// /**@brief   �ƶ�����
// -- @auther  ׯ�ı�
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

// /**@brief   ȷ���ƶ���λ
// -- @auther  ׯ�ı�
// -- @date    2024/4/4
// **/
// bool Confirm_OK()
// {
//     if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)
//     {
//         Reset_Navigation();
//         Left_Board_FSM.CurState = Move;//����ƶ�û��λ�����ٽ���һ�ε���
//         return false;
//     }
//     Pick_Angle = Gyro_YawAngle_Get();
//     return true;
// }

// /**@brief   ȷ���ƶ���λ�ұ�
// -- @auther  ׯ�ı�
// -- @date    2024/4/4
// **/
// bool Confirm_OK_Right()
// {
//     if(FINETUNING_DATA.FINETUNING_FINISH_FLAG == false)
//     {
//         Reset_Navigation();
//         Right_Board_FSM.CurState = Move;//����ƶ�û��λ�����ٽ���һ�ε���
//         return false;
//     }
//     return true;
// }

// /**@brief   ȷ���ƶ��Ƿ��յ����ڷ�������
// -- @auther  ׯ�ı�
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

// /**@brief   �ȴ�ת�����
// -- @auther  ׯ�ı�
// -- @date    2024/4/4
// **/
// bool Wait()
// {
//     if(Turn_Finsh)
//     {
//         CURRENT_FSM = &Line_FSM;//�л�����Ƭ״̬��
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
//  *  @defgroup ״̬��
//  *  @brief
//  *
//  **/

// FSMTable_t LineTable[] =
// {
//     //��ǰ��״̬		��ǰ״ִ̬�к���		��ת�¼���������		��һ��״̬
// 	{Depart,           Motor_Disable,         Depart_Select,         LinePatrol    },
//     {LinePatrol,       Car_run,               Board_Find,            FindBoard     },
// };

// FSMTable_t LeftBoardTable[] =
// {
//     //��ǰ��״̬		��ǰ״ִ̬�к���		��ת�¼���������		��һ��״̬
// 	{Find,              Change_Direction,     TurnAction_Finish,      Wait_Data    },
//     {Wait_Data,         Car_Stop_Wait_Data_L, Data_Receive,           Move         },
//     {Move,              Move_Action,          Move_Finish_,           Confirm      },
//     {Confirm,           Car_Stop_Wait_Data_L, Confirm_OK,             Pick         },
//     {Pick,              Pick_Card,            Pick_Card_Finish,       Return_Line  },
//     {Return_Line,       Return_Action,        Wait,                   Finish       },
// };

// FSMTable_t RightBoardTable[] =
// {
//     //��ǰ��״̬		��ǰ״ִ̬�к���		��ת�¼���������		��һ��״̬
// 	{Find,              Change_Right,         TurnAction_Finish,      Wait_Data     },
//     {Wait_Data,         Car_Stop_Wait_Data_R, Data_Receive,           Move          },
//     {Move,              Move_Action,          Move_Finish_,           Confirm       },
//     {Confirm,           Car_Stop_Wait_Data_R, Confirm_OK_Right,       Pick          },
//     {Pick,              Pick_Card,            Pick_Card_Finish,       Return_Line   },
//     {Return_Line,       Return_Right,         Wait,                   Finish        },
// };

// /**
//  ******************************************************************************
//  *  @defgroup �ⲿ����
//  *  @brief
//  *
//  **/

// /**@brief   ״̬����ʼ��
// -- @auther  ׯ�ı�
// -- @date    2024/3/13
// **/
// void My_FSM_Init()
// {
//     CURRENT_FSM = &Line_FSM;
//     FSMInit(&Line_FSM,Depart,LineTable,0);
//     FSMInit(&Left_Board_FSM,Find,LeftBoardTable,1);
//     FSMInit(&Right_Board_FSM,Find,RightBoardTable,2);
// }
