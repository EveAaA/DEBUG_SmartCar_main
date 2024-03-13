/**
  ******************************************************************************
  * @file    Image.c
  * @author  none
  * @brief   图像部分
  *
    @verbatim
    
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Image.h"
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define use_num     1   //1就是不压缩，2就是压缩一倍
uint8 Original_Image[Image_H][Image_W];//原始图像数组
uint8 Image_Thereshold;//图像分割阈值
uint8 Bin_Image[Image_H][Image_W];//二值化图像数组
uint8 Start_Point_L[2] = { 0 };//左边起点的x，y值
uint8 Start_Point_R[2] = { 0 };//右边起点的x，y值

#define USE_num Image_H*3   //定义找点的数组成员个数按理说300个点能放下，但是有些特殊情况确实难顶，多定义了一点
//存放点的x，y坐标
uint16 Points_L[(uint16)USE_num][2] = { {  0 } };//左线
uint16 Points_R[(uint16)USE_num][2] = { {  0 } };//右线
uint16 Dir_R[(uint16)USE_num] = { 0 };//用来存储右边生长方向
uint16 Dir_L[(uint16)USE_num] = { 0 };//用来存储左边生长方向
uint16 Data_Stastics_L = 0;//统计左边找到点的个数
uint16 Data_Stastics_R = 0;//统计右边找到点的个数
uint8 Hightest = 0;//最高点

uint8 L_Border[Image_H];//左线数组
uint8 R_Border[Image_H];//右线数组
uint8 Center_Line[Image_H];//中线数组

//定义膨胀和腐蚀的阈值区间
#define Threshold_Max   255*5//此参数可根据自己的需求调节
#define Threshold_Min   255*2//此参数可根据自己的需求调节

float Image_Erro;

Flag_Handle Image_Flag;//元素标志位

/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**----------------------------------------------------数学公式------------------------------------------------------------------------**/
/**@brief    求绝对值
-- @param    int value 输入的值
-- @return   value 绝对值
-- @auther   none
-- @date     2023/10/2
**/
int My_Abs(int value)
{
    if(value>=0) return value;
    else return -value;
}


/**@brief    限幅函数
-- @param    int16 x 需要限幅的值
-- @param    int a 最大值
-- @param    int b 最小值
-- @return   x 输出
-- @auther   none
-- @date     2023/10/2
**/
int16 Limit_a_b(int16 x, int a, int b)
{
    if(x<a) x = a;
    if(x>b) x = b;
    return x;
}


/**@brief    求x,y中的最小值
-- @param    int16 x x值
-- @param    int16 y y值
-- @return   返回两值中的最小值
-- @auther   none
-- @date     2023/10/2
**/
int16 Limit1(int16 x, int16 y)
{
    if (x > y)             return y;
    else if (x < -y)       return -y;
    else                return x;
}

/** 
* @brief 最小二乘法
* @param uint8 begin				输入起点
* @param uint8 end					输入终点
* @param uint8 *border				输入需要计算斜率的边界首地址
*  @see CTest		Slope_Calculate(start, end, border);//斜率
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/
float Slope_Calculate(uint8 begin, uint8 end, uint8 *border)
{
	float xsum = 0, ysum = 0, xysum = 0, x2sum = 0;
	int16 i = 0;
	float result = 0;
	static float resultlast;

	for (i = begin; i < end; i++)
	{
		xsum += i;
		ysum += border[i];
		xysum += i * (border[i]);
		x2sum += i * i;

	}
	if ((end - begin)*x2sum - xsum * xsum) //判断除数是否为零
	{
		result = ((end - begin)*xysum - xsum * ysum) / ((end - begin)*x2sum - xsum * xsum);
		resultlast = result;
	}
	else
	{
		result = resultlast;
	}
	return result;
}

/** 
* @brief 计算斜率截距
* @param uint8 start				输入起点
* @param uint8 end					输入终点
* @param uint8 *border				输入需要计算斜率的边界
* @param float *slope_rate			输入斜率地址
* @param float *intercept			输入截距地址
*  @see CTest		calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/
void calculate_s_i(uint8 start, uint8 end, uint8 *border, float *slope_rate, float *intercept)
{
	uint16 i, num = 0;
	uint16 xsum = 0, ysum = 0;
	float y_average, x_average;

	num = 0;
	xsum = 0;
	ysum = 0;
	y_average = 0;
	x_average = 0;
	for (i = start; i < end; i++)
	{
		xsum += i;
		ysum += border[i];
		num++;
	}

	//计算各个平均数
	if (num)
	{
		x_average = (float)(xsum / num);
		y_average = (float)(ysum / num);

	}

	/*计算斜率*/
	*slope_rate = Slope_Calculate(start, end, border);//斜率
	*intercept = y_average - (*slope_rate)*x_average;//截距
}

