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
#define Encoder_LF                   (QTIMER1_ENCODER2)
#define Encoder_LF_A                 (QTIMER1_ENCODER2_CH1_C2)
#define Encoder_LF_B                 (QTIMER1_ENCODER2_CH2_C24)

#define Encoder_RF                   (QTIMER1_ENCODER1)
#define Encoder_RF_A                 (QTIMER1_ENCODER1_CH1_C0)
#define Encoder_RF_B                 (QTIMER1_ENCODER1_CH2_C1)

#define Encoder_LB                   (QTIMER3_ENCODER2)
#define Encoder_LB_A                 (QTIMER3_ENCODER2_CH1_B18)
#define Encoder_LB_B                 (QTIMER3_ENCODER2_CH2_B19)

#define Encoder_RB                   (QTIMER2_ENCODER1)
#define Encoder_RB_A                 (QTIMER2_ENCODER1_CH1_C3)
#define Encoder_RB_B                 (QTIMER2_ENCODER1_CH2_C25)

int16 Encoder_Pules_Buffer[4];
int32 Distant_Buffer[4];
double Encoer_Speed[4];
double Encoder_Distant[4];
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
void Get_Encoder_Pulses()
{
    Encoder_Pules_Buffer[0] = encoder_get_count(Encoder_LF);// ��ȡ����������                 
    Encoder_Pules_Buffer[1] = encoder_get_count(Encoder_RF);
    Encoder_Pules_Buffer[2] = encoder_get_count(Encoder_LB);
    Encoder_Pules_Buffer[3] = encoder_get_count(Encoder_RB);
}

/**@brief   ��ȡ��ǰ���ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/

double Get_LF_Speed()
{
    static double Filter;
    double Speed = (Encoder_Pules_Buffer[0] / 600.0)*100.0;
    Filter = 0.25*Speed + Filter*0.75;
    return Filter;
}

/**@brief   ��ȡ��ǰ���ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
double Get_RF_Speed()
{
    static double Filter;
    double Speed = (Encoder_Pules_Buffer[1] / 600.0)*100.0;
    Filter = 0.25*Speed + Filter*0.75;
    return -Filter;
}

/**@brief   ��ȡ������ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
double Get_LB_Speed()
{
    static double Filter;
    double Speed = (Encoder_Pules_Buffer[2] / 600.0)*100.0;
    Filter = 0.25*Speed + Filter*0.75;
    return Filter;
}

/**@brief   ��ȡ�Һ����ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
double Get_RB_Speed()
{
    static double Filter;
    double Speed = (Encoder_Pules_Buffer[3] / 600.0)*100.0;
    Filter = 0.25*Speed + Filter*0.75;
    return -Filter;
}

/**@brief   ��ȡ����
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/6
**/
void Get_Distant()
{
    static double Cm_Per;
    Cm_Per = (6.3*3.1415926)/2300.0;
    for(int i = 0;i<=3;i++)
    {   
        Distant_Buffer[i] = Encoder_Pules_Buffer[i];
    }
    for(int i = 0;i<=3;i++)
    {   
        Encoder_Distant[i] += Distant_Buffer[i]*Cm_Per;
    }
}

/**@brief   ����������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/7
**/
void Encoder_Process()
{
    Get_Encoder_Pulses();//��ȡ����������ֵ
    Get_Distant();//�������
    Encoer_Speed[0] = Get_LF_Speed();
    Encoer_Speed[1] = Get_RF_Speed();
    Encoer_Speed[2] = Get_LB_Speed();
    Encoer_Speed[3] = Get_RB_Speed();
    encoder_clear_count(Encoder_LF);// ��ձ���������
    encoder_clear_count(Encoder_RF);  
    encoder_clear_count(Encoder_LB);  
    encoder_clear_count(Encoder_RB);  
}

double Encoder_YawAngle_Get()
{
    double L_Distant = Encoder_Distant[0] + Encoder_Distant[2];
    double R_Distant = -Encoder_Distant[1] + (-Encoder_Distant[3]);
    return (((L_Distant-R_Distant)/2.0)/60)*90;
    // return (L_Distant-R_Distant)/2.0;
}