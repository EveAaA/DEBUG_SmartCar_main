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
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
//����������
#define Encoder_LF                   (QTIMER1_ENCODER1)
#define Encoder_LF_A                 (QTIMER1_ENCODER1_CH1_C0)
#define Encoder_LF_B                 (QTIMER1_ENCODER1_CH2_C1)

#define Encoder_RF                   (QTIMER1_ENCODER2)
#define Encoder_RF_A                 (QTIMER1_ENCODER2_CH1_C2)
#define Encoder_RF_B                 (QTIMER1_ENCODER2_CH2_C24)

#define Encoder_LB                   (QTIMER3_ENCODER1)
#define Encoder_LB_A                 (QTIMER3_ENCODER1_CH1_B16)
#define Encoder_LB_B                 (QTIMER3_ENCODER1_CH2_B17)

#define Encoder_RB                   (QTIMER2_ENCODER1)
#define Encoder_RB_A                 (QTIMER2_ENCODER1_CH1_C3)
#define Encoder_RB_B                 (QTIMER2_ENCODER1_CH2_C25)

int16 Encoder_Pules_Buffer[4];//��������
int32 Distance_Buffer[4];//���뻺������
float Encoer_Speed[4];//�ٶ�����
float Encoder_Distance[4];//ʵ�ʾ�������

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   ��ȡ�������ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
static void Get_Encoder_Pulses()
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
static float Get_LF_Speed()
{
    static float Filter;
    float Speed = (Encoder_Pules_Buffer[0] / 600.0f)*100.0f;
    Filter = 0.25f*Speed + Filter*0.75f;
    return Filter;
}

/**@brief   ��ȡ��ǰ���ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
static float Get_RF_Speed()
{
    static float Filter;
    float Speed = (Encoder_Pules_Buffer[1] / 600.0f)*100.0f;
    Filter = 0.25f*Speed + Filter*0.75f;
    return -Filter;
}

/**@brief   ��ȡ������ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
static float Get_LB_Speed()
{
    static float Filter;
    float Speed = (Encoder_Pules_Buffer[2] / 600.0f)*100.0f;
    Filter = 0.25f*Speed + Filter*0.75f;
    return Filter;
}

/**@brief   ��ȡ�Һ����ٶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
static float Get_RB_Speed()
{
    static float Filter;
    float Speed = (Encoder_Pules_Buffer[3] / 600.0f)*100.0f;
    Filter = 0.25f*Speed + Filter*0.75f;
    return -Filter;
}

/**@brief   ��ȡ����
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/6
**/
static void Get_Distance()
{
    static float Cm_Per;
    Cm_Per = (6.3f*3.1415926f)/2048.0f;
    if(Navigation.Start_Flag)//�������Ե�������ʼ�������
    {
        for(int i = 0;i<=3;i++)
        {   
            Distance_Buffer[i] = Encoder_Pules_Buffer[i];
        }
        for(int i = 0;i<=3;i++)
        {   
            Encoder_Distance[i] += Distance_Buffer[i]*Cm_Per;
        }
    }
    else
    {
        for(int i = 0;i<=3;i++)//���
        {   
            Distance_Buffer[i] = 0;
            Encoder_Distance[i] = 0;
        }
    }

}

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

/**@brief   ����������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/7
**/
void Encoder_Process()
{
    Get_Encoder_Pulses();//��ȡ����������ֵ
    Get_Distance();//�������
    Encoer_Speed[0] = Get_LF_Speed();
    Encoer_Speed[1] = Get_RF_Speed();
    Encoer_Speed[2] = Get_LB_Speed();
    Encoer_Speed[3] = Get_RB_Speed();
    encoder_clear_count(Encoder_LF);// ��ձ���������
    encoder_clear_count(Encoder_RF);  
    encoder_clear_count(Encoder_LB);  
    encoder_clear_count(Encoder_RB);  
}

/**@brief   ������ƫ���ǻ�ȡ
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/7
**/
float Encoder_YawAngle_Get()
{
    float L_Distance = Encoder_Distance[0] + Encoder_Distance[2];
    float R_Distance = -Encoder_Distance[1] + (-Encoder_Distance[3]);
    return (((L_Distance-R_Distance)/2.0f)/60)*90;
}

/**@brief   X������ȡ
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/7
**/
float Get_X_Distance()
{
    return (Encoder_Distance[0] + (-Encoder_Distance[3]) - Encoder_Distance[2] - (-Encoder_Distance[1]))/4.0f;
}

/**@brief   Y������ȡ
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/7
**/
float Get_Y_Distance()
{
    return (Encoder_Distance[0] + (-Encoder_Distance[3]) + Encoder_Distance[2] + (-Encoder_Distance[1]))/4.0;
}