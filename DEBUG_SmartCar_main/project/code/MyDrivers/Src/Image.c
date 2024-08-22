/**
  ******************************************************************************
  * @file    Image.c
  * @author  戴骐阳、庄文标
  * @brief   图像处理
  * @date    11/5/2023
    @verbatim

    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Image.h"
#include <math.h>
#include "Gyro.h"
#include "Beep.h"
#include "UserMain.h"
#include "My_FSM.h"
/* Define\Declare ------------------------------------------------------------*/
// #define Left_Ring_debug //是否开启左圆环调试
// #define Right_Ring_debug //是否开启右圆环调试

Ring_Handle LeftRing = { false, false, 0 };
Ring_Handle RightRing = { false, false, 0 };
Flag_Handle Image_Flag = {false,false,false,false,false,false,false};//元素标志位
/* Define\Declare ------------------------------------------------------------*/
#define use_num     1   //1就是不压缩，2就是压缩一倍
uint8 Original_Image[Image_H][Image_W];//原始图像数组
uint8 Image_Thereshold;//图像分割阈值
uint8 Bin_Image[Image_H][Image_W];//二值化图像数组
uint8 Start_Point_L[2] = { 0 };//左边起点的x，y值
uint8 Start_Point_R[2] = { 0 };//右边起点的x，y值

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
uint8 L_Border_Y[Image_W];//左线数组,横向
uint8 R_Border_Y[Image_W];//右线数组,横向
uint16 Left_White = 0;
uint16 Right_White = 0;
uint32 hashMapIndexL[Image_H];
uint32 hashMapIndexR[Image_H];

//定义膨胀和腐蚀的阈值区间
#define Threshold_Max   255*5//此参数可根据自己的需求调节
#define Threshold_Min   255*2//此参数可根据自己的需求调节

float Image_Erro;
float Image_Erro_Y;

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
-- @author   none
-- @date     2023/10/2
**/
int My_Abs(int value)
{
    if (value >= 0) return value;
    else return -value;
}


/**@brief    限幅函数
-- @param    int16_t x 需要限幅的值
-- @param    int a 最大值
-- @param    int b 最小值
-- @return   x 输出
-- @author   none
-- @date     2023/10/2
**/
int16_t Limit_a_b(int16_t x, int a, int b)
{
    if (x < a) x = a;
    if (x > b) x = b;
    return x;
}


/**@brief    求x,y中的最小值
-- @param    int16_t x x值
-- @param    int16_t y y值
-- @return   返回两值中的最小值
-- @author   none
-- @date     2023/10/2
**/
int16_t Limit1(int16_t x, int16_t y)
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
float Slope_Calculate(uint8 begin, uint8 end, uint8* border)
{
    float xsum = 0, ysum = 0, xysum = 0, x2sum = 0;
    int16_t i = 0;
    float result = 0;
    static float resultlast;

    for (i = begin; i < end; i++)
    {
        xsum += i;
        ysum += border[i];
        xysum += i * (border[i]);
        x2sum += i * i;

    }
    if ((end - begin) * x2sum - xsum * xsum) //判断除数是否为零
    {
        result = ((end - begin) * xysum - xsum * ysum) / ((end - begin) * x2sum - xsum * xsum);
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
* @param uint8 *border				输入需要计算斜率的边界
* @param uint8 end					输入终点
* @param float *slope_rate			输入斜率地址
* @param float *intercept			输入截距地址
*  @see CTest		calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/
void calculate_s_i(uint8 start, uint8 end, uint8* border, float* slope_rate, float* intercept)
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
    *intercept = y_average - (*slope_rate) * x_average;//截距
}

/**@brief    求两点斜率和截距
-- @param    uint8 x1 
-- @param    int16 y y值
-- @return   返回两值中的最小值
-- @author   none
-- @date     2023/10/2
**/
bool Get_K_b(uint8 x1,uint8 y1,uint8 x2,uint8 y2, float* slope_rate, float* intercept)
{
    if (x1 == x2) {
        return false;
    }

    // 计算斜率
    *slope_rate = (float)(y2 - y1) / (x2 - x1);

    // 计算截距
    *intercept = y1 - (*slope_rate * x1);

    return true;
}

/**----------------------------------------------------大津法部分------------------------------------------------------------------------**/
/**@brief    获取一副灰度图像
-- @param    uint8(*mt9v03x_image)[Image_W] 摄像头采集图像对应的指针
-- @author   none
-- @date     2023/10/2
**/
void Get_Image(uint8(*mt9v03x_image)[Image_W])
{
    uint8 i = 0, j = 0, Row = 0, Line = 0;
    for (i = 0; i < Image_H; i += use_num)
    {
        for (j = 0; j < Image_W; j += use_num)
        {
            Original_Image[Row][Line] = mt9v03x_image[i][j];
            Line++;
        }
        Line = 0;
        Row++;
    }
}

/**@brief    大津法获取动态阈值
-- @param    uint8 *Image 需要处理的图像
-- @param    uint16 col 列长度
-- @param    uint16 row 行长度
-- @author   none
-- @return   uint8 Threshold 动态阈值
-- @date     2023/10/2
**/
uint8 Otsu_Threshold(uint8* Image, uint16 col, uint16 row)
{
#define GrayScale 256
    uint16 Image_Width  = col;
    uint16 Image_Height = row;
    int X; uint16 Y;
    uint8* data = Image;
    int HistGram[GrayScale] = {0};

    uint32 Amount = 0;
    uint32 Pixel_Back = 0;
    uint32 Pixel_Integral_Back = 0;
    uint32 Pixel_Integral = 0;
    int32 Pixel_IntegralFore = 0;
    int32 Pixel_Fore = 0;
    double Omega_Back=0, Omega_Fore=0, Micro_Back=0, Micro_Fore=0, SigmaB=0, Sigma=0; 
    uint8 Min_Value=0, Max_Value=0;
    uint8 Threshold = 0;

    for (Y = 0; Y <Image_Height; Y++) 
    {
        //Y=Image_Height;
        for (X = 0; X < Image_Width; X++)
        {
            HistGram[(int)data[Y*Image_Width + X]]++; 
        }
    }

    for (Min_Value = 0; Min_Value < 255 && HistGram[Min_Value] == 0; Min_Value++) ;        
    for (Max_Value = 255; Max_Value > Min_Value && HistGram[Min_Value] == 0; Max_Value--) ; 

    if (Max_Value == Min_Value)
    {
        return Max_Value;          
    }
    if (Min_Value + 1 == Max_Value)
    {
        return Min_Value;      
    }

    for (Y = Min_Value; Y <= Max_Value; Y++)
    {
        Amount += HistGram[Y];        
    }

    Pixel_Integral = 0;
    for (Y = Min_Value; Y <= Max_Value; Y++)
    {
        Pixel_Integral += HistGram[Y] * Y;
    }
    SigmaB = -1;
    for (Y = Min_Value; Y < Max_Value; Y++)
    {
          Pixel_Back = Pixel_Back + HistGram[Y];    
          Pixel_Fore = Amount - Pixel_Back;         
          Omega_Back = (double)Pixel_Back / Amount;
          Omega_Fore = (double)Pixel_Fore / Amount;
          Pixel_Integral_Back += HistGram[Y] * Y;  
          Pixel_IntegralFore = Pixel_Integral - Pixel_Integral_Back;
          Micro_Back = (double)Pixel_Integral_Back / Pixel_Back;
          Micro_Fore = (double)Pixel_IntegralFore / Pixel_Fore;
          Sigma = Omega_Back * Omega_Fore * (Micro_Back - Micro_Fore) * (Micro_Back - Micro_Fore);
          if (Sigma > SigmaB)
          {
              SigmaB = Sigma;
              Threshold = (uint8)Y;
          }
    }
   return Threshold;
}

/**@brief    大津法图像二值化
-- @param    无
-- @author   none
-- @date     2023/10/2
**/
void Turn_To_Bin(void)
{
    uint8 i, j;
    Image_Thereshold = 1.075*Otsu_Threshold(Original_Image[0], Image_W, Image_H);//获取大津法阈值
    Left_White = 0;
    Right_White = 0;
    //   printf("begin");
    for (i = 0;i < Image_H;i++)
    {
        for (j = 0;j < Image_W;j++)
        {
            if (Original_Image[i][j] > Image_Thereshold)
            {
                Bin_Image[i][j] = White_Pixel;
            }
            else
            {
                Bin_Image[i][j] = Black_Pixel;
            }
            if ((i >= 35) && (j <= Image_W / 2 - 1) && (j >= Image_W / 2 - 60) && (Bin_Image[i][j] == White_Pixel))
            {
                Left_White += 1;
            }
            if ((i >= 35) && (j >= Image_W / 2 + 1) && (j <= Image_W / 2 + 60) && (Bin_Image[i][j] == White_Pixel))
            {
                Right_White += 1;
            }
            //   printf(" %d",Bin_Image[i][j]);
        }
        //   printf("\r\n");
    }
    //   printf("end");
}


/**----------------------------------------------------八领域部分------------------------------------------------------------------------**/
/**@brief    寻找两个边界的边界点作为八邻域循环的起始点
-- @param    uint8 Start_Row 输入任意行数
-- @author   none
-- @date     2023/10/3
**/
uint8 Get_Start_Point(uint8 Start_Row)
{
    uint8 i = 0, L_Found = 0, R_Found = 0;
    Image_Flag.L_Find = false;
    Image_Flag.R_Find = false;
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
            Image_Flag.L_Find = 1;
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
            Image_Flag.R_Find = 1;
            break;
        }
    }

    if (L_Found && R_Found)//左边和右边都找到起点
    {
        Image_Flag.Get_Start_Point = true;
        return 1;
    }
    else
    {
        Image_Flag.Get_Start_Point = false;
        //printf("未找到起点\n");
        return 0;
    }
}


