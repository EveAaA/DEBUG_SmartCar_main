/**
  ******************************************************************************
  * @file    MatrixKeyBoard.c
  * @author  ��������־Զ��������
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
FSM_t Board_FSM;//��Ƭ״̬��_������


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
    if(fabs(border.dx) > 5 && border.dx != 127)
    {
        CURRENT_FSM = &Board_FSM;//�л�����Ƭ״̬��
        Enable_Navigation();//ʹ�ܹߵ�����¼����
        // printf("board_find");
    }
    return false;
}

/**@brief   �ҵ���Ƭ
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
bool Board_Find_Finish()
{
    if(fabs(border.dx) < 5)
    {
        // printf("board_finishy\r\n");
        return true;
    }
    return false;
}

/**@brief   ������������
-- @auther  ׯ�ı�
-- @date    2024/3/13
**/
bool Back_Autodrome_Finish()
{
    Navigation.Cur_Position_X = Get_X_Distance();//��ȡ��������ֵ
    if((fabs(0 - Navigation.Cur_Position_X) < 1.0f && fabs(Navigation.Cur_Angle < 1.0f)))
    {
        Navigation.Cur_Position_X = 0;//��¼ֵ����
        Navigation.Cur_Position_Y = 0;
        Navigation.Start_Angle = 0;
        Navigation.Start_Flag = 0;//�رչ��Ե���
#ifdef debug_switch
        printf("back_finish\n\r");
#endif
        CURRENT_FSM = &Line_FSM;//�л���Ѳ��״̬��
        return false;
    }
    return false;
}

bool Foward_finish()
{
    if(border.dx == 127)
    {
        return true;
    }
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

FSMTable_t BoardTable[] =
{
    //��ǰ��״̬		��ǰ״ִ̬�к���		��ת�¼���������		��һ��״̬
	{Find,              Change_Direction,     Board_Find_Finish,      Forward           },
    {Forward,           Forward_Board,        Foward_finish,          BackAutodrome     },
    {BackAutodrome,     Back_Autodrome,       Back_Autodrome_Finish,  finish            },
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
    FSMInit(&Board_FSM,Find,BoardTable,1);
}
