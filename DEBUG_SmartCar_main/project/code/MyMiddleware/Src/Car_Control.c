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
Pid_TypeDef Gyroz_Pid;
Pid_TypeDef Foward_PID;
Pid_TypeDef AngleControl_PID;
Pid_TypeDef Angle_PID;
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
-- @auther  庄文标
-- @date    2023/11/5
**/
void All_PID_Init()
{
    //速度环
    Incremental_PID_Init(&LMotor_F_Speed,0.6f,0.2f,0.4f,40,-40);
    Incremental_PID_Init(&RMotor_F_Speed,0.5f,0.25f,0.35f,40,-40);
    Incremental_PID_Init(&LMotor_B_Speed,0.7f,0.3f,0.5f,40,-40);
    Incremental_PID_Init(&RMotor_B_Speed,0.5f,0.35f,0.5f,40,-40);
    PIDInit(&Angle_PID,0.44f,0,2,1.5f,-1.5f);
    PIDInit(&Image_PID,3.8f,0,0.5f,2.5f,-2.5f);
    PIDInit(&BorderPlace_PID,2.1f,0,0,1.5f,-1.5f);
    PIDInit(&Foward_PID,2.1f,0,0,1.5f,-1.5f);
    PIDInit(&Turn_PID,2.55f,0,0.6f,5,-5);   
    PIDInit(&Gyroz_PID,1.75f,0,0.25f,5,-5);
    PIDInit(&Gyroz_Pid,0.65f,0,0.25f,5,-5);
    PIDInit(&AngleControl_PID,0.44f,0,2,1.5f,-1.5f);
}


/**@brief   设置车三个方向的速度
-- @param   float Speed_X X轴速度，既横向速度
-- @param   float Speed_Y Y轴速度，既前向速度
-- @param   float Speed_Z Z轴速度，既旋转速度，正的为顺时针旋转
-- @auther  庄文标
-- @date    2023/11/5
**/
void Set_Car_Speed(float Speed_X,float Speed_Y,float Speed_Z)
{
    float LF_Speed,LB_Speed,RF_Spped,RB_Speed;

//运动解算
    LF_Speed = Speed_Y + Speed_X + Speed_Z;
    LB_Speed = Speed_Y - Speed_X + Speed_Z;
    RF_Spped = Speed_Y - Speed_X - Speed_Z;
    RB_Speed = Speed_Y + Speed_X - Speed_Z;

//速度环
    Set_Motor_Speed(LMotor_F,Get_Incremental_PID_Value(&LMotor_F_Speed,LF_Speed-Encoer_Speed[0]));
    Set_Motor_Speed(LMotor_B,Get_Incremental_PID_Value(&LMotor_B_Speed,LB_Speed-Encoer_Speed[2]));
    Set_Motor_Speed(RMotor_F,Get_Incremental_PID_Value(&RMotor_F_Speed,RF_Spped-Encoer_Speed[1]));
    Set_Motor_Speed(RMotor_B,Get_Incremental_PID_Value(&RMotor_B_Speed,RB_Speed-Encoer_Speed[3]));
}