/**@brief    八邻域找左右边点的函数
-- @param    uint16_t Break_Flag 最多需要循环的次数
-- @param    uint8(*image)[Image_W] 需要进行找点的图像数组，必须是二值图,填入数组名称即可
-- @param    uint16_t *L_Stastic 统计左边数据，用来输入初始数组成员的序号和取出循环次数
-- @param    uint16_t *R_Stastic 统计右边数据，用来输入初始数组成员的序号和取出循环次数
-- @param    uint8 L_Start_X 左边起点横坐标
-- @param    uint8 L_Start_Y 左边起点纵坐标
-- @param    uint8 R_Start_X 右边起点横坐标
-- @param    uint8 R_Start_Y 右边起点纵坐标
-- @param    uint8 Hightest 循环结束所得到的最高高度
-- @author   none
-- @date     2023/10/3
**/
void Search_L_R(uint16 Break_Flag, uint8(*image)[Image_W], uint16* L_Stastic, uint16* R_Stastic, uint8 L_Start_X, uint8 L_Start_Y, uint8 R_Start_X, uint8 R_Start_Y, uint8* Hightest)
{

    uint8 i = 0, j = 0;

    //左边变量
    uint8 Search_Filds_L[8][2] = { {0} };//寻找八领域数组
    uint8 Index_L = 0;
    uint8 Temp_L[8][2] = { {0} };
    uint8 Center_Point_L[2] = { 0 };
    uint16 L_Data_Statics;//统计左边找到的点
    //定义八个邻域
    static int8_t Seeds_L[8][2] = { {0,  1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,  0},{1, 1}, };
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
    static int8_t Seeds_R[8][2] = { {0,  1},{1,1},{1,0}, {1,-1},{0,-1},{-1,-1}, {-1,  0},{-1, 1}, };
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
                // printf("dir[%d]:%d\n", L_Data_Statics - 1, Dir_L[L_Data_Statics - 1]);
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
        if ((Points_R[R_Data_Statics][0] == Points_R[R_Data_Statics - 1][0] && Points_R[R_Data_Statics][0] == Points_R[R_Data_Statics - 2][0]
            && Points_R[R_Data_Statics][1] == Points_R[R_Data_Statics - 1][1] && Points_R[R_Data_Statics][1] == Points_R[R_Data_Statics - 2][1])
            || (Points_L[L_Data_Statics - 1][0] == Points_L[L_Data_Statics - 2][0] && Points_L[L_Data_Statics - 1][0] == Points_L[L_Data_Statics - 3][0]
                && Points_L[L_Data_Statics - 1][1] == Points_L[L_Data_Statics - 2][1] && Points_L[L_Data_Statics - 1][1] == Points_L[L_Data_Statics - 3][1]))
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
                // printf("dir[%d]:%d\n", R_Data_Statics - 1, Dir_R[R_Data_Statics - 1]);
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

/**@brief   从八邻域边界里提取需要的左边线，只有X坐标
-- @param   uint16_t Total_L 找到的点的总数
-- @author  none
-- @date    2023/10/3
**/
void Get_Left(uint16 Total_L)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    //初始化
    for (i = 0;i < Image_H;i++)
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
            L_Border[h] = Points_L[j][0] + 1;
            hashMapIndexL[h] = j; // 存入Edge边缘点入哈希表
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

/**@brief   从八邻域边界里提取需要的左边线，只有Y坐标
-- @param   uint16 Total_L 找到的点的总数
-- @author  庄文标
-- @date    2024/5/2
**/
void Get_Left_Y(uint16 Total_L)
{
    uint16 i = 0;
    uint16 j = 0;
    uint16 w = 0;
    w = Image_W / 2 - 1;
    for (j = Total_L; j > 0; j--)
    {
        if (Points_L[j][0] == w)
        {
            L_Border_Y[w] = Points_L[j][1] + 1;
            //j = 0;
            //draw_point(w, L_Border_Y[w], RED, road_Image);//显示拐点
        }
        else
        {
            continue;
        }
        w--;
        if (w == 0)
        {
            break;//到最后一行退出
        }
    }
}

/**@brief   从八邻域边界里提取需要的右边线，只有X坐标
-- @param   uint16_t Total_R 找到的点的总数
-- @author  none
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
            hashMapIndexR[h] = j;
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

/**@brief   从八邻域边界里提取需要的左边线，只有Y坐标
-- @param   uint16 Total_L 找到的点的总数
-- @author  庄文标
-- @date    2024/5/2
**/
void Get_Right_Y(uint16 Total_R)
{
    uint16 i = 0;
    uint16 j = 0;
    uint16 w = 0;
    w = Image_W / 2;
    for (j = Total_R; j > 0; j--)
    {
        if (Points_R[j][0] == w)
        {
            R_Border_Y[w] = Points_R[j][1] + 1;
            //j = 0;
            //draw_point(w, R_Border_Y[w], BLUE, road_Image);//显示拐点
        }
        else
        {
            continue;
        }
        w++;
        if (w == Image_W-2)
        {
            break;//到最后一行退出
        }
    }
}

/**----------------------------------------------------图像滤波部分------------------------------------------------------------------------**/
/**@brief   形态学滤波
-- @param   uint8(*Bin_Image)[Image_W] 二值化图像
-- @author  none
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
-- @author  none
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

void my_sobel(unsigned char imageIn[Image_H][Image_W], unsigned char imageOut[Image_H][Image_W])
{
    short KERNEL_SIZE = 3;
    short xStart = KERNEL_SIZE / 2;
    short xEnd = Image_W - KERNEL_SIZE / 2;
    short yStart = KERNEL_SIZE / 2;
    short yEnd = Image_H - KERNEL_SIZE / 2;
    short i, j;
    short temp[2];
    short temp1, temp2;
    //for(i = 0; i < Compress_H; i++)//算的更慢不过对比了全局图像
    for (i = yStart; i < yEnd; i++)   //有点的跳跃
    {
        //for(j = 0; j < Compress_W; j++)//算的更慢不过对比了全局图像
        for (j = xStart; j < xEnd; j++)  //有点的跳跃
        {
            /* 计算不同方向梯度幅值  */
            temp[0] = -(short)imageIn[i - 1][j - 1] + (short)imageIn[i - 1][j + 1]     //{{-1, 0, 1},
                - (short)2 * imageIn[i][j - 1] + (short)2 * imageIn[i][j + 1]       // {-2, 0, 2},
                - (short)imageIn[i + 1][j - 1] + (short)imageIn[i + 1][j + 1];    // {-1, 0, 1}};

            temp[1] = -(short)imageIn[i - 1][j - 1] + (short)imageIn[i + 1][j - 1]     //{{-1, -2, -1},
                - (short)2 * imageIn[i - 1][j] + (short)2 * imageIn[i + 1][j]       // { 0,  0,  0},
                - (short)imageIn[i - 1][j + 1] + (short)imageIn[i + 1][j + 1];    // { 1,  2,  1}};

            temp[0] = fabs(temp[0]);
            temp[1] = fabs(temp[1]);

            temp1 = temp[0] + temp[1];

            temp2 = (short)imageIn[i - 1][j - 1] + (short)2 * imageIn[i - 1][j] + (short)imageIn[i - 1][j + 1]
                + (short)2 * imageIn[i][j - 1] + (short)imageIn[i][j] + (short)2 * imageIn[i][j + 1]
                + (short)imageIn[i + 1][j - 1] + (short)2 * imageIn[i + 1][j] + (short)imageIn[i + 1][j + 1];

            if (temp1 > temp2 / 12.0f)
            {
                imageOut[i][j] = Black_Pixel;
            }
            else
            {
                imageOut[i][j] = White_Pixel;
            }
        }
    }
}

/**----------------------------------------------------元素部分------------------------------------------------------------------------**/

/**@brief   是否左右两边都丢线
-- @param   无、
-- @return  是否丢线：true-丢线，false-不丢线
-- @author  庄文标
-- @date    2024/6/15
**/
uint8 Lose_Line(void)
{
    uint8 Lose_Line_Point_L = 0; 
    uint8 Lose_Line_Point_R = 0; 

    for (int i = Image_H/2+20; i > Image_H/2-20; i -= 1) 
    {
        if (L_Border[i] <= 2)
        {
            Lose_Line_Point_L += 1;
        }

        if (R_Border[i] >= 144)
        {
            Lose_Line_Point_R += 1;
        }
    }
    // printf("Line: %d \n", StraightPoint);
    if ((Lose_Line_Point_L >= 10) && (Lose_Line_Point_R >= 10))
    {
        return 1;
    }
    else if ((Lose_Line_Point_L >= 10))
    {
        return 2;
    }
    else if ((Lose_Line_Point_R >= 10))
    {
        return 3;
    }
    else
    {
        return false;
    }
}


/**
* @brief 十字补线函数
* @param uint8(*Bin_Image)[Image_W]		输入二值图像
* @param uint8 *L_Border			输入左边界首地址
* @param uint8 *R_Border			输入右边界首地址
* @param uint16_t Total_Num_L			输入左边循环总次数
* @param uint16_t Total_Num_R			输入右边循环总次数
* @param uint16_t *Dir_L				输入左边生长方向首地址
* @param uint16_t *Dir_R				输入右边生长方向首地址
* @param uint16_t(*Points_L)[2]		输入左边轮廓首地址
* @param uint16_t(*Points_R)[2]		输入右边轮廓首地址
* @see CTest		Cross_Fill(Bin_Image,L_Border, R_Border, data_statics_l, data_statics_r, Dir_L, Dir_R, Points_L, Points_R);
 */
