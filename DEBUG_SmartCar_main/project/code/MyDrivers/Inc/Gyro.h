#ifndef _GYRO_H_
#define _GYRO_H_

#include "zf_common_headfile.h"

typedef struct
{
    float RollAngle;   

    float PitchAngle;  

    float YawAngle;    

}Gyro_AngleTypeDef;

typedef struct
{
    float Xdata ;   

    float Ydata ;  

    float Zdata;   

}Gyro_Param_t;

typedef struct
{
    float acc_x ;   

    float acc_y ;  

    float acc_z;    

    float gyro_x;

    float gyro_y;

    float gyro_z;

}IMU_Param_t;

typedef struct
{
    float q0;
    float q1;
    float q2;
    float q3;

}Quater_Param_t;

float Gyro_RollAngle_Get(void);
float Gyro_PitchAngle_Get(void);
float Gyro_YawAngle_Get(void);
void Gyro_Get_All_Angles();
void Gyro_Offset_Init(void);

#endif
