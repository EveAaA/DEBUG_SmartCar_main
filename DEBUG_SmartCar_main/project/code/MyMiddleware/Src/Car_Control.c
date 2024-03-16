/**
  ******************************************************************************
  * @file    Car_Control.c
  * @author  ׯ�ı�
  * @brief   С����̬����
  * @date    11/5/2023
    @verbatim
    ��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Car_Control.h"
#include "math.h"
/* Define\Declare ------------------------------------------------------------*/
Incremental_PID_TypeDef LMotor_F_Speed;
Incremental_PID_TypeDef RMotor_F_Speed;
Incremental_PID_TypeDef LMotor_B_Speed;
Incremental_PID_TypeDef RMotor_B_Speed;
Pid_TypeDef Image_PID;
Pid_TypeDef BorderPlace_PID;

//���ڻ�ȡ������ƫ��
float Direction_Err = 0.0;
#define debug_swithch 0 //�Ƿ����
/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ����PID������ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void All_PID_Init()
{
    //�ٶȻ�
    Incremental_PID_Init(&LMotor_F_Speed,0.6f,0.2f,0.4f,40,-40);
    Incremental_PID_Init(&RMotor_F_Speed,0.5f,0.25f,0.35f,40,-40);
    Incremental_PID_Init(&LMotor_B_Speed,0.7f,0.3f,0.5f,40,-40);
    Incremental_PID_Init(&RMotor_B_Speed,0.5f,0.35f,0.5f,40,-40);
    PIDInit(&Image_PID,5.0f,0,0,3.0f,-3.0f);
    PIDInit(&BorderPlace_PID,5,0,0,3,-3);
}


/**@brief   ���ó�����������ٶ�
-- @param   float Speed_X X���ٶȣ��Ⱥ����ٶ�
-- @param   float Speed_Y Y���ٶȣ���ǰ���ٶ�
-- @param   float Speed_Z Z���ٶȣ�����ת�ٶȣ�����Ϊ˳ʱ����ת
-- @auther  ׯ�ı�
-- @date    2023/11/5
**/
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z)
{
    float LF_Speed,LB_Speed,RF_Spped,RB_Speed;

//�˶�����
    LF_Speed = Speed_Y + Speed_X + Speed_Z;
    LB_Speed = Speed_Y - Speed_X + Speed_Z;
    RF_Spped = Speed_Y - Speed_X - Speed_Z;
    RB_Speed = Speed_Y + Speed_X - Speed_Z;

//�ٶȻ�
    Set_Motor_Speed(LMotor_F,Get_Incremental_PID_Value(&LMotor_F_Speed,LF_Speed-Encoer_Speed[0]));
    Set_Motor_Speed(LMotor_B,Get_Incremental_PID_Value(&LMotor_B_Speed,LB_Speed-Encoer_Speed[2]));
    Set_Motor_Speed(RMotor_F,Get_Incremental_PID_Value(&RMotor_F_Speed,RF_Spped-Encoer_Speed[1]));
    Set_Motor_Speed(RMotor_B,Get_Incremental_PID_Value(&RMotor_B_Speed,RB_Speed-Encoer_Speed[3]));
}

float Image_Erro_;
float Angle_Erro_;

/**@brief   Ѳ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/12/23
**/
void Car_run()
{
    Image_Erro_ = GetPIDValue(&Image_PID,(80 - Image_Erro)*0.05f);
    // Angle_Erro_ = GetPIDValue(&Angle_PID,Gyro_YawAngle_Get() - Image_Erro);
    Set_Car_Speed(0,4,-Image_Erro_);
#if debug_swithch
    printf("line\r\n");
#endif
}

extern Pid_TypeDef Angle_PID;
/**@brief   �ı�С���н�������ΪĿ��巽��
-- @param   ��
-- @auther  ������
-- @date    2023/12/23
**/
void Change_Direction(void)
{
    float DirectionPidErr = 0.0f;
    Navigation.Start_Flag = 1;//ʹ�ܹ��Ե���
    Navigation.Cur_Angle = Navigation.Start_Angle - Gyro_YawAngle_Get();//��ȡ����Ƕ�
    // Navigation.Cur_Position_X = sin(Navigation.Cur_Angle / 180 * 3.14159f) * Get_X_Distance();
    Navigation.Cur_Position_X = Get_X_Distance();//��ȡ��������ֵ
    Navigation.Cur_Position_Y = Get_Y_Distance();
    DirectionPidErr = GetPIDValue(&BorderPlace_PID, border.dx);
#if debug_swithch
    printf("FIND\r\n");
#endif
    Set_Car_Speed(DirectionPidErr,0,GetPIDValue(&Angle_PID,Navigation.Cur_Angle));  
}

/**@brief   ǰ���ҵ���Ƭ
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/3/23
**/
void Forward_Board()
{
    Navigation.Cur_Angle = Navigation.Start_Angle - Gyro_YawAngle_Get();//��ȡ����Ƕ�
    Set_Car_Speed(0,3,GetPIDValue(&Angle_PID,Navigation.Cur_Angle));
#if debug_swithch
    printf("FIND_X\r\n");
#endif
}

/**@brief   ��������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/3/16
**/
void Back_Autodrome()
{
    Navigation.Cur_Angle = Navigation.Start_Angle - Gyro_YawAngle_Get();//��ȡ����Ƕ�
    if(Navigation.Cur_Position_X > 0)
    {
        Set_Car_Speed(-5,0,GetPIDValue(&Angle_PID,Navigation.Cur_Angle));
    }
    else
    {
        Set_Car_Speed(5,0,GetPIDValue(&Angle_PID,Navigation.Cur_Angle));
    }
#if debug_swithch
    printf("x:%f\r\n",Navigation.Cur_Position_X);
#endif
}