void Cross_Fill(uint8(*Bin_Image)[Image_W],  uint8* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R, uint16* Dir_L, uint16* Dir_R, uint16(*Points_L)[2], uint16(*Points_R)[2])
{
    uint16 i;
    uint16 Break_Num_L_UP = 0;//拐点
    uint16 Break_Num_R_UP = 0;
    uint16 Break_Num_L_DOWN = 0;
    uint16 Break_Num_R_DOWN = 0;
    uint16 start, end;
    float slope_l_rate = 0, intercept_l = 0;

    if (Lose_Line() == 1)
    {
        for (i = 3; i < Image_H / 2 + 35; i++)//寻找左上拐点
        {
            if (abs(L_Border[i] - L_Border[i - 1] <= 5)
                && (abs(L_Border[i - 1] - L_Border[i - 2]) <= 5)
                && (abs(L_Border[i - 2] - L_Border[i - 3]) <= 5)
                && (L_Border[i] - L_Border[i + 2] >= 7))
            {
                Break_Num_L_UP = i;//传递y坐标
                break;
            }
        }

        for (i = Image_H - 5; i > Image_H / 2 - 5; i--)//寻找左下拐点
        {
            //printf("L_Board[%d] = %d\r\n", i, L_Border[i]);
            if (abs(L_Border[i] - L_Border[i + 1] <= 5)
                && (abs(L_Border[i + 1] - L_Border[i + 2]) <= 5)
                && (abs(L_Border[i + 2] - L_Border[i + 3]) <= 5)
                && (L_Border[i] - L_Border[i - 2] >= 7))
            {
                Break_Num_L_DOWN = i;//传递y坐标
                break;
            }
        }

        for (i = 3; i < Image_H / 2 + 35; i++)//寻找右上拐点
        {
            if (abs(R_Border[i] - R_Border[i - 1] <= 5)
                && (abs(R_Border[i - 1] - R_Border[i - 2]) <= 5)
                && (abs(R_Border[i - 2] - R_Border[i - 3]) <= 5)
                && (R_Border[i + 2] - R_Border[i] >= 7))
            {
                Break_Num_R_UP = i;//传递y坐标
                break;
            }
        }

        for (i = Image_H - 5; i > Image_H / 2 - 5; i--)//寻找右下拐点
        {
            //printf("R_Border[%d] = %d\r\n", i, R_Border[i]);
            if (abs(R_Border[i] - R_Border[i + 1] <= 5)
                && (abs(R_Border[i + 1] - R_Border[i + 2]) <= 5)
                && (abs(R_Border[i + 2] - R_Border[i + 3]) <= 5)
                && (R_Border[i - 2] - R_Border[i] >= 7))
            {
                Break_Num_R_DOWN = i;//传递y坐标
                break;
            }
        }
    }

    if ((Break_Num_R_DOWN - Break_Num_R_UP < 0) && (Break_Num_R_DOWN) && (Break_Num_R_UP))
    {
        Break_Num_R_DOWN = 0;
    }
    if ((Break_Num_L_DOWN - Break_Num_L_UP < 0) && (Break_Num_L_DOWN) && (Break_Num_L_UP))
    {
        Break_Num_L_DOWN = 0;
    }
    if ((Break_Num_R_UP - Break_Num_R_DOWN > 0) && (Break_Num_R_DOWN) && (Break_Num_R_UP))
    {
        Break_Num_R_UP = 0;
    }
    if ((Break_Num_L_UP - Break_Num_L_DOWN > 0) && (Break_Num_L_DOWN) && (Break_Num_L_UP))
    {
        Break_Num_L_UP = 0;
    }

    if ((Break_Num_L_DOWN) && (Break_Num_L_UP) && (Break_Num_R_DOWN) && (Break_Num_R_UP))//同时找到四个拐点 1111
    {
        Image_Flag.Cross_Fill = 1;
        //计算斜率,左边斜率
        Get_K_b(L_Border[Break_Num_L_DOWN], Break_Num_L_DOWN, L_Border[Break_Num_L_UP],Break_Num_L_UP,&slope_l_rate,&intercept_l);
        for (i = Break_Num_L_DOWN; i > Break_Num_L_UP; i--)
        {
            L_Border[i] = (i - intercept_l)/ slope_l_rate;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
        }

        //计算斜率,右边斜率
        Get_K_b(R_Border[Break_Num_R_DOWN], Break_Num_R_DOWN, R_Border[Break_Num_R_UP], Break_Num_R_UP, &slope_l_rate, &intercept_l);
        for (i = Break_Num_R_DOWN; i > Break_Num_R_UP; i--)
        {
            R_Border[i] = (i - intercept_l) / slope_l_rate;//y = kx+b
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
        }
    }
    else if ((Break_Num_L_DOWN) && (Break_Num_L_UP) && (!Break_Num_R_DOWN) && (Break_Num_R_UP))//右斜入十字 1101
    {
        Image_Flag.Cross_Fill = 1;
        //计算斜率
        Get_K_b(L_Border[Break_Num_L_UP], Break_Num_L_UP, L_Border[Break_Num_L_DOWN], Break_Num_L_DOWN, &slope_l_rate, &intercept_l);
        for (i = Break_Num_L_DOWN; i > Break_Num_L_UP; i--)
        {
            L_Border[i] = (i - intercept_l) / slope_l_rate;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
        }

        //计算斜率
        start = Break_Num_R_UP - 15;//起点
        start = Limit_a_b(start, 5, Image_H-5);//限幅
        end = Break_Num_R_UP;//终点
        Get_K_b(R_Border[start], start, R_Border[end], end, &slope_l_rate, &intercept_l);
        for (i = Break_Num_R_UP; i < Image_H - 2; i++)
        {
            R_Border[i] = (i - intercept_l) / slope_l_rate;//y = kx+b
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);
        }
    }
    else if ((!Break_Num_L_DOWN) && (Break_Num_L_UP) && (Break_Num_R_UP) && (Break_Num_R_DOWN))//左斜入十字 0111
    {
        Image_Flag.Cross_Fill = 1;
        //计算斜率
        start = Break_Num_L_UP - 15;
        start = Limit_a_b(start, 5, Image_H - 5);
        end = Break_Num_L_UP;
        Get_K_b(L_Border[start], start, L_Border[end], end, &slope_l_rate, &intercept_l);
        for (i = Break_Num_L_UP; i < Image_H - 2; i++)
        {
            L_Border[i] = (i - intercept_l) / slope_l_rate;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
        }

        //计算斜率
        Get_K_b(R_Border[Break_Num_R_UP], Break_Num_R_UP, R_Border[Break_Num_R_DOWN], Break_Num_R_DOWN, &slope_l_rate, &intercept_l);
        for (i = Break_Num_R_DOWN; i > Break_Num_R_UP; i--)
        {
            R_Border[i] = (i - intercept_l) / slope_l_rate;//y = kx+b
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
        }
    }
    else if ((Break_Num_L_UP) && (Break_Num_R_UP) && (!Break_Num_L_DOWN) && (!Break_Num_R_DOWN) && (Bin_Image[10][Image_H - 10]) && (Bin_Image[130][Image_H - 10]))//只有上面两个点 0101
    {
        Image_Flag.Cross_Fill = 2;
        //计算斜率
        start = Break_Num_L_UP - 15;
        start = Limit_a_b(start, 5, Image_H - 5);
        end = Break_Num_L_UP;
        end = Limit_a_b(end, 5, Image_H - 5);
        Get_K_b(L_Border[start], start, L_Border[end], end, &slope_l_rate, &intercept_l);
        for (i = Break_Num_L_UP; i < Image_H - 2; i++)
        {
            L_Border[i] = (i - intercept_l) / slope_l_rate;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
        }

        //计算斜率
        start = Break_Num_R_UP - 15;//起点
        start = Limit_a_b(start, 5, Image_H - 5);//限幅
        end = Break_Num_R_UP;//终点
        end = Limit_a_b(end, 5, Image_H - 5);
        Get_K_b(R_Border[start], start, R_Border[end], end, &slope_l_rate, &intercept_l);
        for (i = Break_Num_R_UP; i < Image_H - 2; i++)
        {
            R_Border[i] = (i - intercept_l) / slope_l_rate;//y = kx+b
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);
        }
        // printf("找到十字\r\n");
    }
    else
    {
        Image_Flag.Cross_Fill = false;
    }
 }


