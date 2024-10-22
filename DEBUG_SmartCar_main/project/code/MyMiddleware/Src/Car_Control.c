/**
  ******************************************************************************
  * @file    Car_Control.c
  * @author  庄文标
  * @brief   小车姿态控制
  * @date    11/5/2023
    @verbatim
    无
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Car_Control.h"
#include "Uart.h"
#include "math.h"
/* Define\Declare ------------------------------------------------------------*/
Incremental_PID_TypeDef LMotor_F_Speed;
Incremental_PID_TypeDef RMotor_F_Speed;
Incremental_PID_TypeDef LMotor_B_Speed;
Incremental_PID_TypeDef RMotor_B_Speed;
Pid_TypeDef Image_PID;
Pid_TypeDef ImageX_PID;
Pid_TypeDef ImageF_PID;
Pid_TypeDef BorderPlace_PID;
Pid_TypeDef Turn_PID;
Pid_TypeDef Gyroz_PID;
Pid_TypeDef Gyroz_Pid;
Pid_TypeDef Foward_PID;
Pid_TypeDef AngleControl_PID;
Pid_TypeDef Angle_PID;
Pid_TypeDef Line_GyroZ_PID;
Pid_TypeDef Line_Double_PID;
//串口获取的像素偏差
float Direction_Err = 0.0;
float Forward_Speed = 5.0f;
bool Turn_Finsh = false;
float Start_Angle = 0;
Car_Handle Car = 
{
    .Speed_X = 0,
    .Speed_Y = 0,
    .Speed_Z = 0,
    .Image_Flag = false,
    .Speed = true,
};
Turn_Handle Turn = 
{
    .Angle_Last = 0,
    .Current_Angle = 0,
    .Offset = 0,
    .Finish = false,
};

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   所有PID参数初始化
-- @param   无
-- @author  庄文标
-- @date    2023/11/5
**/
void All_PID_Init()
{
    //速度环
    Incremental_PID_Init(&LMotor_F_Speed,0.8f,0.1f,0.5f,40,-40);//0.6f,0.2f,0.4f,40,-40
    Incremental_PID_Init(&RMotor_F_Speed,0.75f,0.15f,0.29f,40,-40);//0.3f,0.25f,0.35f,40,-40
    Incremental_PID_Init(&LMotor_B_Speed,0.7f,0.23f,0.5f,40,-40);//0.7f,0.23f,0.5f,40,-40
    Incremental_PID_Init(&RMotor_B_Speed,0.76f,0.10f,0.36f,40,-40);//0.5f,0.22f,0.5f,40,-40
    PIDInit(&Angle_PID,0.44f,0,2,1.5f,-1.5f);
    // PIDInit(&Image_PID,3.8f,0,0.5f,2.5f,-2.5f);
    PIDInit(&Image_PID,4.5f,0,1.56f,6.5f,-6.5f);

    PIDInit(&ImageX_PID,0.060f,0, 0.0f,3.0f,-3.0f);
    PIDInit(&ImageF_PID,0.065f,0,0.3f,3.0f,-3.0f);
    PIDInit(&BorderPlace_PID,2.1f,0,0,1.5f,-1.5f);
    PIDInit(&Foward_PID,2.1f,0,0,1.5f,-1.5f);
    PIDInit(&Turn_PID,3.55f,0,0.6f,5,-5);   
    PIDInit(&Gyroz_PID,1.75f,0,0.25f,5,-5);
    PIDInit(&Gyroz_Pid,2.65f,0,0.25f,5,-5);
    PIDInit(&AngleControl_PID,0.18f,0,0.5,3.0f,-3.0f);
    
    PIDInit(&Line_GyroZ_PID,0,0,0.8f,3.0f,-3.0f);
    PIDInit(&Line_Double_PID,0.24f,0,0,3.0f,-3.0f);
}



/**@brief   设置车三个方向的速度
-- @param   float Speed_X X轴速度，既横向速度
-- @param   float Speed_Y Y轴速度，既前向速度
-- @param   float Speed_Z Z轴速度，既旋转速度，正的为顺时针旋转
-- @author  庄文标
-- @date    2023/11/5
**/
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z)
{
    float LF_Speed,LB_Speed,RF_Spped,RB_Speed;

//运动解算
    if(Car.Speed)
    {
        LF_Speed = Speed_Y + Speed_X + Speed_Z;
        LB_Speed = Speed_Y - Speed_X + (0.3f * Speed_Z);
        RF_Spped = Speed_Y - Speed_X - Speed_Z;
        RB_Speed = Speed_Y + Speed_X - (0.3f*Speed_Z);
    }
    else
    {
        LF_Speed = Speed_Y + Speed_X + Speed_Z;
        LB_Speed = Speed_Y - Speed_X + Speed_Z;
        RF_Spped = Speed_Y - Speed_X - Speed_Z;
        RB_Speed = Speed_Y + Speed_X - Speed_Z;
    }

//速度环
    Set_Motor_Speed(LMotor_F,Get_Incremental_PID_Value(&LMotor_F_Speed,LF_Speed-Encoer_Speed[0]));
    Set_Motor_Speed(LMotor_B,Get_Incremental_PID_Value(&LMotor_B_Speed,LB_Speed-Encoer_Speed[2]));
    Set_Motor_Speed(RMotor_F,Get_Incremental_PID_Value(&RMotor_F_Speed,RF_Spped-Encoer_Speed[1]));
    Set_Motor_Speed(RMotor_B,Get_Incremental_PID_Value(&RMotor_B_Speed,RB_Speed-Encoer_Speed[3]));
}