/**@brief   转动指定角度
-- @param   float Target_Angle 需要转动的角度
-- @auther  庄文标
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

    Offset_Erro = (Target_Angle - Turn.Offset)/50.0f;
    if(Target_Angle - Turn.Offset >= 2.5f)
    {
        Yaw_Erro = 2 + GetPIDValue(&Turn_PID,Offset_Erro);
        Car.Speed_X = 0;
        Car.Speed_Y = 0;
        Car.Speed_Z = GetPIDValue(&Gyroz_Pid,Yaw_Erro - IMU_Data.gyro_z);
        Turn_Finsh = false;
    }
    else if(Target_Angle - Turn.Offset <= -2.5f)
    {
        Yaw_Erro = -2 + GetPIDValue(&Turn_PID,Offset_Erro);
        Car.Speed_X = 0;
        Car.Speed_Y = 0;
        Car.Speed_Z = GetPIDValue(&Gyroz_Pid,Yaw_Erro - IMU_Data.gyro_z);
        Turn_Finsh = false;
    }
    else
    {
        Car.Speed_X = 0;
        Car.Speed_Y = 0;
        Car.Speed_Z = 0;
        Turn_Finsh = true;
        Turn.Finish = true;
    }

    Turn.Angle_Last = Turn.Current_Angle;
}

void Turn_Reset()
{
    Turn.Offset = 0;
    Turn.Finish = false;
}

/**@brief   角度控制
-- @param   需要维持的角度
-- @auther  庄文标
-- @return  PID值
-- @date    2024/3/31
**/
float Angle_Control(float Start_Angle)
{
    float Yaw_Err = 0.0f;
    Yaw_Err = GetPIDValue(&AngleControl_PID,Start_Angle - Gyro_YawAngle_Get());
    return GetPIDValue(&Gyroz_PID,Yaw_Err - IMU_Data.gyro_z);
}


/**@brief   巡线
-- @param   无
-- @auther  庄文标
-- @date    2023/12/23
**/
void Car_run()
{
    float Image_Erro_ = GetPIDValue(&Image_PID,(70 - Image_Erro)*0.03f);
    Car.Speed_X = 0;
    Car.Speed_Y = Forward_Speed;
    Car.Speed_Z = -Image_Erro_;
#ifdef debug_switch
    printf("line\r\n");
#endif
}

/**@brief   改变小车行进方向朝向为目标板方向
-- @param   无
-- @auther  戴骐阳
-- @date    2023/12/23
**/
void Change_Direction(void)
{
    Turn_Angle(-90);
#ifdef debug_switch
    printf("FIND\r\n");
#endif
}

/**@brief   右转到右边赛道
-- @param   无
-- @auther  庄文标
-- @date    2024/4/26
**/
void Change_Right(void)
{
    Turn_Angle(90);
#ifdef debug_switch
    printf("FIND\r\n");
#endif
}

/**@brief   右边返回赛道
-- @param   无
-- @auther  庄文标
-- @date    2024/4/26
**/
void Return_Right()
{
    Turn_Angle(-89);
}

/**@brief   左边返回赛道
-- @param   无
-- @auther  庄文标
-- @date    2024/4/26
**/
void Return_Action()
{
    Turn_Angle(89);
}

/**@brief   左边等待微调串口发来的数据
-- @param   无
-- @auther  庄文标
-- @date    2024/4/26
**/
void Car_Stop_Wait_Data_L()
{
    UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);
    Car.Speed_X = 3;
    Car.Speed_Y = 0;
    Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
#ifdef debug_switch
    printf("wait\r\n");
#endif
}

/**@brief   右边等待微调串口发来的数据
-- @param   无
-- @auther  庄文标
-- @date    2024/4/26
**/
void Car_Stop_Wait_Data_R()
{
    UART_SendByte(&_UART_FINE_TUNING, START_FINETUNING);
    Car.Speed_X = -3;
    Car.Speed_Y = 0;
    Car.Speed_Z = Angle_Control(Navigation.Start_Angle);
#ifdef debug_switch
    printf("wait\r\n");
#endif
}

/**@brief   移动至目标板前方
-- @param   无
-- @auther  庄文标
-- @date    2024/4/7
**/
void Move_Action()
{
    Navigation_Process(FINETUNING_DATA.dx/10.f,FINETUNING_DATA.dy/10.f);
#ifdef debug_switch
    printf("move\r\n");
    printf("tx:%f\r\n,",FINETUNING_DATA.dx/10.f);
    printf("x:%f\r\n",Get_X_Distance());
    printf("ty:%f\r\n,",FINETUNING_DATA.dy/10.f);
    printf("y:%f\r\n",Get_Y_Distance());
#endif
}