/**@brief    判断是否为直线
-- @param    uint8 Border 需要判断的线
-- @param    uint16 Total_Num 数量
-- @return   是否为直线
-- @author   戴骐阳
-- @date     2024/4/30
**/
bool Straight_Line_Judge(uint8* Border, uint16 Total_Num, lineTypeDef lineMode)
{
    uint8 StraightPoint = 0; // 记录是直线的点的个数
    // 如果数组长度为0或1，直接返回1
    if (Total_Num <= 1) {
        return true;
    }
    /*
    *
                       \
                        \
                         \
                          \
                           \
     */

     // 遍历数组，检查是否单调递减
    for (int i = Image_H - 2; i > Hightest; i -= 1) {
        // 如果当前元素小于或等于前一个元素，则不是单调递减的
        // uint8 before = Limit_a_b(i - 1, Hightest, Image_H - 2);
        // uint8 next = Limit_a_b(i - 1, Hightest, Image_H - 2);
       // printf("当前: % d 后一个点 : % d \n", Border[i], Border[i - 1]);
        if (lineMode == RightLine)
        {
            if ((Border[i] - Border[i - 1] >= 0) 
            && (Border[i] - Border[i - 1] < 2) 
            && (Border[i] < 144) 
            && (Border[i - 1] < 144) 
            && (Border[i] > Image_W/2) 
            && (Border[i - 1] > Image_W/2))
            {
                StraightPoint += 1;
                //circle(frame, Point(Border[i], i), 0, Scalar(255, 0, 0), 1);
            }
            // 断裂点直接清零
            else if (abs(Border[i] - Border[i - 1]) > 5)
            {
                // circle(frame, Point(Border[i], i), 0, Scalar(0, 0, 255), 1);
                StraightPoint = 0;
                //break;
            }

            if (StraightPoint >= 60)
            {
                // printf("右直线\r\n");
                return true;
            }
        }
        if (lineMode == LeftLine)
        {
            if ((Border[i] - Border[i - 1] <= 0 ) 
            && (Border[i] - Border[i - 1] > -2) 
            && (Border[i] > 4) 
            && (Border[i - 1] > 4)
            && (Border[i] < Image_W/2)
            && (Border[i-1] < Image_W/2))
            {
                StraightPoint += 1;
                //circle(resizeFrame, Point(Border[i], i), 0, Scalar(255, 0, 0), 1);
            }
            // 断裂点直接清零
            else if (abs(Border[i] - Border[i - 1]) > 5)
            {
                //circle(resizeFrame, Point(Border[i], i), 0, Scalar(0, 0, 255), 1);
                StraightPoint = 0;
                //break;
            }
            if (StraightPoint >= 60)
            {
                // printf("左直线\r\n");
                return true;
            }
        }
    }
    // printf("Line: %d \n", StraightPoint);
    return false;
}

/**@brief    寻找圆环凸点
-- @param    uint8 Border 需要判断的线
-- @return   坐标
-- @author   戴骐阳
-- @date     2024/4/30
**/
uint16 findCircleOutPoint(uint8* L_Border)
{
    // 寻找圆环右顶点
    for (uint8 i = 8; i < Image_H - 8; i++)
    {
        uint8 before = Limit_a_b(i - 5, 0, Image_H - 2);
        uint8 next = Limit_a_b(i + 5, 0, Image_H - 2);
        if ((L_Border[i] - L_Border[before] > 0) 
            && (L_Border[i] - L_Border[next] > 0)
            && (L_Border[i] - L_Border[before] < 10) 
            && (L_Border[i] - L_Border[next] < 10)
            && (L_Border[i] > Border_Min + 5) 
            && (L_Border[before] > Border_Min + 5) 
            && (L_Border[next] > Border_Min + 5) 
            && (i < Image_H - 20))
        {
            return i;
        }
    }
    return NULL;
}

/**
* @brief 左圆环元素处理
* @param uint8(*Bin_Image)[Image_W]		输入二值图像
* @param uint8 *L_Border			输入左边界首地址
* @param uint8 *R_Border			输入右边界首地址
* @param uint16_t Total_Num_L			输入左边循环总次数
* @param uint16_t Total_Num_R			输入右边循环总次数
* @param uint16_t *Dir_L				输入左边生长方向首地址
* @param uint16_t *Dir_R				输入右边生长方向首地址
* @param uint16_t(*Points_L)[2]		输入左边轮廓首地址
* @param uint16_t(*Points_R)[2]		输入右边轮廓首地址
* @author 戴骐阳、庄文标
* @date  2024/4/30
* @see CTest		Left_Ring(Bin_Image,L_Border, R_Border, Data_Statics_L, Data_Statics_R, Dir_L, Dir_R, Points_L, Points_R);
 */
