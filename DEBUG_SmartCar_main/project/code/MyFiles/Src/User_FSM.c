/**
  ******************************************************************************
  * @file    User_FSM.c
  * @author  ׯ�ı�
  * @brief   ��״̬��
  *
    @verbatim
	��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "User_FSM.h"

/* Define\Declare ------------------------------------------------------------*/
FSM_t *CURRENT_FSM;//��ǰ����״̬��
FSM_t Line_FSM;//Ѳ��״̬��
FSM_t Left_Board_FSM;//��Ƭ״̬��_������
FSM_t Right_Board_FSM;//��Ƭ״̬��_������


/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
 **/

/**@brief   ����ѡ��
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
bool Depart_Select()
{
    if(Start == 1)
    {
        return true;
    }
    return false;
}

/**@brief   �ҵ���Ƭ
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
bool Board_Find()
{
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
        CURRENT_FSM = &Left_Board_FSM;//�л�����Ƭ״̬��
    }
    return false;
}

/**@brief   �ҵ���Ƭ
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
bool TurnAction_Finish()
{
    if(Turn_Finsh)
    {
        Start_Angle = Gyro_YawAngle_Get();
        // printf("board_finishy\r\n");
        return true;
    }
    return false;
}

/**@brief   ƽ�ƽ���
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
bool Xmove_finish()
{
    if(fabs(FINETUNING_DATA.dx) <= 8)
    {
        return true;
    }
    return false;
}

/**@brief   ǰ������
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
bool Ymove_finish()
{
    if(fabs(FINETUNING_DATA.dy) <= 8)
    {
        return true;
    }
#ifdef debug_switch
        printf("back_finish\n\r");
#endif
        return false;
}

bool Wait()
{
    return false;
}

/**
 ******************************************************************************
 *  @defgroup ״̬��
 *  @brief
 *
 **/

FSMTable_t LineTable[] =
{
    //��ǰ��״̬		��ǰ״ִ̬�к���		��ת�¼���������		��һ��״̬
	{Depart,           Motor_Disable,         Depart_Select,         LinePatrol    },
    {LinePatrol,       Car_run,               Board_Find,            FindBoard     },
};

FSMTable_t LeftBoardTable[] =
{
    //��ǰ��״̬		��ǰ״ִ̬�к���		��ת�¼���������		��һ��״̬
	{Find,              Change_Direction,     TurnAction_Finish,      X_Move       },
    {X_Move,            X_Move_Action,        Xmove_finish,           Y_Move       },
    {Y_Move,            Y_Move_Action,        Ymove_finish,           finish       },
    {finish,            Motor_Disable,        Wait,                   fuck         },
};


/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
 **/

/**@brief   ״̬����ʼ��
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
void My_FSM_Init()
{
    CURRENT_FSM = &Line_FSM;
    FSMInit(&Line_FSM,Depart,LineTable,0);
    FSMInit(&Left_Board_FSM,Find,LeftBoardTable,1);
}
