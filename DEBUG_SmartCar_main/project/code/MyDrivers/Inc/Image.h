#ifndef MYDRIVERS_INC_IMAGE_H_
#define MYDRIVERS_INC_IMAGE_H_
#include "zf_common_headfile.h"

void Image_Process();

#define Image_H 80//ͼ��߶�
#define Image_W 148//ͼ����
#define White_Pixel 255
#define Black_Pixel 0
#define Border_Max  Image_W-2 //�߽����ֵ
#define Border_Min  1   //�߽���Сֵ
extern uint8 Original_Image[Image_H][Image_W];
extern float Image_Erro;
extern uint8 Image_Thereshold;

typedef struct
{
    int8_t Cross_Fill;
}Flag_Handle;

#endif /* MYDRIVERS_INC_IMAGE_H_ */
