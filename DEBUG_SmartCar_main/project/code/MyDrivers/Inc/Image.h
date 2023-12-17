#ifndef MYDRIVERS_INC_IMAGE_H_
#define MYDRIVERS_INC_IMAGE_H_

#include "UserMain.h"
void Turn_To_Bin(void);
void Image_Process();
int My_Abs(int value);
int16 Limit_a_b(int16 x, int a, int b);
uint8 Otsu_Threshold(uint8 *Image, uint16 col, uint16 row);

#define Image_H 80//图像高度
#define Image_W 148//图像宽度
#define White_Pixel 255
#define Black_Pixel 0
#define Border_Max  Image_W-2 //边界最大值
#define Border_Min  1   //边界最小值
extern uint8 Original_Image[Image_H][Image_W];
extern double Image_Erro;
extern uint8 Image_Thereshold;

typedef struct
{
    int8_t Cross_Fill;
}Flag_Handle;

#endif /* MYDRIVERS_INC_IMAGE_H_ */