void Left_Ring(uint8(*Bin_Image)[Image_W], uint8* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R,
    uint16* Dir_L, uint16* Dir_R, uint16(*Points_L)[2], uint16(*Points_R)[2])
{
    int i;
    static uint8 WhitePixel_Time = 0;
    uint8 hashKey[Image_H] = { 0 };
    uint16 offset = 0;
    uint8 StraightNum = 0;
    uint8 circlePoint[2];
    uint16 Break_Num_L_UP = 0;//拐点
    uint16 Break_Num_L_DOWN = 0;
    uint16 Salient_Point = 0;//圆环凸点
    uint16 Break_Num_R_UP = 0;
    uint16 start, end;
    float slope_l_rate = 0, intercept_l = 0;
    static float Curanglg = 0;
    static float Lastanglg = 0;
    static float Angle_Offest = 0;
    uint8 Lose_Line_Point_L = 0; 
    static uint16 Lose_Salient_Point_Count = 0;
    static uint16 Salient_Point_Count = 0;
    static uint16 Break_Num_L_DOWN_Count = 0;

    switch (LeftRing.Ring_State)
    {
        case Ring_Front:
            // printf("入环前\r\n");
            //cout << "入环前" << endl;
            LeftRing.Stright_Line = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//判断右边是否为长直线
            Salient_Point = findCircleOutPoint(L_Border);
            if(LeftRing.Stright_Line)
            {
                for (uint8 i = Image_H - 2; i > 10; --i)
                {
                    // 断裂区域横坐标相差很大
                    if (L_Border[i] - L_Border[i - 1] > 10)
                    {
                        hashKey[offset] = i;
                        offset++;
                        break;
                    }
                }
            }

            if (offset > 0)
            {
                for (uint16 i = 0;i < offset; i++)
                {
                    uint16 edgeIndex = hashMapIndexL[hashKey[i]];
                    uint16 start = Limit_a_b((edgeIndex - 7), 0, Data_Stastics_L);
                    uint16 end = Limit_a_b((edgeIndex + 7), 0, Data_Stastics_L);
                    for (uint16 j = start; j < end; j++)
                    {
                        uint16 before = Limit_a_b(j - 5, start, end);
                        uint16 next = Limit_a_b(j + 5, start, end);
                        if (Points_L[j][1] < Points_L[before][1] && Points_L[j][1] < Points_L[next][1]
                            && Points_L[j][0] < Points_L[before][0] && Points_L[j][0] > Points_L[next][0] && Points_L[j][0] > Border_Min + 5)
                        {
                            Break_Num_L_DOWN = j;
                            break;
                        }
                    }
                }
            }

            if((LeftRing.Stright_Line) && (Bin_Image[Image_H - 5][2]) && (Image_Flag.Lose_Line_L))
            {
                for (uint16 i = Image_H/2; i <= Image_H - 2; i += 1) 
                {
                    if (L_Border[i] <= 2)
                    {
                        Lose_Line_Point_L += 1;
                    }
                }

                if(Lose_Line_Point_L >= 30)
                {
                    LeftRing.Ring_Front_Flag = 2;
                }
            } 
            else
            {
                Lose_Line_Point_L = 0;
            }

            if(Salient_Point)
            {
                Salient_Point_Count++;
            }
            else
            {
                Salient_Point_Count = 0;
            }

            //有圆环突出点, 右直线, 有左下角点
            if ((Break_Num_L_DOWN) && (LeftRing.Stright_Line) && (Salient_Point_Count>=10)) //  && (!Bin_Image[Image_H - 10][4]) && (!Bin_Image[Image_H - 10][Image_W - 4])
            {
                //计算斜率,左边斜率
                Get_K_b(Salient_Point, L_Border[Salient_Point], Points_L[Break_Num_L_DOWN][1], Points_L[Break_Num_L_DOWN][0], &slope_l_rate, &intercept_l);
                for (i = Salient_Point; i < Points_L[Break_Num_L_DOWN][1]; i++)
                {
                    L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
                }
                LeftRing.Ring_Front_Flag = 1;
                // Set_Beeptime(200);
            }
            else if ((Break_Num_L_DOWN) && (LeftRing.Stright_Line))// 没有圆环突出点, 右直线， 有左下角点
            {
                Get_K_b(Points_L[Break_Num_L_DOWN][1], Points_L[Break_Num_L_DOWN][0], 2, Image_W/2 - 20, &slope_l_rate, &intercept_l);
                for (i = Points_L[Break_Num_L_DOWN][1]; i >= 1; --i)
                {
                    L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
                }
                LeftRing.Ring_Front_Flag = 2;
            }

            if((LeftRing.Ring_Front_Flag == 2) && (Salient_Point_Count>=10))
            {
                LeftRing.Ring_Front_Flag = 1;
                // Set_Beeptime(500);
            }

            // 检测到环左下角点,左下空白,左圆环突出点,右直线
            if ((LeftRing.Ring_Front_Flag == 1) &&
                (Bin_Image[Image_H - 5][4]) && 
                (Salient_Point_Count>=10) && 
                (LeftRing.Stright_Line))// (Bin_Image[Image_H - 5][4]) && (Bin_Image[Image_H - 10][8]) &&
            {
                Salient_Point_Count = 0;
                LeftRing.Ring_Front_Flag = false;
                Image_Flag.Left_Ring = true;
                LeftRing.Ring_State = Enter_Ring_First;//入环中 Enter_Ring_First
                LeftRing.Clear_Time = 0;
                Lose_Line_Point_L = 0;
            }
            //imshow("原图像", frame);
            // waitKey(200);
        break;
        case Enter_Ring_First:
    #ifdef Left_Ring_debug
            printf("左入环中\r\n");
    #endif
            LeftRing.Stright_Line = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//判断右边是否为长直线
            Salient_Point = findCircleOutPoint(L_Border);//Find_Salient_Point(L_Border, Image_H / 2 - 10);
            for (i = 1; i < Total_Num_L; i++)
            {
                uint16 before = Limit_a_b(i - 5, 0, Total_Num_L);
                uint16 next = Limit_a_b(i + 5, 0, Total_Num_L);
                if (before == Total_Num_L || next == Total_Num_L)
                {
                    break;
                }
                if ((Points_L[i][1] > Points_L[before][1]) && (Points_L[i][1] > Points_L[next][1])
                    && (Points_L[i][0] > Points_L[before][0]) && (Points_L[i][0] < Points_L[next][0]) && (Points_L[i][0] > Border_Min + 5))
                {
                    Break_Num_L_UP = i;//传递y坐标
                    break;
                }
            }
            if (Salient_Point) // 判断是否有圆环突出点并且右侧直线
            {
                Get_K_b(L_Border[Salient_Point], Salient_Point, 2, Image_H - 2, &slope_l_rate, &intercept_l);
                for (i = Image_H - 1; i > 1; i--)
                {
                    //L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    L_Border[i] = (i - intercept_l)/ slope_l_rate;
                    L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
                }
                LeftRing.Enter_Ring_First_Flag = true;
            }

            for (int i = Image_H/2+20; i > Image_H/2-20; i -= 1) 
            {
                if (L_Border[i] < 4)
                {
                    Lose_Line_Point_L += 1;
                }
            }

            if(Lose_Line_Point_L>30 && (LeftRing.Stright_Line))
            {
                LeftRing.Lose_Line = true;
            }

            if(Salient_Point == NULL)
            {
                Lose_Salient_Point_Count+=1;
            }
            else
            {
                Lose_Salient_Point_Count=0;
            }
            // 第一次入环标志位, 找到上角点, 找不到圆环突出点, 右直线
            if ((LeftRing.Enter_Ring_First_Flag) && (Lose_Salient_Point_Count>=10) && (Break_Num_L_UP || LeftRing.Lose_Line))    //&& (Bin_Image[20][4]) && (Bin_Image[20][8]) && (Bin_Image[Image_H - 2][4]) && (Bin_Image[Image_H - 2][8])
            {
                Lose_Salient_Point_Count = 0;
                LeftRing.Lose_Line = false;
                LeftRing.Enter_Ring_First_Flag = false;
                LeftRing.Ring_State = Leave_Ring_First;//离开环
                Lastanglg = 0;
                Angle_Offest = 0;
            }
        break;
        case Leave_Ring_First:
    #ifdef Left_Ring_debug
            printf("左第一次离开环\r\n");
    #endif
            Curanglg = Gyro_YawAngle_Get();
            if(Lastanglg == 0)
            {
                Lastanglg = Curanglg;
            }
            Angle_Offest += Curanglg - Lastanglg;
            LeftRing.Stright_Line = false;
            Salient_Point = 0;
            LeftRing.Ring_Front_Flag = 0;
            // cout << "第一次离开环" << endl;
            for (i = 1; i < Total_Num_L; i++)
            {
                uint16 before = Limit_a_b(i - 5, 0, Total_Num_L);
                uint16 next = Limit_a_b(i + 5, 0, Total_Num_L);
                if (before == Total_Num_L || next == Total_Num_L)
                {
                    break;
                }
                if (Points_L[i][1] > Points_L[before][1] && Points_L[i][1] > Points_L[next][1]
                    && Points_L[i][0] > Points_L[before][0] && Points_L[i][0] < Points_L[next][0] && Points_L[i][0] > Border_Min + 5)
                {
                    Break_Num_L_UP = i;//传递y坐标
                    break;
                }
            }

            if (Break_Num_L_UP)
            {
                Get_K_b(Points_L[Break_Num_L_UP][1], Points_L[Break_Num_L_UP][0], Image_H - 2, 2, &slope_l_rate, &intercept_l);
                for (i = Points_L[Break_Num_L_UP][1]; i <= Image_H - 2; ++i)
                {
                    L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
                }
            }
            Lastanglg = Curanglg;
            // 左下一片空白, 找不到左上角点
            // if (Bin_Image[Image_H - 10][20] && Bin_Image[Image_H - 10][25] && !Break_Num_L_UP)
            // {
            //     LeftRing.Leave_Ring_First_Flag = true;
            //     LeftRing.Ring_State = In_Ring;
            //     Lastanglg = 0;
            //     Angle_Offest = 0;
            // }
        break;
        case In_Ring:
    #ifdef Left_Ring_debug
            printf("左环内\r\n");
    #endif
            // printf("%f,%f,%f\r\n",Curanglg,Lastanglg,Angle_Offest);
            for (i = 1; i < Total_Num_R; i++)
            {
                uint16 before = Limit_a_b(i - 8, 0, Total_Num_R);
                uint16 next = Limit_a_b(i + 8, 0, Total_Num_R);
                if (before == Total_Num_R || next == Total_Num_R)
                {
                    break;
                }
                if (Points_R[i][1] < Points_R[before][1] && Points_R[i][1] > Points_R[next][1]
                    && Points_R[i][0] < Points_R[before][0] && Points_R[i][0] < Points_R[next][0] && Points_R[i][0] < Border_Max - 5)
                {
                    Break_Num_R_UP = i;//传递y坐标
                    break;
                }
            }
            if (Break_Num_R_UP) // && (Bin_Image[50][4]) && (Bin_Image[50][8])
            {
                Curanglg = Gyro_YawAngle_Get();
                if(Lastanglg == 0)
                {
                    Lastanglg = Curanglg;
                }
                Angle_Offest += Curanglg - Lastanglg;
                Get_K_b(Points_R[Break_Num_R_UP][0], Points_R[Break_Num_R_UP][1], 2, 2, &slope_l_rate, &intercept_l);
                for (i = Image_H - 1; i > 1; i--)
                {
                    L_Border[i] = Border_Min;
                    R_Border[i] = ((i)-intercept_l) / slope_l_rate;//y = kx+b
                    R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
                }
            }
            Lastanglg = Curanglg;
            // 左右两侧都为白色, 准备要出环
            if (Bin_Image[Image_H - 4][4] == White_Pixel && Bin_Image[Image_H - 4][Image_W - 4] == White_Pixel && fabs(Angle_Offest) >= 25)
            {
                LeftRing.Ring_State = Ready_Out_Ring;
                Angle_Offest = 0;
                Lastanglg = 0;
            }
        break;
        case Ready_Out_Ring:
    #ifdef Left_Ring_debug
            printf("左准备出环\r\n");
    #endif
            Curanglg = Gyro_YawAngle_Get();
            if(Lastanglg == 0)
            {
                Lastanglg = Curanglg;
            }
            Angle_Offest += Curanglg - Lastanglg;
            // printf("%f,%f,%f\r\n",Curanglg,Lastanglg,Angle_Offest);
            Get_K_b(Image_W, Image_H, 2, Image_H - 20, &slope_l_rate, &intercept_l);
            for (i = Image_H - 1; i > 1; i--)
            {
                L_Border[i] = Border_Min;
                R_Border[i] = ((i)-intercept_l) / slope_l_rate;//y = kx+b
                R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
            }
            Lastanglg = Curanglg;
            // 右侧生长线长于左侧生长线则差不多直线, 准备出环
            if (Data_Stastics_R > Data_Stastics_L && fabs(Angle_Offest) > 20)
            {
                LeftRing.Ring_State = Leave_Ring;
                Angle_Offest = 0;
                Lastanglg = 0;
            }
        
        break;
        case Leave_Ring:
    #ifdef Left_Ring_debug
            printf("左出环\r\n");
    #endif
            Break_Num_L_UP = 0;
            for (i = 1; i < Total_Num_L; i++)
            {
                uint16 before = Limit_a_b(i - 5, 0, Total_Num_L);
                uint16 next = Limit_a_b(i + 5, 0, Total_Num_L);
                if (before == Total_Num_L || next == Total_Num_L)
                {
                    break;
                }
                if ((Points_L[i][1] > Points_L[before][1]) && (Points_L[i][1] > Points_L[next][1])
                    && (Points_L[i][0] > Points_L[before][0]) && (Points_L[i][0] < Points_L[next][0]) && (Points_L[i][0] > Border_Min + 5))
                {
                    Break_Num_L_UP = i;//传递y坐标
                    break;
                }
            }

            for (int i = Image_H/2+20; i > Image_H/2-20; i -= 1) 
            {
                if (L_Border[i] <= 2)
                {
                    Lose_Line_Point_L += 1;
                }
            }

            if(Lose_Line_Point_L>10)
            {
                LeftRing.Lose_Line = true;
            }
            else
            {
                LeftRing.Lose_Line = false;
            }

            if (Break_Num_L_UP)
            {
                Get_K_b(Points_L[Break_Num_L_UP][1], Points_L[Break_Num_L_UP][0], Image_H - 2, 2, &slope_l_rate, &intercept_l);
                for (i = Points_L[Break_Num_L_UP][1]; i <= Image_H - 2; ++i)
                {
                    L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
                }
            }
            // 检测到已经看见过环的标志位, 右下角为黑色, 并且看不到上角点, 代表已经出环
            else if ((Bin_Image[Image_H - 4][4] == Black_Pixel) && (Bin_Image[Image_H - 2][Image_W - 4] == Black_Pixel )
                    && (!Break_Num_L_UP))
            {
                LeftRing.Ring_Front_Flag = 0; // 出环
                LeftRing.Enter_Ring_First_Flag = false;
                LeftRing.Leave_Ring_First_Flag = false;
                LeftRing.Stright_Line = false;
                Image_Flag.Left_Ring = false;
                LeftRing.Ring_State = Ring_Front;
            }
        // waitKey(200);
        break;
    }
}

