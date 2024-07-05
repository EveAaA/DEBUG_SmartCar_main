#ifndef MYDRIVERS_INC_IMAGE_H_
#define MYDRIVERS_INC_IMAGE_H_
#include "zf_common_headfile.h"
void Get_Bin_Image();
void Image_Process();

#define Image_H 80//图像高度
#define Image_W 148//图像宽度
#define White_Pixel 255
#define Black_Pixel 0
#define Border_Max  Image_W-2 //边界最大值
#define Border_Min  1   //边界最小值
#define USE_num Image_H*3   //定义找点的数组成员个数按理说300个点能放下，但是有些特殊情况确实难顶，多定义了一点
extern uint8 Original_Image[Image_H][Image_W];
extern float Image_Erro;
extern float Image_Erro_Y;
extern uint8 Image_Thereshold;
extern uint8 L_Border[Image_H];//左线数组
extern uint8 R_Border[Image_H];//右线数组
extern uint8 Center_Line[Image_H];//中线数组
extern uint8 Bin_Image[Image_H][Image_W];//二值化图像数组
extern uint8 Hightest;//最高点
extern uint16 Data_Stastics_L;//统计左边找到点的个数
extern uint16 Data_Stastics_R;//统计右边找到点的个数
extern uint16 Points_L[(uint16)USE_num][2];//左线
extern uint16 Points_R[(uint16)USE_num][2];//右线
//坐标点结构体
typedef struct {
	uint8 x;//横坐标
	uint8 y;//纵坐标
	uint8 valid_flag;//有标标志位
}Border_Typedef;

typedef enum {
	LeftLine,
	RightLine,
}lineTypeDef;

typedef struct
{
    uint8 Cross_Fill;//十字标志位
	bool Left_Ring;
	bool Right_Ring;
	bool Zerba;
	bool Roadblock;
	uint8 Cross_Type;
}Flag_Handle;

typedef enum {
	Ring_Front = 0,//入环前
	Enter_Ring_First,//第一次进环
	Leave_Ring_First,//第一次离开环
	In_Ring,   // 入环
	Ready_Out_Ring,  // 准备出环  
	Leave_Ring     // 出环
}Ring_State_Handle;

typedef struct {
	bool LeftStraightLine; // 左侧是否是直线
	bool Stright_Line;//直线
	uint8 Ring_Front_Flag;//前方是否存在圆环
	bool Ring;
	bool Enter_Ring_First_Flag;
	bool Leave_Ring_First_Flag;
	Ring_State_Handle Ring_State;
	uint16 Clear_Time;
}Ring_Handle;

typedef struct {
	float slope_rate;//横坐标
	float intercept;//纵坐标
	bool dir[Image_H];//有标标志位
	uint8 valid_num; //边界有效数据统计
}Pram_Typedef;
extern Ring_Handle RightRing;
extern Ring_Handle LeftRing;
extern Flag_Handle Image_Flag;
#endif /* MYDRIVERS_INC_IMAGE_H_ */
