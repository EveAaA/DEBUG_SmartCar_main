#ifndef MEAU_H_
#define MEAU_H_

#include "UserMain.h"

void Menu_Display();
void Flash_Init();
//行的宏定义
#define Line_0 (16*0)//第零行
#define Line_1 (16*1)//第一行
#define Line_2 (16*2)//第二行
#define Line_3 (16*3)//第三行
#define Line_4 (16*4)//第四行
#define Line_5 (16*5)//第五行
#define Line_6 (16*6)//第六行
#define Line_7 (16*7)//第七行
#define Line_8 (16*8)//第八行
#define Line_9 (16*9)//第九行
#define Line_10 (16*10)//第十行
#define Line_11 (16*11)//第十一行
#define Line_12 (16*12)//第十二行
#define Line_13 (16*13)//第十三行
#define Line_14 (16*14)//第十四行
#define Line_15 (16*15)//第十五行
#define Line_16 (16*16)//第十六行
//列的宏定义
#define Row_0 (8*0)//第零列
#define Row_1 (8*1)//第一列
#define Row_2 (8*2)//第二列
#define Row_3 (8*3)//第三列
#define Row_4 (8*4)//第四列
#define Row_5 (8*5)//第五列
#define Row_6 (8*6)//第六列
#define Row_7 (8*7)//第七列
#define Row_8 (8*8)//第八列
#define Row_9 (8*9)//第九列
#define Row_10 (8*10)//第十列
#define Row_11 (8*11)//第十一列
#define Row_12 (8*12)//第十二列
#define Row_13 (8*13)//第十三列
#define Row_14 (8*14)//第十四列
#define Row_15 (8*15)//第十五列
#define Row_16 (8*16)//第十六列
#define Row_17 (8*17)//第十五列
#define Row_18 (8*18)//第十六列
typedef enum
{
	Page0 = 0,
	Page1 = 1,
	Page2 = 2,
	Page3 = 3,
	Page4 = 4,
	Page5 = 5,
	Page6 = 6,
	Page7 = 7,
	Page8 = 8,
	Page9 = 9,
	Page_Select = 10,//页面选择
}Menu_Handle;

typedef enum
{
	Normal_Mode = 0,
	Flash_Mode,
}Mode_Handle;

typedef struct
{
	uint8 Set_Line;//设置的行
	uint8 Flash_Set;
	Mode_Handle Set_Mode;
	bool  Image_Show;//是否为图像显示
	uint16 Ex_Time;//曝光时间
	uint8 Turn_Point;
	uint16 Zebra_First_Dis;//斑马前距离
}Menu_;

extern int Show_Mode;
extern Menu_ Menu;
#endif