/**@brief    寻找圆环凸点
-- @param    uint8 Border 需要判断的线
-- @return   坐标
-- @author   戴骐阳
-- @date     2024/5/10
**/
uint16_t findCircleOutPoint_R(uint8* R_Border)
{
    // 寻找圆环右顶点
    for (uint8 i = 8; i < Image_H - 8; i++)
    {
        uint8 before = Limit_a_b(i - 5, 0, Image_H - 2);
        uint8 next = Limit_a_b(i + 5, 0, Image_H - 2);
        //circle(resizeFrame, Point(R_Border[i], i), 0, Scalar(255, 255, 0), 1);
        //circle(resizeFrame, Point(R_Border[before], before), 0, Scalar(0, 255, 0), 1);
        //circle(resizeFrame, Point(R_Border[next], next), 0, Scalar(0, 0, 255), 1);
        //printf("now - before: %d now - next: %d \n", R_Border[i] - R_Border[before], R_Border[i] - R_Border[next]);
        if ((R_Border[i] - R_Border[before] < 0) 
            && (R_Border[i] - R_Border[next] < 0)//原-1
            && (R_Border[i] - R_Border[before] > -10) 
            && (R_Border[i] - R_Border[next] > -10)
            && R_Border[i] < Border_Max - 5 
            && R_Border[before] < Border_Max - 5 
            && R_Border[next] < Border_Max - 5 
            && i < Image_H - 20)
        {
            // printf("找到突出点\r\n");
            return i;
        }
        //imshow("原图像", resizeFrame);
        //waitKey(50);
    }
    // printf("没找到突出点\r\n");
    return NULL;
}

/**
* @brief 右圆环元素处理
* @param uint8(*Bin_Image)[Image_W]		输入二值图像
* @param uint8 *L_Border			输入左边界首地址
* @param uint8 *R_Border			输入右边界首地址
* @param uint16_t Total_Num_L			输入左边循环总次数
* @param uint16_t Total_Num_R			输入右边循环总次数
* @param uint16_t *Dir_L				输入左边生长方向首地址
* @param uint16_t *Dir_R				输入右边生长方向首地址
* @param uint16_t(*Points_L)[2]		输入左边轮廓首地址
* @param uint16_t(*Points_R)[2]		输入右边轮廓首地址
* @author 戴骐阳、庄文标
* @date  2024/5/10
* @see CTest		Right_Ring(Bin_Image,L_Border, R_Border, Data_Statics_L, Data_Statics_R, Dir_L, Dir_R, Points_L, Points_R);
 */
