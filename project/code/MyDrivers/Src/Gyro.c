/**
  ******************************************************************************
  * @file    Gyro.c
  * @author  庄文标
  * @brief   姿态解算，速度解算
  * @date    11/04/2023
    @verbatim
    姿态、速度解算
    @endverbatim
  * @{
**/


/* Includes ------------------------------------------------------------------*/
#include "Gyro.h"
#include "math.h"

/* Define\Declare ------------------------------------------------------------*/
#define delta_T     0.005f  // 采样周期1ms 即频率1KHZ
#define  pI  3.1415926f
float I_ex, I_ey, I_ez;  // 误差积分
float IMU_kp= 0.17;    // 加速度计的收敛速率比例增益
float IMU_ki= 0.004;   // 陀螺仪收敛速率的积分增益
Quater_Param_t Q_info = {1, 0, 0, 0};  // 四元数初始化
Gyro_AngleTypeDef Gyro_Angle;
Gyro_Param_t GyroOffset;
Gyro_Param_t AccOffset;
IMU_Param_t IMU_Data;

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   陀螺仪数据的初始化，避免零点飘逸
-- @param   无
-- @auther  庄文标
-- @date    2023/7/4
**/
void Gyro_Offset_Init(void)
{
    GyroOffset.Xdata = 0;
    GyroOffset.Ydata = 0;
    GyroOffset.Zdata = 0;
    for (uint16_t i = 0; i < 100; ++i)
    {
        imu660ra_get_gyro();    // 获取陀螺仪角速度
        GyroOffset.Xdata += imu660ra_gyro_x;
        GyroOffset.Ydata += imu660ra_gyro_y;
        GyroOffset.Zdata += imu660ra_gyro_z;
        system_delay_ms(5);    // 最大 1Khz
    }

    GyroOffset.Xdata /= 100;
    GyroOffset.Ydata /= 100;
    GyroOffset.Zdata /= 100;
}


/**@brief   获取滚动角
-- @param   无
-- @auther  庄文标
-- @return  Gyro_Angle->RollAngle 滚动角
-- @date    2023/6/29
**/
double Gyro_RollAngle_Get(void)
{
    return Gyro_Angle.RollAngle;
}

/**@brief   获取俯仰角
-- @param   无
-- @auther  庄文标
-- @return  Gyro_Angle->PitchAngle 俯仰角
-- @date    2023/6/29
**/
double Gyro_PitchAngle_Get(void)
{
    return Gyro_Angle.PitchAngle;
}

/**@brief   获取偏航角
-- @param   无
-- @auther  庄文标
-- @return  Gyro_Angle->YawAngle 偏航角
-- @date    2023/11/04
**/
double Gyro_YawAngle_Get(void)
{
    return Gyro_Angle.YawAngle;
}

/**@brief   获取陀螺仪数据
-- @param   无
-- @auther  庄文标
-- @date    2023/7/4
**/
void Gyro_Get_All_Angles()
{
    imu660ra_get_acc();
    imu660ra_get_gyro();
    IMU_Get_Values();
    Get_ACC_Distant();
    // printf("X_Distant = %lf\r\n",IMU_Data.acc_x*0.005f);
    IMU_AHRS_update(&IMU_Data);
    float q0 = Q_info.q0;
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;
    // atan2返回输入坐标点与坐标原点连线与X轴正方形夹角的弧度值
    Gyro_Angle.PitchAngle = asin(2 * q0 * q2 - 2 * q1 * q3) * 180 / pI;
    Gyro_Angle.RollAngle = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 / pI;
    Gyro_Angle.YawAngle = -atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 / pI;

}
double X_Distant = 0;
double X_Velocity = 0;

void Get_ACC_Distant()
{
    X_Velocity = 0;
    X_Velocity+=IMU_Data.acc_y*0.005f;
    // X_Distant+= X_Velocity;
}
/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   倒数的根号
-- @param   无
-- @auther  庄文标
-- @date    2023/7/4
**/
float My_Rsqrt(float num)
{
    float halfx = 0.5f * num;
    float y = num;
    long i = *(long*)&y;
    i = 0x5f375a86 - (i >> 1);

    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));

    return y;
}

