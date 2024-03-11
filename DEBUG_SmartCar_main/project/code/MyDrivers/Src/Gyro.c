/**
  ******************************************************************************
  * @file    Gyro.c
  * @author  ׯ�ı�
  * @brief   ��̬���㣬�ٶȽ���
  * @date    11/04/2023
    @verbatim
    ��̬���ٶȽ���
    @endverbatim
  * @{
**/


/* Includes ------------------------------------------------------------------*/
#include "Gyro.h"
#include "math.h"

/* Define\Declare ------------------------------------------------------------*/
#define delta_T     0.005f  // ��������1ms ��Ƶ��1KHZ
#define  pI  3.1415926f
float I_ex, I_ey, I_ez;  // ������
float IMU_kp= 0.15f;    // ���ٶȼƵ��������ʱ�������
float IMU_ki= 0.002f;   // �������������ʵĻ�������
Quater_Param_t Q_info = {1, 0, 0, 0};  // ��Ԫ����ʼ��
Gyro_AngleTypeDef Gyro_Angle;
Gyro_Param_t GyroOffset;
Gyro_Param_t AccOffset;
IMU_Param_t IMU_Data;
float YawAngle_Trans = 0;//ת�����ƫ���ǣ��鵽0-360

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   �����ĸ���
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/7/4
**/
static float My_Rsqrt(float num)
{
    float halfx = 0.5f * num;
    float y = num;
    long i = *(long*)&y;
    i = 0x5f375a86 - (i >> 1);

    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));

    return y;
}

/**@brief   �������ݵ��˲���ת��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/7/4
**/
static void IMU_Get_Values(void)
{
    float alpha = 0.5;
    //һ�׵�ͨ�˲�����λg
    IMU_Data.acc_x = (((float) imu660ra_acc_x) * alpha) / 4096 + IMU_Data.acc_x * (1 - alpha);
    IMU_Data.acc_y = (((float) imu660ra_acc_y) * alpha) / 4096 + IMU_Data.acc_y * (1 - alpha);
    IMU_Data.acc_z = (((float) imu660ra_acc_z) * alpha) / 4096 + IMU_Data.acc_z * (1 - alpha);

    //! �����ǽ��ٶȱ���ת��Ϊ�����ƽ��ٶ�: deg/s -> rad/s
    IMU_Data.gyro_x = ((float) imu660ra_gyro_x - GyroOffset.Xdata) * pI / 180 / 16.4f;
    IMU_Data.gyro_y = ((float) imu660ra_gyro_y - GyroOffset.Ydata) * pI / 180 / 16.4f;
    IMU_Data.gyro_z = ((float) imu660ra_gyro_z - GyroOffset.Zdata) * pI / 180 / 16.4f;
}



