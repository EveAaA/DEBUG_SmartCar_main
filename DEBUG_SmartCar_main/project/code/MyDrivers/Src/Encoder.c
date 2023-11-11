/**
  ******************************************************************************
  * @file    Encoder.c
  * @author  ׯ�ı�
  * @brief   ����������
  * @date    11/04/2023
    @verbatim
    ���������ݴ���
    @endverbatim
  * @{
**/


/* Includes ------------------------------------------------------------------*/
#include "Encoder.h"


/* Define\Declare ------------------------------------------------------------*/
//����������
#define Encoder_LF                   (QTIMER2_ENCODER1)
#define Encoder_LF_A                 (QTIMER2_ENCODER1_CH1_C3)
#define Encoder_LF_B                 (QTIMER2_ENCODER1_CH2_C25)

#define Encoder_RF                   (QTIMER1_ENCODER2)
#define Encoder_RF_A                 (QTIMER1_ENCODER2_CH1_C2)
#define Encoder_RF_B                 (QTIMER1_ENCODER2_CH2_C24)

#define Encoder_LB                   (QTIMER3_ENCODER2)
#define Encoder_LB_A                 (QTIMER3_ENCODER2_CH1_B18)
#define Encoder_LB_B                 (QTIMER3_ENCODER2_CH2_B19)

#define Encoder_RB                   (QTIMER1_ENCODER1)
#define Encoder_RB_A                 (QTIMER1_ENCODER1_CH1_C0)
#define Encoder_RB_B                 (QTIMER1_ENCODER1_CH2_C1)

int16 Encoder_Pules_Buffer[4];

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/


/**@brief   ���б�������ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
void All_Encoder_Init()
{
    encoder_quad_init(Encoder_LF, Encoder_LF_A, Encoder_LF_B);                     // ��ʼ��������ģ�������� �������������ģʽ
    encoder_quad_init(Encoder_RF, Encoder_RF_A, Encoder_RF_B);                     // ��ʼ��������ģ�������� �������������ģʽ
    encoder_quad_init(Encoder_LB, Encoder_LB_A, Encoder_LB_B);                     // ��ʼ��������ģ�������� �������������ģʽ
    encoder_quad_init(Encoder_RB, Encoder_RB_A, Encoder_RB_B);                     // ��ʼ��������ģ�������� �������������ģʽ
}

/**@brief   ��ȡ�������ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
void Get_Encoder_Speed()
{
    Encoder_Pules_Buffer[0] = encoder_get_count(Encoder_LF);// ��ȡ����������
    encoder_clear_count(Encoder_LF);                        // ��ձ���������
    Encoder_Pules_Buffer[1] = encoder_get_count(Encoder_RF);
    encoder_clear_count(Encoder_RF);
    Encoder_Pules_Buffer[2] = encoder_get_count(Encoder_LB);
    encoder_clear_count(Encoder_LB);
    Encoder_Pules_Buffer[3] = encoder_get_count(Encoder_RB);
    encoder_clear_count(Encoder_RB);
}

/**@brief   ��ȡ��ǰ���ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
double Filter;
double Get_LF_Speed()
{
    double New_Spped = (Encoder_Pules_Buffer[0] / 3000.0)*100.0;
    Filter = 0.25*New_Spped + Filter*0.75;
    return Filter;
}

/**@brief   ��ȡ��ǰ���ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
double Get_RF_Speed()
{
    double Speed = 0;
    Speed = (Encoder_Pules_Buffer[1] / 3000.0)*100.0;
    return -Speed;
}

/**@brief   ��ȡ������ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
double Get_LB_Speed()
{
    double Speed = 0;
    Speed = (Encoder_Pules_Buffer[2] / 3000.0)*100.0;
    return Speed;
}

/**@brief   ��ȡ�Һ����ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
double Get_RB_Speed()
{
    double Speed = 0;
    Speed = (Encoder_Pules_Buffer[3] / 3000.0)*100.0;
    return -Speed;
}