/**----------------------------------------------------大津法部分------------------------------------------------------------------------**/
/**@brief    获取一副灰度图像
-- @param    uint8(*mt9v03x_image)[Image_W] 摄像头采集图像对应的指针
-- @auther   none
-- @date     2023/10/2
**/
void Get_Image(uint8(*mt9v03x_image)[Image_W])
{
    uint8 i = 0, j = 0, Row = 0, Line = 0;
    for (i = 0; i < Image_H; i += use_num)
    {
        for (j = 0; j <Image_W; j += use_num)
        {
            Original_Image[Row][Line] = mt9v03x_image[i][j];
            Line++;
        }
        Line = 0;
        Row++;
    }
}

/**@brief    大津法获取动态阈值
-- @param    uint8 *image 需要处理的图像
-- @param    uint16 col 列长度
-- @param    uint16 row 行长度
-- @auther   none
-- @return   uint8 Threshold 动态阈值
-- @date     2023/10/2
**/
uint8 Otsu_Threshold(uint8 *Image, uint16 col, uint16 row)
{
#define GrayScale 256
    uint16 Image_Width  = col;
    uint16 Image_Height = row;
    int X; uint16 Y;
    uint8* data = Image;
    int HistGram[GrayScale] = {0};//直方图

    uint32 Amount = 0;
    uint32 Pixel_Back = 0;//背景像素
    uint32 Pixel_Integral_Back = 0;//背景像素的累积灰度值
    uint32 Pixel_Integral = 0;//像素累计值
    int32 Pixel_IntegralFore = 0;//前景像素的累积灰度值
    int32 Pixel_Fore = 0;//前景像素，被认为是目标的像素
    float Omega_Back=0, Omega_Fore=0, Micro_Back=0, Micro_Fore=0, SigmaB=0, Sigma=0; // 类间方差，分别为背景像素的相对权重，前景像素的相对权重，背景像素的平均灰度值，前景像素的平均灰度值;
    uint8 Min_Value=0, Max_Value=0;
    uint8 Threshold = 0;//阈值


    for (Y = 0; Y <Image_Height; Y++) //Y<Image_Height改为Y =Image_Height；以便进行 行二值化
    {
        //Y=Image_Height;
        for (X = 0; X < Image_Width; X++)
        {
            HistGram[(int)data[Y*Image_Width + X]]++; //统计每个灰度值的个数信息
        }
    }




    for (Min_Value = 0; Min_Value < 255 && HistGram[Min_Value] == 0; Min_Value++) ;        //获取最小灰度的值
    for (Max_Value = 255; Max_Value > Min_Value && HistGram[Min_Value] == 0; Max_Value--) ; //获取最大灰度的值

    if (Max_Value == Min_Value)
    {
        return Max_Value;          // 图像中只有一个颜色
    }
    if (Min_Value + 1 == Max_Value)
    {
        return Min_Value;      // 图像中只有二个颜色
    }

    for (Y = Min_Value; Y <= Max_Value; Y++)
    {
        Amount += HistGram[Y];        //  像素总数
    }

    Pixel_Integral = 0;
    for (Y = Min_Value; Y <= Max_Value; Y++)
    {
        Pixel_Integral += HistGram[Y] * Y;//灰度值总数
    }
    SigmaB = -1;
    for (Y = Min_Value; Y < Max_Value; Y++)
    {
          Pixel_Back = Pixel_Back + HistGram[Y];    //前景像素点数
          Pixel_Fore = Amount - Pixel_Back;         //背景像素点数
          Omega_Back = (float)Pixel_Back / Amount;//前景像素百分比
          Omega_Fore = (float)Pixel_Fore / Amount;//背景像素百分比
          Pixel_Integral_Back += HistGram[Y] * Y;  //前景灰度值
          Pixel_IntegralFore = Pixel_Integral - Pixel_Integral_Back;//背景灰度值
          Micro_Back = (float)Pixel_Integral_Back / Pixel_Back;//前景灰度百分比
          Micro_Fore = (float)Pixel_IntegralFore / Pixel_Fore;//背景灰度百分比
          Sigma = Omega_Back * Omega_Fore * (Micro_Back - Micro_Fore) * (Micro_Back - Micro_Fore);//g
          if (Sigma > SigmaB)//遍历最大的类间方差g
          {
              SigmaB = Sigma;
              Threshold = (uint8)Y;
          }
    }
   return Threshold;
}