/**@brief   六轴数据的滤波和转换
-- @param   无
-- @auther  庄文标
-- @date    2023/7/4
**/
void IMU_Get_Values(void)
{
    float alpha = 0.3;

    //一阶低通滤波，单位g
    IMU_Data.acc_x = (((float) imu660ra_acc_x) * alpha) / 4096 + IMU_Data.acc_x * (1 - alpha);
    IMU_Data.acc_y = (((float) imu660ra_acc_y) * alpha) / 4096 + IMU_Data.acc_y * (1 - alpha);
    IMU_Data.acc_z = (((float) imu660ra_acc_z) * alpha) / 4096 + IMU_Data.acc_z * (1 - alpha);

    //! 陀螺仪角速度必须转换为弧度制角速度: deg/s -> rad/s
    IMU_Data.gyro_x = ((float) imu660ra_gyro_x - GyroOffset.Xdata) * pI / 180 / 16.4f;
    IMU_Data.gyro_y = ((float) imu660ra_gyro_y - GyroOffset.Ydata) * pI / 180 / 16.4f;
    IMU_Data.gyro_z = ((float) imu660ra_gyro_z - GyroOffset.Zdata) * pI / 180 / 16.4f;
}



/**@brief   六轴四原数的获取
-- @param   无
-- @auther  庄文标
-- @date    2023/7/4
**/
void IMU_AHRS_update(IMU_Param_t* IMU)
{
    float halfT = 0.5 * delta_T;    // 采样周期一半
    float vx, vy, vz;               // 当前姿态计算得来的重力在三轴上的分量
    float ex, ey, ez;               // 当前加速计测得的重力加速度在三轴上的分量与用当前姿态计算得来的重力在三轴上的分量的误差

    float q0 = Q_info.q0;  //四元数
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;

    float q0q0 = q0 * q0;  //先相乘，方便后续计算
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
//    float q0q3 = q0 * q3;
    float q1q1 = q1 * q1;
//    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;

    // 正常静止状态为-g 反作用力。
    if(IMU->acc_x * IMU->acc_y * IMU->acc_z == 0) // 加计处于自由落体状态时(此时g = 0)不进行姿态解算，因为会产生分母无穷大的情况
        return;

    // 对加速度数据进行归一化 得到单位加速度 (a^b -> 载体坐标系下的加速度)
    float norm = My_Rsqrt(IMU->acc_x * IMU->acc_x + IMU->acc_y * IMU->acc_y + IMU->acc_z * IMU->acc_z);
    IMU->acc_x = IMU->acc_x * norm;
    IMU->acc_y = IMU->acc_y * norm;
    IMU->acc_z = IMU->acc_z * norm;

    // 载体坐标系下重力在三个轴上的分量
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    // g^b 与 a^b 做向量叉乘，得到陀螺仪的校正补偿向量e的系数
    ex = IMU->acc_y * vz - IMU->acc_z * vy;
    ey = IMU->acc_z * vx - IMU->acc_x * vz;
    ez = IMU->acc_x * vy - IMU->acc_y * vx;

    // 误差累加
    I_ex += halfT * ex;
    I_ey += halfT * ey;
    I_ez += halfT * ez;

    // 使用pI控制器消除向量积误差(陀螺仪漂移误差)
    IMU->gyro_x = IMU->gyro_x + IMU_kp* ex + IMU_ki* I_ex;
    IMU->gyro_y = IMU->gyro_y + IMU_kp* ey + IMU_ki* I_ey;
    IMU->gyro_z = IMU->gyro_z + IMU_kp* ez + IMU_ki* I_ez;

    // 一阶龙格库塔法求解四元数微分方程，其中halfT为测量周期的1/2，gx gy gz为b系陀螺仪角速度。
    q0 = q0 + (-q1 * IMU->gyro_x - q2 * IMU->gyro_y - q3 * IMU->gyro_z) * halfT;
    q1 = q1 + (q0 * IMU->gyro_x + q2 * IMU->gyro_z - q3 * IMU->gyro_y) * halfT;
    q2 = q2 + (q0 * IMU->gyro_y - q1 * IMU->gyro_z + q3 * IMU->gyro_x) * halfT;
    q3 = q3 + (q0 * IMU->gyro_z + q1 * IMU->gyro_y - q2 * IMU->gyro_x) * halfT;

    // 单位化四元数在空间旋转时不会拉伸，仅有旋转角度，下面算法类似线性代数里的正交变换
    norm = My_Rsqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    Q_info.q0 = q0 * norm;
    Q_info.q1 = q1 * norm;
    Q_info.q2 = q2 * norm;
    Q_info.q3 = q3 * norm;  // 用全局变量记录上一次计算的四元数值
}


