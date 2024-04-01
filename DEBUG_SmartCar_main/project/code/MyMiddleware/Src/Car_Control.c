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
Pid_TypeDef Turn_PID;
Pid_TypeDef Gyroz_PID;
Pid_TypeDef Foward_PID;
Pid_TypeDef AngleControl_PID;
Pid_TypeDef Angle_PID;
//���ڻ�ȡ������ƫ��
float Direction_Err = 0.0;
float Forward_Speed = 5.0f;
bool Turn_Finsh = false;
float Start_Angle = 0;
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
    PIDInit(&Angle_PID,0.44f,0,2,1.5f,-1.5f);
    PIDInit(&Image_PID,3.8f,0,0.5f,2.5f,-2.5f);
    PIDInit(&BorderPlace_PID,2.1f,0,0,1.5f,-1.5f);
    PIDInit(&Foward_PID,2.1f,0,0,1.5f,-1.5f);
    PIDInit(&Turn_PID,1.3f,0,0.3f,5,-5);
    PIDInit(&Gyroz_PID,1.5f,0,0.5f,5,-5);
    PIDInit(&AngleControl_PID,0.44f,0,2,1.5f,-1.5f);
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

/**@brief   ת��ָ���Ƕ�
-- @param   float Target_Angle ��Ҫת���ĽǶ�
-- @auther  ׯ�ı�
-- @date    2024/3/31
**/
void Turn_Angle(float Target_Angle)
{   
    float Current_Angle = Gyro_YawAngle_Get();//��ȡ��ǰ�Ƕ�
    static float Angle_Last = 0;
    static float Turn_Offset;//ת���ĽǶ� 
    static float Target_Angle_Last = 0;
    float Offset_Erro = 0;
    float Yaw_Erro = 0;

    if(Angle_Last == 0)
    {
        Angle_Last = Current_Angle;
    }

    if(Target_Angle_Last!=Target_Angle)//Ŀ��ֵ�����ı�
    {
        Turn_Offset = 0;
        Turn_Finsh = false;
    }

    Turn_Offset += Current_Angle - Angle_Last;

    Offset_Erro = (Target_Angle - Turn_Offset)/50.0f;
    if(Target_Angle - Turn_Offset > 1.5f)
    {
        Yaw_Erro = 2 + GetPIDValue(&Turn_PID,Offset_Erro);
        Set_Car_Speed(0,0,GetPIDValue(&Gyroz_PID,Yaw_Erro - (-IMU_Data.gyro_z)));
        Turn_Finsh = false;
    }
    else if(Target_Angle - Turn_Offset < -1.5f)
    {
        Yaw_Erro = -2 + GetPIDValue(&Turn_PID,Offset_Erro);
        Set_Car_Speed(0,0,GetPIDValue(&Gyroz_PID,Yaw_Erro - (-IMU_Data.gyro_z)));
        Turn_Finsh = false;
    }
    else
    {
        Set_Car_Speed(0,0,0);
        Turn_Finsh = true;
    }

    Angle_Last = Current_Angle;
    Target_Angle_Last = Target_Angle;
}

/**@brief   �Ƕȿ���
-- @param   ��Ҫά�ֵĽǶ�
-- @auther  ׯ�ı�
-- @return  PIDֵ
-- @date    2024/3/31
**/
float Angle_Control(float Start_Angle)
{
    float Yaw_Err = 0.0f;
    Yaw_Err = GetPIDValue(&AngleControl_PID,Start_Angle - Gyro_YawAngle_Get());
    return GetPIDValue(&Gyroz_PID,Yaw_Err - (-IMU_Data.gyro_z));
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
    Image_Erro_ = GetPIDValue(&Image_PID,(70 - Image_Erro)*0.03f);
    // Angle_Erro_ = GetPIDValue(&Angle_PID,Gyro_YawAngle_Get() - Image_Erro);
    Set_Car_Speed(0,Forward_Speed,-Image_Erro_);
#ifdef debug_switch
    printf("line\r\n");
#endif
}

/**@brief   �ı�С���н�������ΪĿ��巽��
-- @param   ��
-- @auther  ������
-- @date    2023/12/23
**/
void Change_Direction(void)
{
    Turn_Angle(-90);
#ifdef debug_switch
    printf("FIND\r\n");
#endif
    // Set_Car_Speed(DirectionPidErr,0,GetPIDValue(&Angle_PID,Navigation.Cur_Angle));  
}

/**@brief   X���ƶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/3/23
**/
void X_Move_Action()
{
    float DirectionPidErr = 0.0f;
    DirectionPidErr = GetPIDValue(&BorderPlace_PID, FINETUNING_DATA.dx);
    // DirectionPidErr_y = GetPIDValue(&BorderPlace_PID, FINETUNING_DATA.dy);
    Set_Car_Speed(1.5,0,GetPIDValue(&Angle_PID,Start_Angle - Gyro_YawAngle_Get()));
#ifdef debug_switch
    printf("FIND_X\r\n");
#endif
}

/**@brief   Y���ƶ�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/3/16
**/
void Y_Move_Action()
{
    float DirectionPidErr_y = 0.0f;
    DirectionPidErr_y = GetPIDValue(&Foward_PID, FINETUNING_DATA.dy);
    Set_Car_Speed(0,1,GetPIDValue(&Angle_PID,Start_Angle - Gyro_YawAngle_Get()));
#ifdef debug_switch
    printf("x:%f\r\n",Navigation.Cur_Position_X);
#endif
}

