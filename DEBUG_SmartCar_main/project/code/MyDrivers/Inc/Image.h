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
extern float Image_Erro_Y;
extern uint8 Image_Thereshold;
extern uint8 L_Border[Image_H];//左线数组
extern uint8 R_Border[Image_H];//右线数组
extern uint8 Center_Line[Image_H];//中线数组
extern uint8 Bin_Image[Image_H][Image_W];//二值化图像数组
extern uint8 Hightest;//最高点


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
    bool Cross_Fill;//十字标志位
}Flag_Handle;

//元素参数结构体
typedef struct {
	uint8 forkpoint;//三岔点
	uint8 cross_up;//十字上拐点
	uint8 cross_down;//十字下拐点
	uint8 circle_up;//环岛上拐点
	uint8 circle_down;//环岛下拐点
	uint8 circlepoint;//环岛中心点
	uint8 garage_up;//车库上拐点
	uint8 garage_down;//车库下拐点
	uint8 fill_start;//补线起点 tip：起点必须小于终点
	uint8 fill_end;//补线终点
}Element_Typedef;


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
}Ring_Handle;

typedef struct {
	float slope_rate;//横坐标
	float intercept;//纵坐标
	bool dir[Image_H];//有标标志位
	uint8 valid_num; //边界有效数据统计
}Pram_Typedef;

//赛道参数结构体
typedef struct {
	uint8 white_num;					 //白点数
	uint8 black_num;					 //黑点数
	Border_Typedef l_border[Image_H];   //左边线数组
	Border_Typedef r_border[Image_H];   //右边线数组
	Border_Typedef center_line[Image_H];//定义中线数组
	Pram_Typedef   l_pram;				 //左边界的参数
	Pram_Typedef   r_pram;				 //右边界的参数
	Element_Typedef   l_elem;			 //左边界的元素参数
	Element_Typedef   r_elem;			 //右边界的元素参数
	uint8 l_start;						 //右边搜索起点
	uint8 r_start;						 //右边搜索起点
	uint8 hightest;					 //最长列
	uint8 longest;						 //最长列坐标
}Road_Typedef;

#endif /* MYDRIVERS_INC_IMAGE_H_ */