void Right_Ring(uint8(*Bin_Image)[Image_W], uint8* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R,
    uint16* Dir_L, uint16* Dir_R, uint16(*Points_L)[2], uint16(*Points_R)[2])
{
    int i;
    static uint8 WhitePixel_Time = 0;
    uint8 hashKey[Image_H] = { 0 };
    uint16 offset = 0;
    uint8 StraightNum = 0;
    uint8 circlePoint[2];
    uint16 Break_Num_L_UP = 0;//拐点
    uint16 Break_Num_R_UP = 0;//拐点
    uint16 Break_Num_R_DOWN = 0;
    uint16 Salient_Point = 0;//圆环凸点
    uint16 start, end;
    float slope_l_rate = 0, intercept_l = 0;
    static float Curanglg = 0;
    static float Lastanglg = 0;
    static float Angle_Offest = 0;
    uint16 Lose_Line_Point_R = 0;
    static uint16 Salient_Point_Count = 0;
    static uint16 Lose_Salient_Point_Count = 0;
    static uint16 Break_Num_R_DOWN_Count = 0;
    switch (RightRing.Ring_State)
    {
        case Ring_Front:
            RightRing.Stright_Line = Straight_Line_Judge(L_Border, Total_Num_L - 10, LeftLine);
            Salient_Point = findCircleOutPoint_R(R_Border);
            if(RightRing.Stright_Line)
            {
                for (uint8_t i = Image_H - 2; i > 10; --i)
                {
                    if (R_Border[i] - R_Border[i - 1] < -10)
                    {
                        hashKey[offset] = i;
                        offset++;
                        break;
                    }
                }
            }

            if (offset > 0)
            {
                for (uint16_t i = 0;i < offset; i++)
                {
                    uint16_t edgeIndex = hashMapIndexR[hashKey[i]];
                    uint16_t start = Limit_a_b((edgeIndex - 7), 0, Data_Stastics_R); // -2
                    uint16_t end = Limit_a_b((edgeIndex + 7), 0, Data_Stastics_R);
                    for (uint16_t j = start; j < end; j++)
                    {
                        uint16_t before = Limit_a_b(j - 5, start, end);  // -10
                        uint16_t next = Limit_a_b(j + 5, start, end);    // 10
                        if ((Points_R[j][1] < Points_R[before][1]) 
                            && Points_R[j][0] < Points_R[before][0] 
                            && Points_R[j][0] < Points_R[next][0] 
                            && Points_R[j][0] < Border_Max - 5)
                        {
                            Break_Num_R_DOWN = j;
                            // printf("找到拐点\r\n");
                            break;
                        }
                    }
                }
            }

            if((RightRing.Stright_Line) && (Bin_Image[Image_H - 5][Image_W - 5]) && (Image_Flag.Lose_Line_R))
            {
                for (uint16 i = Image_H/2; i <= Image_H - 2; i += 1)
                {
                    if (R_Border[i] >= 144)
                    {
                        Lose_Line_Point_R += 1;
                    }
                }

                if(Lose_Line_Point_R>=30)
                {
                    RightRing.Ring_Front_Flag = 2;
                    // printf("右边丢线\r\n");
                }
            }
            else
            {
                LeftRing.Ring_Front_Flag = 0;
            }

            if(Salient_Point)
            {
                Salient_Point_Count++;
            }
            else
            {
                Salient_Point_Count = 0;
            }
            //下拐点，左直线，圆环突出点
            if (Break_Num_R_DOWN && RightRing.Stright_Line && Salient_Point_Count>=10) //  && (!Bin_Image[Image_H - 10][4]) && (!Bin_Image[Image_H - 10][Image_W - 4])
            {
                //计算斜率,左边斜率
                Get_K_b(Salient_Point, L_Border[Salient_Point], Points_R[Break_Num_R_DOWN][1], Points_R[Break_Num_R_DOWN][0], &slope_l_rate, &intercept_l);
                for (i = Salient_Point; i < Points_R[Break_Num_R_DOWN][1]; i++)
                {
                    R_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
                }
                RightRing.Ring_Front_Flag = 1;
                // Set_Beeptime(200);
            }
            else if (Break_Num_R_DOWN && RightRing.Stright_Line)//下拐点，左直线
            {
                // end = Limit_a_b(R_Border[i], Break_Num_R_DOWN + 15, Image_H - 2);//限幅
                Get_K_b(Points_R[Break_Num_R_DOWN][1], Points_R[Break_Num_R_DOWN][0], 2, Image_W/2+20, &slope_l_rate, &intercept_l);
                for (i = Points_R[Break_Num_R_DOWN][1]; i >= 1; --i)
                {
                    R_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
                }
                RightRing.Ring_Front_Flag = 2;
            }

            if((RightRing.Ring_Front_Flag == 2) && Salient_Point_Count>=10)
            {
                RightRing.Ring_Front_Flag = 1;
                // Set_Beeptime(500);
            }

            if(RightRing.Ring_Front_Flag && RightRing.Clear_Time == 0)
            {
                RightRing.Clear_Time = 1;
            }

            if(RightRing.Ring_Front_Flag && RightRing.Clear_Time >=5000)
            {
                RightRing.Ring_Front_Flag = 0;
                RightRing.Clear_Time = 0;
            }
            if ((RightRing.Ring_Front_Flag == 1) 
                && (Bin_Image[Image_H - 5][Image_W - 4])
                && (Salient_Point_Count>=10)
                && (RightRing.Stright_Line))
            {
                RightRing.Ring_Front_Flag = 0;
                Salient_Point_Count=0;
                Image_Flag.Right_Ring = true;
                RightRing.Ring_State = Enter_Ring_First;
                RightRing.Clear_Time = 0;
            }
        break;
        case Enter_Ring_First:
    #ifdef Right_Ring_debug
            printf("右初入环\r\n");
    #endif
            RightRing.Stright_Line = Straight_Line_Judge(L_Border, Total_Num_L - 10, LeftLine);
            Salient_Point = findCircleOutPoint_R(R_Border);
            // tft180_Draw_ColorCircle(R_Border[Salient_Point],Salient_Point,8,RGB565_RED);
            for (i = 1; i < Total_Num_R; i++)
            {
                uint16 before = Limit_a_b(i - 5, 0, Total_Num_R);
                uint16 next = Limit_a_b(i + 5, 0, Total_Num_R);
                if (before == Total_Num_R || next == Total_Num_R)
                {
                    break;
                }
                if (Points_R[i][1] > Points_R[before][1] && Points_R[i][1] > Points_R[next][1]
                    && Points_R[i][0] < Points_R[before][0] && Points_R[i][0] > Points_R[next][0] && Points_R[i][0] < Border_Max - 5)
                {
                    Break_Num_R_UP = i;//传递y坐标
                    //circle(resizeFrame, Point(Points_R[i][0], Points_R[i][1]), 3, Scalar(0, 255, 255));
                    break;
                }
            }

            if (Salient_Point) // 判断是否有圆环突出点并且左侧直线
            {
                // line(resizeFrame, Point(R_Border[Salient_Point], Salient_Point), Point(Image_W, Image_H - 1), Scalar(255, 0, 0));
                Get_K_b(Salient_Point, R_Border[Salient_Point], Image_H - 2, Image_W - 2, &slope_l_rate, &intercept_l);
                for (i = Image_H - 1; i > 1; i--)
                {
                    R_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
                }
                RightRing.Enter_Ring_First_Flag = true;
            }

            for (int i = Image_H - 2; i > 2; i -= 1) 
            {
                if (R_Border[i] >= 144)
                {
                    Lose_Line_Point_R += 1;
                }
            }

            if(Lose_Line_Point_R>=30&&RightRing.Stright_Line)
            {
                RightRing.Lose_Line = true;
            }
            
            if(Salient_Point == NULL)
            {
                Lose_Salient_Point_Count+=1;
            }
            else
            {
                Lose_Salient_Point_Count = 0;
            }

            if (RightRing.Enter_Ring_First_Flag && Lose_Salient_Point_Count>=15 && (Break_Num_L_UP || RightRing.Lose_Line))
            {
                Lose_Salient_Point_Count = 0;
                RightRing.Lose_Line = false;
                RightRing.Enter_Ring_First_Flag = false;
                RightRing.Ring_State = Leave_Ring_First;
                Lastanglg = 0;
                Angle_Offest = 0;
            }
        break;
        case Leave_Ring_First:
    #ifdef Right_Ring_debug
        printf("右第一次出环\r\n");
    #endif
            RightRing.Stright_Line = false;
            Salient_Point = 0;
            RightRing.Ring_Front_Flag = 0;

            Curanglg = Gyro_YawAngle_Get();
            if(Lastanglg == 0)
            {
                Lastanglg = Curanglg;
            }
            Angle_Offest += Curanglg - Lastanglg;
            for (i = Total_Num_R - 5; i > 1; --i)
            {
                uint16 before = Limit_a_b(i - 7, 0, Total_Num_R);
                uint16 next = Limit_a_b(i + 7, 0, Total_Num_R);
                if (before == 1 || next == 1)
                {
                    break;
                }
                if (Points_R[i][1] > Points_R[before][1] && Points_R[i][1] > Points_R[next][1]
                    && Points_R[i][0] < Points_R[before][0] && Points_R[i][0] > Points_R[next][0] && Points_R[i][0] < Border_Max - 5)
                {
                    //circle(resizeFrame, Point(Points_R[i][0], Points_R[i][1]), 3, Scalar(0, 255, 255));
                    Break_Num_R_UP = i;//传递y坐标
                    break;
                }
                //imshow("原图像", resizeFrame);
                //waitKey(50);
            }
            if (Break_Num_R_UP)
            {
                Get_K_b(Points_R[Break_Num_R_UP][1], Points_R[Break_Num_R_UP][0], Image_H - 2, Image_W - 2, &slope_l_rate, &intercept_l);
                for (i = Points_R[Break_Num_R_UP][1]; i <= Image_H - 1; ++i)
                {
                    R_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                    R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
                }
            }

            Lastanglg = Curanglg;
            // if ((Bin_Image[Image_H - 10][Image_W - 8]) && (Bin_Image[Image_H - 10][Image_W - 5]) && (!Break_Num_R_UP) && (fabs(Angle_Offest) >= 50))
            // {
            //     RightRing.Leave_Ring_First_Flag = true;
            //     RightRing.Ring_State = In_Ring;
            //     Lastanglg = 0;
            //     Angle_Offest = 0;
            // }
        break;
        case In_Ring:
    #ifdef Right_Ring_debug
            printf("右环内\r\n");
    #endif
            for (i = 1; i < Total_Num_L; i++)
            {
                uint16 before = Limit_a_b(i - 8, 0, Total_Num_L);
                uint16 next = Limit_a_b(i + 8, 0, Total_Num_L);
                if (before == Total_Num_L || next == Total_Num_L)
                {
                    break;
                }
                if (Points_L[i][1] < Points_L[before][1] && Points_L[i][1] > Points_L[next][1]
                    && Points_L[i][0] > Points_L[before][0] && Points_L[i][0] > Points_L[next][0] && Points_L[i][0] > Border_Min + 5)
                {
                    Break_Num_L_UP = i;//传递y坐标
                    break;
                }
            }
            // 狠补线
            if (Break_Num_L_UP) // && (Bin_Image[50][4]) && (Bin_Image[50][8])
            {
                Curanglg = Gyro_YawAngle_Get();
                if (Lastanglg == 0)
                {
                    Lastanglg = Curanglg;
                }
                Angle_Offest += Curanglg - Lastanglg;
                Get_K_b(Points_L[Break_Num_L_UP][0], Points_L[Break_Num_L_UP][1], Image_W - 2, 2, &slope_l_rate, &intercept_l);
                for (i = Image_H - 1; i > 1; i--)
                {
                    L_Border[i] = (i-intercept_l) / slope_l_rate;
                    L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
                }
            }
            Lastanglg = Curanglg;
            // 左右两侧都为白色, 准备要出环
            if (Bin_Image[Image_H - 4][4] == White_Pixel && Bin_Image[Image_H - 4][Image_W - 4] == White_Pixel && (fabs(Angle_Offest) >= 25))
            {
                RightRing.Ring_State = Ready_Out_Ring;
                Angle_Offest = 0;
                Lastanglg = 0;
            }
            break;
            case Ready_Out_Ring:
        #ifdef Right_Ring_debug
                printf("右准备出环\r\n");
        #endif
                Curanglg = Gyro_YawAngle_Get();
                if (Lastanglg == 0)
                {
                   Lastanglg = Curanglg;
                }
                Angle_Offest += Curanglg - Lastanglg;
                // printf("%f,%f,%f\r\n",Curanglg,Lastanglg,Angle_Offest);
                Get_K_b(2, Image_H, Image_W, Image_H - 20, &slope_l_rate, &intercept_l);
                for (i = Image_H - 1; i > 1; i--)
                {
                    R_Border[i] = Border_Max;
                    L_Border[i] = ((i)-intercept_l) / slope_l_rate;//y = kx+b
                    L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
                }
                Lastanglg = Curanglg;
                // 右侧生长线长于左侧生长线则差不多直线, 准备出环
                if ((Data_Stastics_R < Data_Stastics_L) && (fabs(Angle_Offest) > 20))
                {
                    RightRing.Ring_State = Leave_Ring;
                    Angle_Offest = 0;
                    Lastanglg = 0;
                }
            break;
            case Leave_Ring:
        #ifdef Right_Ring_debug
                printf("右出环\r\n");
        #endif
                Break_Num_L_UP = 0;
                //cout << "出环中" << ;
                for (i = Total_Num_R - 5; i > 1; --i)
                {
                    uint16 before = Limit_a_b(i - 7, 0, Total_Num_R);
                    uint16 next = Limit_a_b(i + 7, 0, Total_Num_R);
                    if ((before == 1) || (next == 1))
                    {
                        break;
                    }
                    if ((Points_R[i][1] > Points_R[before][1]) && (Points_R[i][1] > Points_R[next][1])
                        && (Points_R[i][0] < Points_R[before][0]) && (Points_R[i][0] > Points_R[next][0]) && (Points_R[i][0] < Border_Max - 5))
                    {
                        Break_Num_R_UP = i;//传递y坐标
                        break;
                    }
                }

                for (int i = Image_H/2+20; i > Image_H/2-20; i -= 1) 
                {
                    if (R_Border[i] >= 144)
                    {
                        Lose_Line_Point_R += 1;
                    }
                }

                if(Lose_Line_Point_R>=10)
                {
                    RightRing.Lose_Line = true;
                }
                else
                {
                    RightRing.Lose_Line = false;
                }
                if (Break_Num_R_UP)
                {
                    Get_K_b(Points_R[Break_Num_R_UP][1], Points_R[Break_Num_R_UP][0], Image_H - 2, Image_W - 2, &slope_l_rate, &intercept_l);
                    for (i = Points_R[Break_Num_R_UP][1]; i <= Image_H - 1; ++i)
                    {
                        R_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                        R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
                    }
                }
                // 检测到已经看见过环的标志位, 右下角为黑色, 并且看不到上角点, 代表已经出环
                else if ((Bin_Image[Image_H - 4][4] == Black_Pixel) && (Bin_Image[Image_H - 2][Image_W - 4] == Black_Pixel)
                    && (!Break_Num_R_UP))
                {
                    RightRing.Ring_Front_Flag = false; // 出环
                    RightRing.Enter_Ring_First_Flag = false;
                    RightRing.Leave_Ring_First_Flag = false;
                    RightRing.Stright_Line = false;
                    Image_Flag.Right_Ring = false;
                    RightRing.Ring_State = Ring_Front;
                }
            // waitKey(200);
            break;
        } 

}

/**@brief   斑马线扫描
-- @param   uint8(*Bin_Image)[Image_W] 二值化图像
-- @author  庄文标
-- @date    2024/6/5
**/
void Zebra_Seek(uint8(*Bin_Image)[Image_W],uint8* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R)
{
    uint8 total = 0;
    uint16 Right_Straight = 0;
    uint16 Left_Straight = 0;
    Right_Straight = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//判断右边是否为长直线
    Left_Straight = Straight_Line_Judge(L_Border, Total_Num_L - 10, LeftLine);//判断左边是否为长直线

    if(Left_Straight || Right_Straight)
    {
        for(uint8 i = 50;i <= 120;i++)
        {
            if(Bin_Image[45][i] == Black_Pixel && Bin_Image[45][i+1] == White_Pixel)
            {
                total ++;
            }
        }
    }

    if((total >= 5))
    {
        total = 0;
        Image_Flag.Zerba = true;
    }
}

