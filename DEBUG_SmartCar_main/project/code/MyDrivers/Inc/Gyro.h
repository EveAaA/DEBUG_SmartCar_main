#ifndef _GYRO_H_
#define _GYRO_H_

#include "zf_common_headfile.h"

typedef struct
{
    double RollAngle;   

    double PitchAngle;  

    double YawAngle;    

}Gyro_AngleTypeDef;

typedef struct
{
    double Xdata ;   

    double Ydata ;  

    double Zdata;   

}Gyro_Param_t;

typedef struct
{
    double acc_x ;   

    double acc_y ;  

    double acc_z;    

    double gyro_x;

    double gyro_y;

    double gyro_z;

}IMU_Param_t;

typedef struct
{
    double q0;
    double q1;
    double q2;
    double q3;

}Quater_Param_t;

double Gyro_RollAngle_Get(void);
double Gyro_PitchAngle_Get(void);
double Gyro_YawAngle_Get(void);
void Gyro_Get_All_Angles();
float My_Rsqrt(float num);
void Gyro_Offset_Init(void);
void IMU_Get_Values(void);
void IMU_AHRS_update(IMU_Param_t* IMU);
#endif
