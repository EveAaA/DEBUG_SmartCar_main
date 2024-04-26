#ifndef MYDRIVERS_INC_IMAGE_H_
#define MYDRIVERS_INC_IMAGE_H_
#include "zf_common_headfile.h"

void Image_Process();

#define Image_H 80//图像高度
#define Image_W 148//图像宽度
#define White_Pixel 255
#define Black_Pixel 0
#define Border_Max  Image_W-2 //边界最大值
#define Border_Min  1   //边界最小值
extern uint8 Original_Image[Image_H][Image_W];
extern float Image_Erro;
extern uint8 Image_Thereshold;
extern uint8 L_Border[Image_H];//左线数组
extern uint8 R_Border[Image_H];//右线数组
extern uint8 Center_Line[Image_H];//中线数组
extern uint8 Bin_Image[Image_H][Image_W];//二值化图像数组
extern uint8 Hightest;//最高点
typedef struct
{
    bool Cross_Fill;//十字标志位
}Flag_Handle;

typedef enum {
	Ring_Front = 0,//入环前
	Enter_Ring_First,//第一次进环
	Leave_Ring_First,//第一次离开环
}Ring_State_Handle;

typedef struct {
	bool Stright_Line;//直线
	bool Ring_Front_Flag;//前方是否存在圆环
	bool Enter_Ring_First_Flag;
	bool Leave_Ring_First_Flag;
	Ring_State_Handle Ring_State;
}Ring_Handle;

#endif /* MYDRIVERS_INC_IMAGE_H_ */