/**@brief   转动指定角度
-- @param   float Target_Angle 需要转动的角度
-- @author  庄文标
-- @date    2024/3/31
**/
void Turn_Angle(float Target_Angle)
{   
    Turn.Current_Angle = Gyro_YawAngle_Get();//获取当前角度
    float Offset_Erro = 0;
    float Yaw_Erro = 0;

    if(Turn.Angle_Last == 0)
    {
        Turn.Angle_Last = Turn.Current_Angle;
    }

    Turn.Offset += Turn.Current_Angle - Turn.Angle_Last;
    Turn.Angle_Last = Turn.Current_Angle;
    Offset_Erro = (Target_Angle - Turn.Offset)/50.0f;
    
    if(Target_Angle - Turn.Offset >= 2.5f)
    {
        Yaw_Erro = 4 + GetPIDValue(&Turn_PID,Offset_Erro);
        Car.Speed_X = 0;
        Car.Speed_Y = 0;
        Car.Speed_Z = GetPIDValue(&Gyroz_Pid,Yaw_Erro - IMU_Data.gyro_z);
        Turn.Finish = false;
    }
    else if(Target_Angle - Turn.Offset <= -2.5f)
    {
        Yaw_Erro = -4 + GetPIDValue(&Turn_PID,Offset_Erro);
        Car.Speed_X = 0;
        Car.Speed_Y = 0;
        Car.Speed_Z = GetPIDValue(&Gyroz_Pid,Yaw_Erro - IMU_Data.gyro_z);
        Turn.Finish = false;
    }
    else
    {
        Car.Speed_X = 0;
        Car.Speed_Y = 0;
        Car.Speed_Z = 0;
        Turn.Finish = true;
        Turn.Offset = 0;
        Turn.Angle_Last = 0;
    }
}


/**@brief   角度控制
-- @param   需要维持的角度
-- @author  庄文标
-- @return  PID值
-- @date    2024/3/31
**/
float Angle_Control(float Start_Angle)
{
    float Yaw_Err = 0.0f;
    static float Yaw_Filter = 0;
    Yaw_Filter = Gyro_YawAngle_Get();
    Yaw_Err = GetPIDValue(&AngleControl_PID,Start_Angle - Gyro_YawAngle_Get());
    
    // return GetPIDValue(&Gyroz_Pid,Yaw_Err - IMU_Data.gyro_z);
    return Yaw_Err;
}

/**@brief   根据赛道边线的角度控制
-- @author  庄文标
-- @return  PID值
-- @date    2024/6/1
**/
float Get_Image_Errox()
{
    float Image_ErroX_ = GetPIDValue(&ImageX_PID,(0 - VOLUMEUP_DATA.AngleErr));
    return  Image_ErroX_;//GetPIDValue(&Gyroz_Pid,Image_ErroX_ - IMU_Data.gyro_z);
}

/**@brief   巡线
-- @param   无
-- @author  庄文标
-- @date    2023/12/23
**/
void Car_run(float Speed)
{
    float Pid_Err = (74 - Image_Erro)*0.03f;
    float Image_Erro_ = GetPIDValue(&Image_PID,Pid_Err);
    Car.Speed_X = 0;
    Car.Speed_Y = Speed;// + GetPIDValue(&Line_GyroZ_PID,(0 - IMU_Data.gyro_z)) - GetPIDValue(&Line_Double_PID,fabs(Pid_Err)*Pid_Err)
    Car.Speed_Z = -Image_Erro_;
}

/**@brief   横向巡线
-- @param   无
-- @author  庄文标
-- @date    2024/5/3
**/
void Car_run_X(float Speed)
{
    UART_SendByte(&_UART_FINE_TUNING, UART_MOVEVOLUMEUP_FLAG);
    float Image_ErroF_ = GetPIDValue(&ImageF_PID, 0 - (VOLUMEUP_DATA.HeightErr));
    Car.Speed_X = Speed;
    Car.Speed_Y = Image_ErroF_;
    Car.Speed_Z = Get_Image_Errox();
}

bool Car_Put_Board()
{
    static uint8 testmode = 0;
    switch(testmode)
    {
        case 0:
            if(Bufcnt(true,300))
            {
                testmode = 1;
            }
            Car.Speed_X = -5;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
        break;
        case 1:
            if(Bufcnt(true,300))
            {
                testmode = 2;
            }
            Car.Speed_X = 5;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;                    
        break;
        case 2:
            Car.Speed_X = 0;
            Car.Speed_Y = 0;
            Car.Speed_Z = 0;
            testmode = 0;
            return 1;       
        break;
    }
    return 0;
}