/**@brief   路障识别
-- @param   uint8(*Bin_Image)[Image_W] 二值化图像
-- @param   uint16* L_Border 左线数组
-- @param   uint16* L_Border 右线数组
-- @param   uint16 Total_Num_L 左线点的数量
-- @param   uint16 Total_Num_R 右线点的数量
-- @author  庄文标
-- @date    2024/7/4
**/
void Roadblock_Seek(uint8(*Bin_Image)[Image_W], uint8* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R)
{
    uint8 Right_Straight = 0;
    uint8 Left_Straight = 0;
    uint16 i = 0;
    uint16 Break_Num_L_UP = 0;//拐点xxz
    uint16 Break_Num_R_UP = 0;
    uint16 Break_Num_L_DOWN = 0;
    uint16 Break_Num_R_DOWN = 0;
    uint16 start, end;
    float slope_l_rate = 0, intercept_l = 0;

    Right_Straight = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//判断右边是否为长直线
    Left_Straight = Straight_Line_Judge(L_Border, Total_Num_L - 10, LeftLine);//判断左边是否为长直线
    if (Right_Straight)//先找一条直线
    {
        for (i = Image_H - 5; i > Image_H/2; i--)//寻找左下拐点
        {
            if (abs(L_Border[i + 1] - L_Border[i + 2] <= 5)
                && (abs(L_Border[i + 2] - L_Border[i + 3]) <= 5)
                && (abs(L_Border[i + 3] - L_Border[i + 4]) <= 5)
                && (L_Border[i] - L_Border[i + 2] >= 7))
            {
                Break_Num_L_DOWN = i;//传递y坐标
                break;
            }
        }

        if ((Break_Num_L_DOWN)
            && (!Bin_Image[Break_Num_L_DOWN - 10][L_Border[Break_Num_L_DOWN]])
            && (Lose_Line() == false))
        {
            Image_Flag.Roadblock = true;
            // printf("左障碍\r\n");
        }
        else if ((!Break_Num_L_DOWN)
            && (!Lose_Line())
            && (All_Stright()))
        {
            Image_Flag.Roadblock = false;
            // printf("不是障碍\r\n");
        }
    }
    else if (Left_Straight)
    {
        for (i = Image_H - 5; i > Image_H/2; i--)//寻找右下拐点
        {
            if (abs(R_Border[i + 1] - R_Border[i + 2] <= 5)
                && (abs(R_Border[i + 2] - R_Border[i + 3]) <= 5)
                && (abs(R_Border[i + 3] - R_Border[i + 4]) <= 5)
                && (R_Border[i + 2] - R_Border[i] >= 7))
            {
                Break_Num_R_DOWN = i;//传递y坐标
                break;
            }
        }
        if ((Break_Num_R_DOWN) 
            &&(!Bin_Image[Break_Num_R_DOWN - 10][R_Border[Break_Num_R_DOWN]])
            && (Lose_Line() == false))
        {
            // printf("右障碍\r\n");
            Image_Flag.Roadblock = true;
        }
        else if ((!Break_Num_R_DOWN)
            && (!Lose_Line())
            && (All_Stright()))
        {
            Image_Flag.Roadblock = false;
            // printf("不是右边障碍\r\n");
        }
    }
    else
    {
        Image_Flag.Roadblock = false;
    }
}

/**@brief   坡道识别
-- @param   无
-- @author  庄文标
-- @date    2024/7/13
**/
void Ramp_Find(void)
{
    if((dl1b_distance_mm < 550) 
    &&(dl1b_distance_mm > 80)
    &&(Gyro_PitchAngle_Get()>2)
    &&(Image_Flag.Ramp==0))
    {
        Image_Flag.Ramp = 1;//正在上坡
    }

    if(Image_Flag.Ramp == 1 && Gyro_PitchAngle_Get()<-5)
    {
        Image_Flag.Ramp = 2;//正在下坡
    }

    if(Image_Flag.Ramp == 2 && Gyro_PitchAngle_Get()>-2 && dl1b_distance_mm > 550)
    {
        Image_Flag.Ramp = 0;
    }
}

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   是否获取边线
-- @param   无
-- @author  庄文标
-- @date    2024/7/7
**/
bool No_Get_Line()
{
    static uint8 Start_Point = 0;
    if (Image_Flag.Get_Start_Point)
    {
        Start_Point+=1;
    }

    if((Start_Point >= Menu.Turn_Point))
    {
        Start_Point = 0;
        return false;
    }
    return true;
}

/**@brief   直道判断
-- @param   无
-- @return  是否为可以加速的直道
-- @author  庄文标
-- @date    2024/7/10
**/
bool All_Stright()
{
    if((Straight_Line_Judge(R_Border, Data_Stastics_R - 10, RightLine))
    &&(Straight_Line_Judge(L_Border, Data_Stastics_L - 10, LeftLine))
    &&(Hightest <= 2))
    {
        return true;
    }
    return false;
}

/**@brief   获取二值化图像
-- @param   无
-- @author  庄文标
-- @date    2024/7/7
**/
void Get_Bin_Image()
{
    memcpy(Original_Image, mt9v03x_image, sizeof(Original_Image));
    // timer_start(GPT_TIM_1);
    // my_sobel(Original_Image,Bin_Image);
    Turn_To_Bin();//二值化
    Image_Filter(Bin_Image);//滤波
    Image_Draw_Rectan(Bin_Image);//预处理
}

/**@brief   最终调用的图像处理的函数
-- @param   none
-- @author  庄文标、戴骐阳
-- @date    2023/10/3
**/
void Image_Process(void)
{
    static float Image_Erro_Y_ = 0;
    // timer_start(GPT_TIM_1);
    // my_sobel(Original_Image,Bin_Image);
    memcpy(Original_Image, mt9v03x_image, sizeof(Original_Image));    
    Turn_To_Bin();//二值化

    Image_Filter(Bin_Image);//滤波
    Image_Draw_Rectan(Bin_Image);//预处理

    //清零
    Data_Stastics_L = 0;
    Data_Stastics_R = 0;

    if (Get_Start_Point(Image_H - 2))//从图像的最下面开始找，找到起点了，再执行八领域，没找到就一直找
    {
        //        printf("正在开始八领域\n");
        Search_L_R((uint16)USE_num, Bin_Image, &Data_Stastics_L, &Data_Stastics_R, Start_Point_L[0], Start_Point_L[1], Start_Point_R[0], Start_Point_R[1], &Hightest);
        //        printf("八邻域已结束\n");
        // 从爬取的边界线内提取边线 ， 这个才是最终有用的边线
        Get_Left(Data_Stastics_L);
        Get_Right(Data_Stastics_R);

        if(MyFSM.CurState == Unload || MyFSM.CurState == Line_Patrol || MyFSM.Line_Board_State == Finsh_Return)
        {
            Zebra_Seek(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R);
        }

        // if((MyFSM.CurState == Line_Patrol)
        // &&(!Image_Flag.Right_Ring)
        // &&(!Image_Flag.Left_Ring)
        // &&(!Image_Flag.Cross_Fill))
        // {
        //     Roadblock_Seek(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R);
        // }

        // if(MyFSM.CurState == Line_Patrol)
        // {
        //     Ramp_Find();
        // }
        // 优先判断是否是十字如果是十字则不对圆环判断
        if((!Image_Flag.Right_Ring) 
        && (!Image_Flag.Left_Ring) 
        && (!Image_Flag.Zerba) 
        && (!Image_Flag.Roadblock) 
        && ((MyFSM.CurState == Line_Patrol) || (MyFSM.CurState == Cross_Board) || (MyFSM.Line_Board_State == Finsh_Return))
        && (!Image_Flag.Ramp))
        {
            Cross_Fill(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);//十字补线
        }

        // 同上
        // if ((!Image_Flag.Cross_Fill) 
        // && (!Image_Flag.Right_Ring) 
        // && (!Image_Flag.Zerba) 
        // && (!Image_Flag.Roadblock) 
        // && ((MyFSM.CurState == Line_Patrol) || (MyFSM.CurState == Ring_Board) || (MyFSM.Line_Board_State == Finsh_Return))
        // && (MyFSM.Ring_Board_State != Return_Line)
        // && (MyFSM.Ring_Board_State != Finsh_Return)
        // && (!Image_Flag.Ramp)
        // && (MyFSM.Ring_Flag == false)
        // && (MyFSM.Simple_Flag!=0))
        // {
        //     Left_Ring(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);
        // }

        if ((!Image_Flag.Cross_Fill) 
        && (!Image_Flag.Left_Ring) 
        && (!Image_Flag.Zerba) 
        && (!Image_Flag.Roadblock) 
        && ((MyFSM.CurState == Line_Patrol) || (MyFSM.CurState == Ring_Board) || (MyFSM.Line_Board_State == Finsh_Return))
        && (MyFSM.Ring_Board_State != Return_Line)
        && (MyFSM.Ring_Board_State != Finsh_Return)
        && (!Image_Flag.Ramp)
        && (MyFSM.Ring_Flag == false)
        && (MyFSM.Simple_Flag!=0))
        {
            Right_Ring(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);
        }
    }
    
    for (int i = Hightest; i < Image_H-1; i++)
    {
        Center_Line[i] = (L_Border[i] + R_Border[i]) >> 1;//求中线
    }
    Image_Erro = (Center_Line[69])*0.375f + (Center_Line[70])*0.5f + (Center_Line[71])*0.1f;
}