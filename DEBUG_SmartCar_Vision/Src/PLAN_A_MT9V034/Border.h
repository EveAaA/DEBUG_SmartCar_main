#ifndef _BORDER_H
#define _BORDER_H
#include "zf_common_headfile.h"
#include <math.h>
#define CannyMode 2   // 边缘检测模式宏定义
#define DEBUG_MODE 1  // 输出阶段性测试图像宏定义
#define IMAGE_H 80    // 80
#define IMAGE_W 148   // 148
#define RadToAngle(rad) 57.29578*(rad)

typedef struct Image{
    uint8_t GrayImage[IMAGE_H][IMAGE_W];
    uint8_t OutImage[IMAGE_H][IMAGE_W];
    uint8_t HarrisImage[IMAGE_H][IMAGE_W];
    // uint8_t RGBImage[IMAGE_H][IMAGE_W]; 
}ImageTypeDef;


extern void FindBorder(double k, double Threshold);

#endif