/**@brief    大津法图像二值化
-- @param    无
-- @auther   none
-- @date     2023/10/2
**/
void Turn_To_Bin(void)
{
  uint8 i,j;
  Image_Thereshold = 1.075 * Otsu_Threshold(Original_Image[0], Image_W, Image_H);//获取大津法阈值
  for(i = 0;i<Image_H;i++)
  {
      for(j = 0;j<Image_W;j++)
      {
          if(Original_Image[i][j]>Image_Thereshold)
          {
              Bin_Image[i][j] = White_Pixel;
          }
          else
          {
              Bin_Image[i][j] = Black_Pixel;
          }
      }
  }
}


/**----------------------------------------------------八领域部分------------------------------------------------------------------------**/
/**@brief    寻找两个边界的边界点作为八邻域循环的起始点
-- @param    uint8 Start_Row 输入任意行数
-- @auther   none
-- @date     2023/10/3
**/
uint8 Get_Start_Point(uint8 Start_Row)
{
    uint8 i = 0,L_Found = 0,R_Found = 0;
    //清零
    Start_Point_L[0] = 0;//x
    Start_Point_L[1] = 0;//y

    Start_Point_R[0] = 0;//x
    Start_Point_R[1] = 0;//y

        //从中间往左边，先找起点
    for (i = Image_W / 2; i > Border_Min; i--)
    {
        Start_Point_L[0] = i;//x
        Start_Point_L[1] = Start_Row;//y
        if (Bin_Image[Start_Row][i] == 255 && Bin_Image[Start_Row][i - 1] == 0)//找到黑白跳变点
        {
            //printf("找到左边起点image[%d][%d]\n", Start_Row,i);
            L_Found = 1;
            break;
        }
    }

    for (i = Image_W / 2; i < Border_Max; i++)
    {
        Start_Point_R[0] = i;//x
        Start_Point_R[1] = Start_Row;//y
        if (Bin_Image[Start_Row][i] == 255 && Bin_Image[Start_Row][i + 1] == 0)
        {
            //printf("找到右边起点image[%d][%d]\n",Start_Row, i);
            R_Found = 1;
            break;
        }
    }

    if(L_Found&&R_Found)//左边和右边都找到起点
    {
        return 1;
    }
    else
    {
        //printf("未找到起点\n");
        return 0;
    }
}


