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
    if(fabs(border.dx) > 5)
    {
        CURRENT_FSM = &Board_FSM;//�л�����Ƭ״̬��
        Enable_Navigation();//ʹ�ܹߵ�����¼����
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
    if((Navigation.Target_Position_X - Navigation.Cur_Position_X) < 1.0f && (Navigation.Target_Position_Y - Navigation.Cur_Position_Y) < 1.0f)
    {
        Navigation.Cur_Position_X = 0;//��¼ֵ����
        Navigation.Cur_Position_Y = 0;
        Navigation.Start_Angle = 0;
        Navigation.Start_Flag = 0;//�رչ��Ե���
        CURRENT_FSM = &Line_FSM;//�л���Ѳ��״̬��
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
	{FindBoard,       Change_Direction,      Board_Find_Finish,     BackAutodrome },
    {BackAutodrome,   Back_Autodrome,        Back_Autodrome_Finish, LinePatrol     },
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
    FSMInit(&Line_FSM,Depart,LineTable);
    FSMInit(&Board_FSM,FindBoard,BoardTable);
}