/**@brief   ������ԭ���Ļ�ȡ
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/7/4
**/
static void IMU_AHRS_update(IMU_Param_t* IMU)
{
    float halfT = 0.5 * delta_T;    // ��������һ��
    float vx, vy, vz;               // ��ǰ��̬��������������������ϵķ���
    float ex, ey, ez;               // ��ǰ���ټƲ�õ��������ٶ��������ϵķ������õ�ǰ��̬��������������������ϵķ��������

    float q0 = Q_info.q0;  //��Ԫ��
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;

    float q0q0 = q0 * q0;  //����ˣ������������
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
//    float q0q3 = q0 * q3;
    float q1q1 = q1 * q1;
//    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;

    // ������ֹ״̬Ϊ-g ����������
    if(IMU->acc_x * IMU->acc_y * IMU->acc_z == 0) // �Ӽƴ�����������״̬ʱ(��ʱg = 0)��������̬���㣬��Ϊ�������ĸ���������
        return;

    // �Լ��ٶ����ݽ��й�һ�� �õ���λ���ٶ� (a^b -> ��������ϵ�µļ��ٶ�)
    float norm = My_Rsqrt(IMU->acc_x * IMU->acc_x + IMU->acc_y * IMU->acc_y + IMU->acc_z * IMU->acc_z);
    IMU->acc_x = IMU->acc_x * norm;
    IMU->acc_y = IMU->acc_y * norm;
    IMU->acc_z = IMU->acc_z * norm;

    // ��������ϵ���������������ϵķ���
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    // g^b �� a^b ��������ˣ��õ������ǵ�У����������e��ϵ��
    ex = IMU->acc_y * vz - IMU->acc_z * vy;
    ey = IMU->acc_z * vx - IMU->acc_x * vz;
    ez = IMU->acc_x * vy - IMU->acc_y * vx;

    // ����ۼ�
    I_ex += halfT * ex;
    I_ey += halfT * ey;
    I_ez += halfT * ez;

    // ʹ��pI�������������������(������Ư�����)
    IMU->gyro_x = IMU->gyro_x + IMU_kp* ex + IMU_ki* I_ex;
    IMU->gyro_y = IMU->gyro_y + IMU_kp* ey + IMU_ki* I_ey;
    IMU->gyro_z = IMU->gyro_z + IMU_kp* ez + IMU_ki* I_ez;

    // һ����������������Ԫ��΢�ַ��̣�����halfTΪ�������ڵ�1/2��gx gy gzΪbϵ�����ǽ��ٶȡ�
    q0 = q0 + (-q1 * IMU->gyro_x - q2 * IMU->gyro_y - q3 * IMU->gyro_z) * halfT;
    q1 = q1 + (q0 * IMU->gyro_x + q2 * IMU->gyro_z - q3 * IMU->gyro_y) * halfT;
    q2 = q2 + (q0 * IMU->gyro_y - q1 * IMU->gyro_z + q3 * IMU->gyro_x) * halfT;
    q3 = q3 + (q0 * IMU->gyro_z + q1 * IMU->gyro_y - q2 * IMU->gyro_x) * halfT;

    // ��λ����Ԫ���ڿռ���תʱ�������죬������ת�Ƕȣ������㷨�������Դ�����������任
    norm = My_Rsqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    Q_info.q0 = q0 * norm;
    Q_info.q1 = q1 * norm;
    Q_info.q2 = q2 * norm;
    Q_info.q3 = q3 * norm;  // ��ȫ�ֱ�����¼��һ�μ������Ԫ��ֵ
}

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ���������ݵĳ�ʼ�����������Ʈ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/7/4
**/
void Gyro_Offset_Init(void)
{
    GyroOffset.Xdata = 0;
    GyroOffset.Ydata = 0;
    GyroOffset.Zdata = 0;
    for (uint16_t i = 0; i < 200; ++i)
    {
        imu660ra_get_gyro();    // ��ȡ�����ǽ��ٶ�
        GyroOffset.Xdata += imu660ra_gyro_x;
        GyroOffset.Ydata += imu660ra_gyro_y;
        GyroOffset.Zdata += imu660ra_gyro_z;
        system_delay_ms(10);    // ��� 1Khz
    }

    GyroOffset.Xdata /= 200.0f;
    GyroOffset.Ydata /= 200.0f;
    GyroOffset.Zdata /= 200.0f;
}


/**@brief   ��ȡ������
-- @param   ��
-- @auther  ׯ�ı�
-- @return  Gyro_Angle->RollAngle ������
-- @date    2023/6/29
**/
float Gyro_RollAngle_Get(void)
{
    return Gyro_Angle.RollAngle;
}

/**@brief   ��ȡ������
-- @param   ��
-- @auther  ׯ�ı�
-- @return  Gyro_Angle->PitchAngle ������
-- @date    2023/6/29
**/
float Gyro_PitchAngle_Get(void)
{
    return Gyro_Angle.PitchAngle;
}

/**@brief   ��ȡƫ����
-- @param   ��
-- @auther  ׯ�ı�
-- @return  Gyro_Angle->YawAngle ƫ����
-- @date    2023/11/04
**/
float Gyro_YawAngle_Get(void)
{
    return YawAngle_Trans;
}

/**@brief   ��ȡ����������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/7/4
**/
void Gyro_Get_All_Angles()
{
    static float Yaw_Angle_Old = 0;
    imu660ra_get_acc();
    imu660ra_get_gyro();
    IMU_Get_Values();
    // printf("X_Distant = %lf\r\n",IMU_Data.acc_x*0.005f);
    IMU_AHRS_update(&IMU_Data);
    float q0 = Q_info.q0;
    float q1 = Q_info.q1;
    float q2 = Q_info.q2;
    float q3 = Q_info.q3;
    // atan2�������������������ԭ��������X�������μнǵĻ���ֵ
    Gyro_Angle.PitchAngle = asin(2 * q0 * q2 - 2 * q1 * q3) * 180 / pI;
    Gyro_Angle.RollAngle = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 180 / pI;
    Gyro_Angle.YawAngle = -atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 180 / pI;
    float i = Gyro_Angle.YawAngle - Yaw_Angle_Old;
    if (i < -180)//Ϊ�˱��������㷨�ĽǶ����䣬ȫ��ת��Ϊ0-360 
    {
        i += 360;
    }
    else if (i > 180) 
    {
        i -= 360;
    }
    YawAngle_Trans += i;
    Yaw_Angle_Old = Gyro_Angle.YawAngle;
}