/**@brief    八邻域找左右边点的函数
-- @param    uint16 Break_Flag 最多需要循环的次数
-- @param    uint8(*image)[Image_W] 需要进行找点的图像数组，必须是二值图,填入数组名称即可
-- @param    uint16 *L_Stastic 统计左边数据，用来输入初始数组成员的序号和取出循环次数
-- @param    uint16 *R_Stastic 统计右边数据，用来输入初始数组成员的序号和取出循环次数
-- @param    uint8 L_Start_X 左边起点横坐标
-- @param    uint8 L_Start_Y 左边起点纵坐标
-- @param    uint8 R_Start_X 右边起点横坐标
-- @param    uint8 R_Start_Y 右边起点纵坐标
-- @param    uint8 Hightest 循环结束所得到的最高高度
-- @auther   none
-- @date     2023/10/3
**/
void Search_L_R(uint16 Break_Flag, uint8(*image)[Image_W], uint16 *L_Stastic, uint16 *R_Stastic, uint8 L_Start_X, uint8 L_Start_Y, uint8 R_Start_X, uint8 R_Start_Y, uint8*Hightest)
{

    uint8 i = 0, j = 0;

    //左边变量
    uint8 Search_Filds_L[8][2] = {{0}};//寻找八领域数组
    uint8 Index_L = 0;
    uint8 Temp_L[8][2] = {{0}};
    uint8 Center_Point_L[2] = {0};
    uint16 L_Data_Statics;//统计左边找到的点
    //定义八个邻域
    static int8 Seeds_L[8][2] = { {0,  1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,  0},{1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //这个是顺时针

    //右边变量
    uint8 Search_Filds_R[8][2] = { {  0 } };
    uint8 Center_Point_R[2] = { 0 };//中心坐标点
    uint8 Index_R = 0;//索引下标
    uint8 Temp_R[8][2] = { {  0 } };
    uint16 R_Data_Statics;//统计右边
    //定义八个邻域
    static int8 Seeds_R[8][2] = { {0,  1},{1,1},{1,0}, {1,-1},{0,-1},{-1,-1}, {-1,  0},{-1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //这个是逆时针

    L_Data_Statics = *L_Stastic;//统计找到了多少个点，方便后续把点全部画出来
    R_Data_Statics = *R_Stastic;//统计找到了多少个点，方便后续把点全部画出来

    //第一次更新坐标点，起点，将上一步找到的起点值传进来
    Center_Point_L[0] = L_Start_X;//x
    Center_Point_L[1] = L_Start_Y;//y
    Center_Point_R[0] = R_Start_X;//x
    Center_Point_R[1] = R_Start_Y;//y

        //开启邻域循环
    while (Break_Flag--)
    {

        //左边
        for (i = 0; i < 8; i++)//传递八领域坐标
        {
            Search_Filds_L[i][0] = Center_Point_L[0] + Seeds_L[i][0];//x
            Search_Filds_L[i][1] = Center_Point_L[1] + Seeds_L[i][1];//y
        }
        //中心坐标点填充到已经找到的点内
        Points_L[L_Data_Statics][0] = Center_Point_L[0];//x
        Points_L[L_Data_Statics][1] = Center_Point_L[1];//y
        L_Data_Statics++;//索引加一

        //右边
        for (i = 0; i < 8; i++)//传递八领域坐标
        {
            Search_Filds_R[i][0] = Center_Point_R[0] + Seeds_R[i][0];//x
            Search_Filds_R[i][1] = Center_Point_R[1] + Seeds_R[i][1];//y
        }
        //中心坐标点填充到已经找到的点内
        Points_R[R_Data_Statics][0] = Center_Point_R[0];//x
        Points_R[R_Data_Statics][1] = Center_Point_R[1];//y

        Index_L = 0;//先清零，后使用
        for (i = 0; i < 8; i++)
        {
            Temp_L[i][0] = 0;//先清零，后使用
            Temp_L[i][1] = 0;//先清零，后使用
        }

        //左边判断
        //Search_Filds_L[i][1]表示八领域点的X值，Search_Filds_L[i][0]]表示八领域点的Y值
        for (i = 0; i < 8; i++)
        {
            if (image[Search_Filds_L[i][1]][Search_Filds_L[i][0]] == 0
                && image[Search_Filds_L[(i + 1) & 7][1]][Search_Filds_L[(i + 1) & 7][0]] == 255)//两个相邻号位存在黑白跳变点
            {
                Temp_L[Index_L][0] = Search_Filds_L[(i)][0];//记录下当前黑点
                Temp_L[Index_L][1] = Search_Filds_L[(i)][1];
                Index_L++;
                Dir_L[L_Data_Statics - 1] = (i);//记录生长方向
            }

            if (Index_L)
            {
                //更新坐标点，以新的中心点开始八领域
                Center_Point_L[0] = Temp_L[0][0];//x
                Center_Point_L[1] = Temp_L[0][1];//y
                for (j = 0; j < Index_L; j++)
                {
                    if (Center_Point_L[1] > Temp_L[j][1])//判断一下边线是不是往上走的
                    {
                        Center_Point_L[0] = Temp_L[j][0];//x
                        Center_Point_L[1] = Temp_L[j][1];//y
                    }
                }
            }

        }
        if ((Points_R[R_Data_Statics][0]== Points_R[R_Data_Statics-1][0]&& Points_R[R_Data_Statics][0] == Points_R[R_Data_Statics - 2][0]
            && Points_R[R_Data_Statics][1] == Points_R[R_Data_Statics - 1][1] && Points_R[R_Data_Statics][1] == Points_R[R_Data_Statics - 2][1])
            ||(Points_L[L_Data_Statics-1][0] == Points_L[L_Data_Statics - 2][0] && Points_L[L_Data_Statics-1][0] == Points_L[L_Data_Statics - 3][0]
                && Points_L[L_Data_Statics-1][1] == Points_L[L_Data_Statics - 2][1] && Points_L[L_Data_Statics-1][1] == Points_L[L_Data_Statics - 3][1]))
        {
            //printf("三次进入同一个点，退出\n");
            break;
        }
        if (My_Abs(Points_R[R_Data_Statics][0] - Points_L[L_Data_Statics - 1][0]) < 2
            && My_Abs(Points_R[R_Data_Statics][1] - Points_L[L_Data_Statics - 1][1] < 2)
            )
        {
            //printf("\n左右相遇退出\n");
            *Hightest = (Points_R[R_Data_Statics][1] + Points_L[L_Data_Statics - 1][1]) >> 1;//取出最高点
            //printf("\n在y=%d处退出\n",*Hightest);
            break;
        }
        if ((Points_R[R_Data_Statics][1] < Points_L[L_Data_Statics - 1][1]))
        {
            // printf("\n如果左边比右边高了，左边等待右边\n");
            continue;//如果左边比右边高了，左边等待右边
        }
        if (Dir_L[L_Data_Statics - 1] == 7
            && (Points_R[R_Data_Statics][1] > Points_L[L_Data_Statics - 1][1]))//左边比右边高且已经向下生长了
        {
            //printf("\n左边开始向下了，等待右边，等待中... \n");
            Center_Point_L[0] = Points_L[L_Data_Statics - 1][0];//x
            Center_Point_L[1] = Points_L[L_Data_Statics - 1][1];//y
            L_Data_Statics--;
        }
        R_Data_Statics++;//索引加一

        Index_R = 0;//先清零，后使用
        for (i = 0; i < 8; i++)
        {
            Temp_R[i][0] = 0;//先清零，后使用
            Temp_R[i][1] = 0;//先清零，后使用
        }

        //右边判断
        for (i = 0; i < 8; i++)
        {
            if (image[Search_Filds_R[i][1]][Search_Filds_R[i][0]] == 0
                && image[Search_Filds_R[(i + 1) & 7][1]][Search_Filds_R[(i + 1) & 7][0]] == 255)
            {
                Temp_R[Index_R][0] = Search_Filds_R[(i)][0];
                Temp_R[Index_R][1] = Search_Filds_R[(i)][1];
                Index_R++;//索引加一
                Dir_R[R_Data_Statics - 1] = (i);//记录生长方向
                //printf("dir[%d]:%d\n", R_Data_Statics - 1, Dir_R[R_Data_Statics - 1]);
            }
            if (Index_R)
            {

                //更新坐标点
                Center_Point_R[0] = Temp_R[0][0];//x
                Center_Point_R[1] = Temp_R[0][1];//y
                for (j = 0; j < Index_R; j++)
                {
                    if (Center_Point_R[1] > Temp_R[j][1])
                    {
                        Center_Point_R[0] = Temp_R[j][0];//x
                        Center_Point_R[1] = Temp_R[j][1];//y
                    }
                }

            }
        }


    }


    //取出循环次数
    *L_Stastic = L_Data_Statics;
    *R_Stastic = R_Data_Statics;

}


/**@brief   从八邻域边界里提取需要的左边线
-- @param   uint16 Total_L 找到的点的总数
-- @auther  none
-- @date    2023/10/3
**/
void Get_Left(uint16 Total_L)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    //初始化
    for (i = 0;i<Image_H;i++)
    {
        L_Border[i] = Border_Min;
    }
    h = Image_H - 2;
    //左边
    for (j = 0; j < Total_L; j++)
    {
        //printf("%d\n", j);
        if (Points_L[j][1] == h)
        {
            L_Border[h] = Points_L[j][0]+1;
        }
        else
        {
            continue;//每行只取一个点，没到下一行就不记录
        }
        h--;
        if (h == 0)
        {
            break;//到最后一行退出
        }
    }
}

/**@brief   从八邻域边界里提取需要的右边线
-- @param   uint16 Total_R 找到的点的总数
-- @auther  none
-- @date    2023/10/3
**/
void Get_Right(uint16 Total_R)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    for (i = 0; i < Image_H; i++)
    {
        R_Border[i] = Border_Max;//右边线初始化放到最右边，左边线放到最左边，这样八邻域闭合区域外的中线就会在中间，不会干扰得到的数据
    }
    h = Image_H - 2;
    //右边
    for (j = 0; j < Total_R; j++)
    {
        if (Points_R[j][1] == h)
        {
            R_Border[h] = Points_R[j][0] - 1;
        }
        else
        {
            continue;//每行只取一个点，没到下一行就不记录
        }
        h--;
        if (h == 0)
        {
            break;//到最后一行退出
        }
    }
}

/**----------------------------------------------------图像滤波部分------------------------------------------------------------------------**/
/**@brief   形态学滤波
-- @param   uint8(*Bin_Image)[Image_W] 二值化图像
-- @auther  none
-- @date    2023/10/3
**/
void Image_Filter(uint8(*Bin_Image)[Image_W])//形态学滤波，简单来说就是膨胀和腐蚀的思想
{
    uint16 i, j;
    uint32 num = 0;


    for (i = 1; i < Image_H - 1; i++)
    {
        for (j = 1; j < (Image_W - 1); j++)
        {
            //统计八个方向的像素值
            num =
                Bin_Image[i - 1][j - 1] + Bin_Image[i - 1][j] + Bin_Image[i - 1][j + 1]
                + Bin_Image[i][j - 1] + Bin_Image[i][j + 1]
                + Bin_Image[i + 1][j - 1] + Bin_Image[i + 1][j] + Bin_Image[i + 1][j + 1];


            if (num >= Threshold_Max && Bin_Image[i][j] == 0)
            {

                Bin_Image[i][j] = 255;//白  可以搞成宏定义，方便更改

            }
            if (num <= Threshold_Min && Bin_Image[i][j] == 255)
            {

                Bin_Image[i][j] = 0;//黑

            }

        }
    }

}


/**@brief   给图像画一个黑框
-- @param   uint8(*Image)[Image_W]  图像首地址
-- @auther  none
-- @date    2023/10/3
**/
void Image_Draw_Rectan(uint8(*Image)[Image_W])
{

    uint8 i = 0;
    for (i = 0; i < Image_H; i++)
    {
        Image[i][0] = 0;
        Image[i][1] = 0;
        Image[i][Image_W - 1] = 0;
        Image[i][Image_W - 2] = 0;

    }
    for (i = 0; i < Image_W; i++)
    {
        Image[0][i] = 0;
        Image[1][i] = 0;
        //image[Image_H-1][i] = 0;

    }
}

/**----------------------------------------------------元素部分------------------------------------------------------------------------**/
/** 
* @brief 十字补线函数
* @param uint8(*Bin_Image)[Image_W]		输入二值图像
* @param uint8 *L_Border			输入左边界首地址
* @param uint8 *R_Border			输入右边界首地址
* @param uint16 Total_Num_L			输入左边循环总次数
* @param uint16 Total_Num_R			输入右边循环总次数
* @param uint16 *Dir_L				输入左边生长方向首地址
* @param uint16 *Dir_R				输入右边生长方向首地址
* @param uint16(*Points_L)[2]		输入左边轮廓首地址
* @param uint16(*Points_R)[2]		输入右边轮廓首地址
*  @see CTest		Cross_Fill(Bin_Image,L_Border, R_Border, data_statics_l, data_statics_r, Dir_L, Dir_R, Points_L, Points_R);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
 */
void Cross_Fill(uint8(*Bin_Image)[Image_W], uint8 *L_Border, uint8 *R_Border, uint16 Total_Num_L, uint16 Total_Num_R,
										 uint16 *Dir_L, uint16 *Dir_R, uint16(*Points_L)[2], uint16(*Points_R)[2])
{
	uint8 i;
	uint8 Break_Num_L = 0;
	uint8 Break_Num_R = 0;
	uint8 start, end;
	float slope_l_rate = 0, intercept_l = 0;
	//出十字
	for (i = 1; i < Total_Num_L; i++)
	{
		if (Dir_L[i - 1] == 4 && Dir_L[i] == 4 && Dir_L[i + 3] == 6 && Dir_L[i + 5] == 6 && Dir_L[i + 7] == 6)
		{
			Break_Num_L = Points_L[i][1];//传递y坐标
			// printf("brea_knum-L:%d\n", Break_Num_L);
			// printf("I:%d\n", i);
			// printf("十字标志位：1\n");
			break;
		}
	}
	for (i = 1; i < Total_Num_R; i++)
	{
		if (Dir_R[i - 1] == 4 && Dir_R[i] == 4 && Dir_R[i + 3] == 6 && Dir_R[i + 5] == 6 && Dir_R[i + 7] == 6)
		{
			Break_Num_R = Points_R[i][1];//传递y坐标
			// printf("brea_knum-R:%d\n", Break_Num_R);
			// printf("I:%d\n", i);
			// printf("十字标志位：1\n");
			break;
		}
	}
	if (Break_Num_L && Break_Num_R && Bin_Image[Image_H - 1][4] && Bin_Image[Image_H - 1][Image_W - 4])//两边生长方向都符合条件
	{
		//计算斜率
		start = Break_Num_L - 15;
		start = Limit_a_b(start, 0, Image_H);
		end = Break_Num_L - 5;
		calculate_s_i(start, end, L_Border, &slope_l_rate, &intercept_l);
		//printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
		for (i = Break_Num_L - 5; i < Image_H - 1; i++)
		{
			L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
			L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
		}

		//计算斜率
		start = Break_Num_R - 15;//起点
		start = Limit_a_b(start, 0, Image_H);//限幅
		end = Break_Num_R - 5;//终点
		calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
		//printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
		for (i = Break_Num_R - 5; i < Image_H - 1; i++)
		{
			R_Border[i] = slope_l_rate * (i)+intercept_l;
			R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);
		}
	}

}


/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   最终调用的图像处理的函数
-- @param   无
-- @auther  none
-- @date    2023/10/3
**/
void Image_Process()
{
    uint16 i;
    uint8 Hightest = 0;//定义一个最高行，tip：这里的最高指的是y值的最小

    Get_Image(mt9v03x_image);//获取一副图像
    Turn_To_Bin();//二值化

    Image_Filter(Bin_Image);//滤波
    Image_Draw_Rectan(Bin_Image);//预处理

    //清零
    Data_Stastics_L = 0;
    Data_Stastics_R = 0;

    if(Get_Start_Point(Image_H - 2))//从图像的最下面开始找，找到起点了，再执行八领域，没找到就一直找
    {
//        printf("正在开始八领域\n");
        Search_L_R((uint16)USE_num, Bin_Image, &Data_Stastics_L, &Data_Stastics_R, Start_Point_L[0], Start_Point_L[1], Start_Point_R[0], Start_Point_R[1], &Hightest);
//        printf("八邻域已结束\n");
        // 从爬取的边界线内提取边线 ， 这个才是最终有用的边线
        Get_Left(Data_Stastics_L);
        Get_Right(Data_Stastics_R);
        //元素处理函数放这里
        Cross_Fill(Bin_Image,L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);//十字补线
    }

    // for(i = 0;i < Data_Stastics_L;i++)
    // {
    //     tft180_draw_point(Points_L[i][0]+2, Points_L[i][1], RGB565_BLUE);
    // }
    // for (i = 0; i < Data_Stastics_R; i++)
    // {
    //     tft180_draw_point(Points_R[i][0]+2, Points_R[i][1], RGB565_RED);
    // }

    for (i = Hightest; i < Image_H-1; i++)
    {
        Center_Line[i] = (L_Border[i] + R_Border[i]) >> 1;//求中线
        //求中线最好最后求，不管是补线还是做状态机，全程最好使用一组边线，中线最后求出，不能干扰最后的输出
        tft180_draw_point(Center_Line[i], i, RGB565_BLACK);//显示起点 显示中线
        tft180_draw_point(L_Border[i], i, RGB565_BLUE);//显示起点 显示左边线
        tft180_draw_point(R_Border[i], i, RGB565_RED);//显示起点 显示右边线
    }
    Image_Erro = (Center_Line[59])*0.375 + (Center_Line[60])*0.5 + (Center_Line[61])*0.1;
}

