#include "Image.h"
#include <math.h>
#include "Gyro.h"

Ring_Handle LeftRing = { false, false, true };

typedef struct
{
    bool firstFindCircle;
    bool firstIntoCircle;
    bool firstFindCircleOut;
    bool readyIntoCircle;
    bool areInCircle;
    bool readyOutCircle;
    bool outCircle;
    bool secondSeeOutCircleOut;
    bool secondReadyOutCircle;
    bool OutCircle;
}circleLeftHandle;

circleLeftHandle LeftFlag = { false, false, false, false, false, false, false, false, false, false };
Flag_Handle Image_Flag = { false };//元素标志位
/* Define\Declare ------------------------------------------------------------*/
#define use_num     1   //1就是不压缩，2就是压缩一倍
uint8_t Original_Image[Image_H][Image_W];//原始图像数组
uint8_t Image_Thereshold;//图像分割阈值
uint8_t Bin_Image[Image_H][Image_W];//二值化图像数组
uint8_t Start_Point_L[2] = { 0 };//左边起点的x，y值
uint8_t Start_Point_R[2] = { 0 };//右边起点的x，y值

#define USE_num Image_H*3   //定义找点的数组成员个数按理说300个点能放下，但是有些特殊情况确实难顶，多定义了一点
//存放点的x，y坐标
uint16_t Points_L[(uint16_t)USE_num][2] = { {  0 } };//左线
uint16_t Points_R[(uint16_t)USE_num][2] = { {  0 } };//右线
uint16_t Dir_R[(uint16_t)USE_num] = { 0 };//用来存储右边生长方向
uint16_t Dir_L[(uint16_t)USE_num] = { 0 };//用来存储左边生长方向
uint16_t Data_Stastics_L = 0;//统计左边找到点的个数
uint16_t Data_Stastics_R = 0;//统计右边找到点的个数
uint8_t Hightest = 0;//最高点

uint8_t L_Border[Image_H];//左线数组
uint8_t R_Border[Image_H];//右线数组
uint8_t Center_Line[Image_H];//中线数组
uint32_t hashMapIndexL[Image_H];
uint32_t hashMapIndexR[Image_H];

//定义膨胀和腐蚀的阈值区间
#define Threshold_Max   255*5//此参数可根据自己的需求调节
#define Threshold_Min   255*2//此参数可根据自己的需求调节

float Image_Erro;
uint8_t Image_Show = 0;
bool isLeftCircle(uint8_t(*Bin_Image)[Image_W], uint8_t* L_Border, uint8_t* R_Border, uint16_t Total_Num_L, uint16_t Total_Num_R, uint16_t* Dir_L, uint16_t* Dir_R, uint16_t(*Points_L)[2], uint16_t(*Points_R)[2]);
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
    if (value >= 0) return value;
    else return -value;
}


/**@brief    限幅函数
-- @param    int16_t x 需要限幅的值
-- @param    int a 最大值
-- @param    int b 最小值
-- @return   x 输出
-- @auther   none
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
-- @auther   none
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
* @param uint8_t begin				输入起点
* @param uint8_t end					输入终点
* @param uint8_t *border				输入需要计算斜率的边界首地址
*  @see CTest		Slope_Calculate(start, end, border);//斜率
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/
float Slope_Calculate(uint8_t begin, uint8_t end, uint8_t* border)
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
* @param uint8_t start				输入起点
* @param uint8_t *border				输入需要计算斜率的边界
* @param uint8_t end					输入终点
* @param float *slope_rate			输入斜率地址
* @param float *intercept			输入截距地址
*  @see CTest		calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/
void calculate_s_i(uint8_t start, uint8_t end, uint8_t* border, float* slope_rate, float* intercept)
{
    uint16_t i, num = 0;
    uint16_t xsum = 0, ysum = 0;
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
-- @auther   none
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
-- @param    uint8_t(*mt9v03x_image)[Image_W] 摄像头采集图像对应的指针
-- @auther   none
-- @date     2023/10/2
**/
void Get_Image(uint8_t(*mt9v03x_image)[Image_W])
{
    uint8_t i = 0, j = 0, Row = 0, Line = 0;
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
-- @auther   none
-- @return   uint8_t Threshold 动态阈值
-- @date     2023/10/2
**/
uint8_t Otsu_Threshold(uint8_t* Image, uint16_t col, uint16_t row)
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
    uint8 Threshold = 0;//???


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
-- @auther   none
-- @date     2023/10/2
**/
void Turn_To_Bin(void)
{
    uint8_t i, j;
    Image_Thereshold = 1.075*Otsu_Threshold(Original_Image[0], Image_W, Image_H);//获取大津法阈值
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
            //   printf(" %d",Bin_Image[i][j]);
        }
        //   printf("\r\n");
    }
    //   printf("end");
}


/**----------------------------------------------------八领域部分------------------------------------------------------------------------**/
/**@brief    寻找两个边界的边界点作为八邻域循环的起始点
-- @param    uint8_t Start_Row 输入任意行数
-- @auther   none
-- @date     2023/10/3
**/
uint8_t Get_Start_Point(uint8_t Start_Row)
{
    uint8_t i = 0, L_Found = 0, R_Found = 0;
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

    if (L_Found && R_Found)//左边和右边都找到起点
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
-- @param    uint16_t_t Break_Flag 最多需要循环的次数
-- @param    uint8_t(*image)[Image_W] 需要进行找点的图像数组，必须是二值图,填入数组名称即可
-- @param    uint16_t_t *L_Stastic 统计左边数据，用来输入初始数组成员的序号和取出循环次数
-- @param    uint16_t_t *R_Stastic 统计右边数据，用来输入初始数组成员的序号和取出循环次数
-- @param    uint8_t L_Start_X 左边起点横坐标
-- @param    uint8_t L_Start_Y 左边起点纵坐标
-- @param    uint8_t R_Start_X 右边起点横坐标
-- @param    uint8_t R_Start_Y 右边起点纵坐标
-- @param    uint8_t Hightest 循环结束所得到的最高高度
-- @auther   none
-- @date     2023/10/3
**/
void Search_L_R(uint16_t Break_Flag, uint8_t(*image)[Image_W], uint16_t* L_Stastic, uint16_t* R_Stastic, uint8_t L_Start_X, uint8_t L_Start_Y, uint8_t R_Start_X, uint8_t R_Start_Y, uint8_t* Hightest)
{

    uint8_t i = 0, j = 0;

    //左边变量
    uint8_t Search_Filds_L[8][2] = { {0} };//寻找八领域数组
    uint8_t Index_L = 0;
    uint8_t Temp_L[8][2] = { {0} };
    uint8_t Center_Point_L[2] = { 0 };
    uint16_t L_Data_Statics;//统计左边找到的点
    //定义八个邻域
    static int8_t Seeds_L[8][2] = { {0,  1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,  0},{1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //这个是顺时针

    //右边变量
    uint8_t Search_Filds_R[8][2] = { {  0 } };
    uint8_t Center_Point_R[2] = { 0 };//中心坐标点
    uint8_t Index_R = 0;//索引下标
    uint8_t Temp_R[8][2] = { {  0 } };
    uint16_t R_Data_Statics;//统计右边
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

/**@brief   从八邻域边界里提取需要的左边线
-- @param   uint16_t_t Total_L 找到的点的总数
-- @auther  none
-- @date    2023/10/3
**/
void Get_Left(uint16_t Total_L)
{
    uint8_t i = 0;
    uint16_t j = 0;
    uint8_t h = 0;
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

/**@brief   从八邻域边界里提取需要的右边线
-- @param   uint16_t_t Total_R 找到的点的总数
-- @auther  none
-- @date    2023/10/3
**/
void Get_Right(uint16_t Total_R)
{
    uint8_t i = 0;
    uint16_t j = 0;
    uint8_t h = 0;
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


/**----------------------------------------------------图像滤波部分------------------------------------------------------------------------**/
/**@brief   形态学滤波
-- @param   uint8_t(*Bin_Image)[Image_W] 二值化图像
-- @auther  none
-- @date    2023/10/3
**/
void Image_Filter(uint8_t(*Bin_Image)[Image_W])//形态学滤波，简单来说就是膨胀和腐蚀的思想
{
    uint16_t i, j;
    uint32_t num = 0;


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
-- @param   uint8_t(*Image)[Image_W]  图像首地址
-- @auther  none
-- @date    2023/10/3
**/
void Image_Draw_Rectan(uint8_t(*Image)[Image_W])
{

    uint8_t i = 0;
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
/**
* @brief 十字补线函数
* @param uint8_t(*Bin_Image)[Image_W]		输入二值图像
* @param uint8_t *L_Border			输入左边界首地址
* @param uint8_t *R_Border			输入右边界首地址
* @param uint16_t_t Total_Num_L			输入左边循环总次数
* @param uint16_t_t Total_Num_R			输入右边循环总次数
* @param uint16_t_t *Dir_L				输入左边生长方向首地址
* @param uint16_t_t *Dir_R				输入右边生长方向首地址
* @param uint16_t_t(*Points_L)[2]		输入左边轮廓首地址
* @param uint16_t_t(*Points_R)[2]		输入右边轮廓首地址
* @see CTest		Cross_Fill(Bin_Image,L_Border, R_Border, data_statics_l, data_statics_r, Dir_L, Dir_R, Points_L, Points_R);
 */
void Cross_Fill(uint8_t(*Bin_Image)[Image_W],  uint8_t* L_Border, uint8_t* R_Border, uint16_t Total_Num_L, uint16_t Total_Num_R, uint16_t* Dir_L, uint16_t* Dir_R, uint16_t(*Points_L)[2], uint16_t(*Points_R)[2])
{
    uint8_t i;
    uint8_t Break_Num_L_UP = 0;//拐点
    uint8_t Break_Num_R_UP = 0;
    uint8_t Break_Num_L_DOWN = 0;
    uint8_t Break_Num_R_DOWN = 0;
    uint8_t start, end;
    float slope_l_rate = 0, intercept_l = 0;

    for (i = 1; i < Total_Num_L; i++)
    {
        if (Dir_L[i - 1] == 2 && Dir_L[i] == 2 && Dir_L[i + 3] == 4 && Dir_L[i + 5] == 4 && Dir_L[i + 7] == 4)//2-4跳变，左下拐点
        {
            Break_Num_L_DOWN = Points_L[i][1];//传递y坐标
            // printf("find l_down\r\n");
            // printf("%d %d\r\n",i,Break_Num_L_DOWN);
            // if(Menu.Image_Show)
            // {
            //     tft180_Draw_ColorCircle(L_Border[Break_Num_L_DOWN],Break_Num_L_DOWN,5,RGB565_BLUE);//画出拐点
            // }
            break;
        }
    }
    for (i = 1; i < Total_Num_R; i++)
    {
        // printf("DIR_R[%d] = %d\r\n",i,Dir_R[i]);
        if (Dir_R[i - 1] == 2 && Dir_R[i] == 2 && Dir_R[i + 3] == 4 && Dir_R[i + 5] == 4 && Dir_R[i + 7] == 4)//2-4跳变，右下拐点
        {
            Break_Num_R_DOWN = Points_R[i][1];//传递y坐标
            // printf("find r_down\r\n");
            // printf("%d %d\r\n",R_Border[Break_Num_R_DOWN],Break_Num_R_DOWN);
            // if(Menu.Image_Show)
            // {
            //     tft180_Draw_ColorCircle(R_Border[Break_Num_R_DOWN],Break_Num_R_DOWN,5,RGB565_RED);
            // }
            break;
        }
    }
    for (i = 1; i < Total_Num_L; i++)
    {
        if (Dir_L[i - 1] == 4 && Dir_L[i] == 4 && Dir_L[i + 3] == 6 && Dir_L[i + 5] == 6 && Dir_L[i + 7] == 6)//4-6跳变，左上拐点
        {
            Break_Num_L_UP = Points_L[i][1];//传递y坐标
            // printf("find l_up\r\n");
            // printf("%d %d\r\n",L_Border[Break_Num_L_UP - 10],Break_Num_L_UP);
            // if(Menu.Image_Show)
            // {
            //     tft180_Draw_ColorCircle(L_Border[Break_Num_L_UP - 10],Break_Num_L_UP,5,RGB565_BLUE);
            // }
            break;
        }
    }
    for (i = 1; i < Total_Num_R; i++)
    {
        // printf("DIR_R[%d] = %d\r\n",i,Dir_R[i]);
        if (Dir_R[i - 1] == 4 && Dir_R[i] == 4 && Dir_R[i + 3] == 6 && Dir_R[i + 5] == 6 && Dir_R[i + 7] == 6)//4-6跳变，右上拐点
        {
            Break_Num_R_UP = Points_R[i][1];//传递y坐标
            // printf("find R_up\r\n");
            // printf("%d %d\r\n",R_Border[Break_Num_R_UP - 10],Break_Num_R_UP);
            // if(Menu.Image_Show)
            // {
            //     tft180_Draw_ColorCircle(R_Border[Break_Num_R_UP - 10],Break_Num_R_UP,5,RGB565_RED);
            // }
            break;
        }
    }

    if (Break_Num_L_DOWN && Break_Num_R_DOWN && (Bin_Image[Image_H - 10][4] == 0) && (Bin_Image[Image_H - 10][Image_W - 4] == 0))//十字前
    {
        Image_Flag.Cross_Fill = true;
        //计算斜率,左边斜率
        start = Break_Num_L_DOWN + 2;
        end = Break_Num_L_DOWN + 7;
        end = Limit_a_b(end, 0, Image_H);//限幅

        calculate_s_i(start, end, L_Border, &slope_l_rate, &intercept_l);
        // printf("slope_l_rate:%f\nintercept_l:%f\n", slope_l_rate, intercept_l);
        for (i = Break_Num_L_DOWN; i < Image_H - 1; i--)
        {
            L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
        }

        //计算斜率,右边斜率
        start = Break_Num_R_DOWN + 2;
        end = Break_Num_R_DOWN + 7;
        end = Limit_a_b(end, 0, Image_H);//限幅
        calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
        //printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
        for (i = Break_Num_R_DOWN + 2; i < Image_H - 1; i--)
        {
            R_Border[i] = slope_l_rate * (i)+intercept_l;
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);
        }
        // tft180_show_string(Row_10,Line_7,"fuck");
    }
    else if (Break_Num_L_UP && Break_Num_R_UP && Bin_Image[Image_H - 10][4] && Bin_Image[Image_H - 10][Image_W - 4])//十字中
    {
        Image_Flag.Cross_Fill = true;
        //计算斜率
        start = Break_Num_L_UP - 15;
        start = Limit_a_b(start, 0, Image_H);
        end = Break_Num_L_UP - 5;
        calculate_s_i(start, end, L_Border, &slope_l_rate, &intercept_l);
        // printf("slope_l_rate:%f\nintercept_l:%f\n", slope_l_rate, intercept_l);
        for (i = Break_Num_L_UP - 5; i < Image_H - 1; i++)
        {
            L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
        }

        //计算斜率
        start = Break_Num_R_UP - 15;//起点
        start = Limit_a_b(start, 0, Image_H);//限幅
        end = Break_Num_R_UP - 5;//终点
        calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
        //printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
        for (i = Break_Num_R_UP - 5; i < Image_H - 1; i++)
        {
            R_Border[i] = slope_l_rate * (i)+intercept_l;
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);
        }
        // tft180_show_string(Row_10,Line_7,"aaaa");
    }

 }


/**@brief    判断是否为直线
-- @param    uint16 Border 需要判断的线
-- @param    uint16 Total_Num 数量
-- @return   是否为直线
-- @auther   none
-- @date     2023/10/2
**/
bool Straight_Line_Judge(uint8* Border, uint16 Total_Num, lineTypeDef lineMode)
{
    uint8_t StraightPoint = 0; // 记录是直线的点的个数
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
    for (int i = Image_H - 2; i > Hightest; i-= 1) {
        // 如果当前元素小于或等于前一个元素，则不是单调递减的
        // uint8_t before = Limit_a_b(i - 1, Hightest, Image_H - 2);
        // uint8_t next = Limit_a_b(i - 1, Hightest, Image_H - 2);
       // printf("当前: % d 后一个点 : % d \n", Border[i], Border[i - 1]);
        if (lineMode == RightLine)
        {             
            if (Border[i] - Border[i - 1] >= 0 && Border[i] - Border[i - 1] < 2)
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
            else
            {
                //circle(frame, Point(Border[i], i), 0, Scalar(0, 0, 255), 1);
                //return false;
            }
            if (StraightPoint > 70)
            {
                break;
            }
            //imshow("原图像", frame);
            //waitKey(2);
        }
        if (lineMode == LeftLine)
        {
            // if (Border[i - 1] - Border[i] >= 0 && Border[i - 1] - Border[i] < 2)
            // {
            //     //circle(frame, Point(Border[i], i), 0, Scalar(255, 0, 0), 1);
            // }
            // else
            // {
            //     //circle(frame, Point(Border[i], i), 0, Scalar(0, 0, 255), 1);
            //     //return false;
            // }
        }
        //if (Border[i] < Border[i - 1]) {
        //    //printf("b1[%d] b2[%d]\r\n",i,i-1);
        //    //return false;
        //}

        //if (Border[i] - Border[i - 1] >= 2)
        //{
        //    //return false;
        //}

    }
    // printf("Line: %d \n", StraightPoint);
    if (StraightPoint >= 68)
    {
        //printf("Line\n");
        return true;
    }
    else
    {
        //printf("Not Line");
        return false;
    }
    //if (Total_Num >= 120)
    //{
    //    return false;
    //}
    // 如果没有发现违反以上的情况，返回true
    //return true;
}


uint16 Find_Salient_Point(uint8* Border, uint16 size)
{
    int maxIndex = 0;

    // 遍历数组，找到最大值的索引
    for (int i = size; i < Image_H - 1; i++) {
        if (Border[i] > Border[maxIndex]) {
            maxIndex = i;
        }
    }

    return maxIndex;
}


uint16_t findCircleOutPoint(uint8_t* L_Border)
{
    // 寻找圆环右顶点
    for (uint8_t i = 8; i < Image_H - 8; i++)
    {
        uint8_t before = Limit_a_b(i - 5, 0, Image_H - 2);
        uint8_t next = Limit_a_b(i + 5, 0, Image_H - 2);
        //circle(frame, Point(L_Border[i], i), 0, Scalar(255, 255, 0), 1);
        //circle(frame, Point(L_Border[before], before), 0, Scalar(0, 255, 0), 1);
        //circle(frame, Point(L_Border[next], next), 0, Scalar(0, 0, 255), 1);
        // printf("now - before: %d now - next: %d \n", L_Border[i] - L_Border[before], L_Border[i] - L_Border[next]);
        if ((L_Border[i] - L_Border[before] > 0) && (L_Border[i] - L_Border[next] > 0)
            && (L_Border[i] - L_Border[before] < 10) && (L_Border[i] - L_Border[next] < 10)
            && L_Border[i] > Border_Min + 5 && L_Border[before] > Border_Min + 5 && L_Border[next] > Border_Min + 5 && i < Image_H - 20)
        {
  /*          circlePoint[0] = i;
            circlePoint[1] = L_Border[i];*/
            //circleFlag = true;
            //circle(frame, Point(L_Border[i], i), 5, Scalar(255, 0, 0), 1);
            return i;
        }
        //imshow("原图像", frame);
        //waitKey(5);
    }
    return NULL;
}

// 右圆环
//void Right_Ring(uint8_t(*Bin_Image)[Image_W], uint8_t* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R,
//    uint16* Dir_L, uint16* Dir_R, uint16(*Points_L)[2], uint16(*Points_R)[2])
//{
//    int i;
//    static uint8_t WhitePixel_Time = 0;
//    uint8_t hashKey[Image_H] = { 0 };
//    uint8_t offset = 0;
//    uint8_t StraightNum = 0;
//    uint8_t circlePoint[2];
//    uint16 Break_Num_R_UP = 0;//拐点
//    uint16 Break_Num_R_DOWN = 0;
//    uint16 Salient_Point = 0;//圆环凸点
//    uint16_t Break_Num_R_UP = 0;
//    uint16 start, end;
//    float slope_l_rate = 0, intercept_l = 0;
//}

void Left_Ring(uint8_t(*Bin_Image)[Image_W], uint8_t* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R,
    uint16* Dir_L, uint16* Dir_R, uint16(*Points_L)[2], uint16(*Points_R)[2])
{
    int i;
    static uint8_t WhitePixel_Time = 0;
    uint8_t hashKey[Image_H] = { 0 };
    uint8_t offset = 0;
    uint8_t StraightNum = 0;
    uint8_t circlePoint[2];
    uint16 Break_Num_L_UP = 0;//拐点
    uint16 Break_Num_L_DOWN = 0;
    uint16 Salient_Point = 0;//圆环凸点
    uint16_t Break_Num_R_UP = 0;
    uint16 start, end;
    float slope_l_rate = 0, intercept_l = 0;
    static float Curanglg = 0;
    static float Lastanglg = 0;
    static float Angle_Offest = 0;
    //LeftRing.Ring_State = Enter_Ring_First;//入环中
    switch (LeftRing.Ring_State)
    {
    case Ring_Front:
        // printf("入环前\r\n");
        //cout << "入环前" << endl;
        LeftRing.Stright_Line = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//判断右边是否为长直线
        Salient_Point = findCircleOutPoint(L_Border);
        // tft180_Draw_ColorCircle(L_Border[Salient_Point],Salient_Point,5,RGB565_RED);
        for (uint8_t i = Image_H - 2; i > Image_H / 2; --i)
        {
            // 断裂区域横坐标相差很大
            if (L_Border[i] - L_Border[i - 1] > 10 && i > Image_H / 2)
            {
                hashKey[offset] = i;
                offset++;
                break;
            }

        }
        if (offset > 0)
        {
            for (uint8_t i = 0;i < offset; i++)
            {
                uint8_t edgeIndex = hashMapIndexL[hashKey[i]];
                uint16_t start = Limit_a_b((edgeIndex - 2), 0, Data_Stastics_L);
                uint16_t end = Limit_a_b((edgeIndex + 7), 0, Data_Stastics_L);
                for (uint16_t j = start; j < end; j++)
                {
                    uint16_t before = Limit_a_b(j - 5, start, end);
                    uint16_t next = Limit_a_b(j + 5, start, end);
                    if (Points_L[j][1] < Points_L[j - 5][1] && Points_L[j][1] < Points_L[j + 5][1]
                        && Points_L[j][0] < Points_L[j - 5][0] && Points_L[j][0] > Points_L[j + 5][0] && Points_L[j][0] > Border_Min + 5)
                    {
                        Break_Num_L_DOWN = j;
                        break;
                    }
                }
            }
        }
        // tft180_Draw_ColorCircle(Points_L[Break_Num_L_DOWN][0],Points_L[Break_Num_L_DOWN][1],5,RGB565_BLUE);
        // 有圆环突出点 右直线 没有左下角点
        if (Salient_Point && LeftRing.Stright_Line && !Break_Num_L_DOWN && LeftRing.Ring_Front_Flag) // 判断是否有圆环突出点并且右侧直线
        {
            Get_K_b(L_Border[Salient_Point], Salient_Point, 0, Image_H - 1, &slope_l_rate, &intercept_l);
            for (i = Image_H - 1; i > 1; i--)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
            }
        }
        //有圆环突出点, 右直线, 有左下角点
        else if (Break_Num_L_DOWN && LeftRing.Stright_Line && Salient_Point) //  && (!Bin_Image[Image_H - 10][4]) && (!Bin_Image[Image_H - 10][Image_W - 4])
        {
            //计算斜率,左边斜率
            Get_K_b(Salient_Point, L_Border[Salient_Point], Points_L[Break_Num_L_DOWN][1], Points_L[Break_Num_L_DOWN][0], &slope_l_rate, &intercept_l);
            for (i = Salient_Point; i < Points_L[Break_Num_L_DOWN][1]; i++)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
            }
            LeftRing.Ring_Front_Flag = true;
        }
        // 没有圆环突出点, 右直线， 有左下角点
        else if (Break_Num_L_DOWN && LeftRing.Stright_Line && !Salient_Point)
        {
            Get_K_b(Points_L[Break_Num_L_DOWN][1], Points_L[Break_Num_L_DOWN][0], 1, (Image_W / 2 - 10), &slope_l_rate, &intercept_l);
            for (i = Points_L[Break_Num_L_DOWN][1]; i >= 1; --i)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
            }
            LeftRing.Ring_Front_Flag = true;
        }
        // else
        // {
        //     LeftRing.Ring_Front_Flag = false;
        // }
        // printf("%d,%d,%d,%d\r\n",Bin_Image[Image_H - 5][4],Bin_Image[Image_H - 10][8],Bin_Image[Image_H / 2][Image_W - 2],Bin_Image[Image_H-5][Image_W - 2]);
        // 左下空白 且检测到环左下角点, 右直线, 左圆环突出点, 右边中心为黑色, 右边下方为黑色
        if ((Bin_Image[Image_H - 5][4]) && (Bin_Image[Image_H - 10][8]) && LeftRing.Ring_Front_Flag &&
            (!Bin_Image[Image_H / 2][Image_W - 2]) && (!Bin_Image[Image_H-5][Image_W - 2]))//图像左下方为一片白 (Bin_Image[Image_H - 5][4]) && (Bin_Image[Image_H - 10][8]) &&
        {
            LeftRing.Ring_Front_Flag = false;
            LeftRing.Ring = true;
            LeftRing.Ring_State = Enter_Ring_First;//入环中 Enter_Ring_First
        }
        //imshow("原图像", frame);
        // waitKey(200);
        break;
    case Enter_Ring_First:
        // printf("入环中\r\n");
        LeftRing.Stright_Line = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//判断右边是否为长直线
        Salient_Point = findCircleOutPoint(L_Border);//Find_Salient_Point(L_Border, Image_H / 2 - 10);
        for (i = 1; i < Total_Num_L; i++)
        {
            uint16_t before = Limit_a_b(i - 5, 0, Total_Num_L);
            uint16_t next = Limit_a_b(i + 5, 0, Total_Num_L);
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
        if (Salient_Point && LeftRing.Stright_Line) // 判断是否有圆环突出点并且右侧直线
        {
            Get_K_b(L_Border[Salient_Point], Salient_Point, 0, Image_H - 1, &slope_l_rate, &intercept_l);
            for (i = Image_H - 1; i > 1; i--)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
            }
            LeftRing.Enter_Ring_First_Flag = true;
        }
        // 第一次入环标志位, 找到上角点, 找不到圆环突出点, 右直线
        if (LeftRing.Enter_Ring_First_Flag && Salient_Point == NULL && Break_Num_L_UP)    //&& (Bin_Image[20][4]) && (Bin_Image[20][8]) && (Bin_Image[Image_H - 2][4]) && (Bin_Image[Image_H - 2][8])
        {
            LeftRing.Enter_Ring_First_Flag = false;
            LeftRing.Ring_State = Leave_Ring_First;//第一次离开环
        }
        break;
    case Leave_Ring_First:
        // printf("第一次离开环\r\n");
        // cout << "第一次离开环" << endl;
        for (i = 1; i < Total_Num_L; i++)
        {
            uint16_t before = Limit_a_b(i - 5, 0, Total_Num_L);
            uint16_t next = Limit_a_b(i + 5, 0, Total_Num_L);
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

        if (Break_Num_L_UP) // && (Bin_Image[50][4]) && (Bin_Image[50][8])
        {
            //Get_K_b(2, Image_H - 10, Image_W - 2, Image_H - 5, &slope_l_rate, &intercept_l);
            for (i = Image_H - 1; i > 1; i--)
            {
                L_Border[i] = Border_Min;
                R_Border[i] = Border_Min;//y = kx+b
                R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
            }
            LeftRing.Leave_Ring_First_Flag = true;
        }
        // 左下一片空白, 找不到左上角点
        if (Bin_Image[Image_H - 10][20] && Bin_Image[Image_H - 10][25] && !Break_Num_L_UP)
        {
            LeftRing.Leave_Ring_First_Flag = true;
            LeftRing.Ring_State = In_Ring;
        }
        break;
    case In_Ring:
        // printf("环内\r\n");
        Curanglg = Gyro_YawAngle_Get();
        if(Lastanglg == 0)
        {
            Lastanglg = Curanglg;
        }
        Angle_Offest += Curanglg - Lastanglg;
        // printf("%f,%f,%f\r\n",Curanglg,Lastanglg,Angle_Offest);
        for (i = 1; i < Total_Num_R; i++)
        {
            uint16_t before = Limit_a_b(i - 8, 0, Total_Num_R);
            uint16_t next = Limit_a_b(i + 8, 0, Total_Num_R);
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
            // Get_K_b(Points_R[Break_Num_R_UP][0], Points_R[Break_Num_R_UP][1], 2, 2, &slope_l_rate, &intercept_l);
            for (i = Image_H - 1; i > 1; i--)
            {
                L_Border[i] = Border_Min;
                R_Border[i] = Border_Min;//((i)-intercept_l) / slope_l_rate;//y = kx+b
                R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//限幅
            }
        }
        Lastanglg = Curanglg;
        // 左右两侧都为白色, 准备要出环
        if (Bin_Image[Image_H - 4][4] == White_Pixel && Bin_Image[Image_H - 4][Image_W - 4] == White_Pixel && fabs(Angle_Offest) >= 50)
        {
            LeftRing.Ring_State = Ready_Out_Ring;
            Angle_Offest = 0;
            Lastanglg = 0;
        }
        
        break;
    case Ready_Out_Ring:
        // printf("准备出环\r\n");
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
        if (Data_Stastics_R > Data_Stastics_L && fabs(Angle_Offest) > 35)
        {
            LeftRing.Ring_State = Leave_Ring;
            Angle_Offest = 0;
            Lastanglg = 0;
        }
        
        break;
    case Leave_Ring:
        // printf("出环\r\n");
        Break_Num_L_UP = 0;
        //cout << "出环中" << ;
        for (i = 1; i < Total_Num_L; i++)
        {
            uint16_t before = Limit_a_b(i - 10, 0, Total_Num_L);
            uint16_t next = Limit_a_b(i + 10, 0, Total_Num_L);
            if (before == Total_Num_L || next == Total_Num_L || Points_L[i][1] < 20)
            {
                break;
            }
            if (Dir_L[before] == 6 && Dir_L[i] == 4 && Dir_L[next] == 4 && Dir_L[next-1] == 4 && Dir_L[next-2] == 4)
            {
                Break_Num_L_UP = i;//传递y坐标
                break;
            }
        }

        if (LeftRing.Leave_Ring_First_Flag && Break_Num_L_UP)
        {
            Get_K_b(Points_L[Break_Num_L_UP][1], Points_L[Break_Num_L_UP][0], Image_H - 1, 2, &slope_l_rate, &intercept_l);
            for (i = Points_L[Break_Num_L_UP][1]; i <= Image_H - 1; ++i)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//限幅
            }
        }
        // 检测到已经看见过环的标志位, 右下角为黑色, 并且看不到上角点, 代表已经出环
        else if (LeftRing.Leave_Ring_First_Flag && Bin_Image[Image_H - 4][4] == Black_Pixel && Bin_Image[Image_H - 2][Image_W - 4] == Black_Pixel 
                && !Break_Num_L_UP)
        {
            LeftRing.Ring_Front_Flag = false; // 出环
            LeftRing.Enter_Ring_First_Flag = false;
            LeftRing.Leave_Ring_First_Flag = false;
            LeftRing.Stright_Line = false;
            LeftRing.Ring = false;
            LeftRing.Ring_State = Ring_Front;
        }
        // waitKey(200);
        break;
    }
}

void Test()
{
    Get_Image(mt9v03x_image);//获取一副图像
    // timer_start(GPT_TIM_1);
    // my_sobel(Original_Image,Bin_Image);
    Turn_To_Bin();//二值化
}

/**@brief   最终调用的图像处理的函数
-- @param   无
-- @auther  none
-- @date    2023/10/3
**/
void Image_Process(void)
{
    uint16_t i;
    // timer_start(GPT_TIM_1);
    // my_sobel(Original_Image,Bin_Image);
    Get_Image(mt9v03x_image);
    Turn_To_Bin();//二值化

    Image_Filter(Bin_Image);//滤波
    Image_Draw_Rectan(Bin_Image);//预处理

    //清零
    Data_Stastics_L = 0;
    Data_Stastics_R = 0;

    if (Get_Start_Point(Image_H - 2))//从图像的最下面开始找，找到起点了，再执行八领域，没找到就一直找
    {
        //        printf("正在开始八领域\n");
        Search_L_R((uint16_t)USE_num, Bin_Image, &Data_Stastics_L, &Data_Stastics_R, Start_Point_L[0], Start_Point_L[1], Start_Point_R[0], Start_Point_R[1], &Hightest);
        //        printf("八邻域已结束\n");
                // 从爬取的边界线内提取边线 ， 这个才是最终有用的边线
        Get_Left(Data_Stastics_L);
        Get_Right(Data_Stastics_R);
        // 优先判断是否是十字如果是十字则不对圆环判断
        if (!LeftRing.Ring)
        {
            Cross_Fill(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);//十字补线
        }
        // 同上
        //Cross_Fill(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);//十字补线
        if (!Image_Flag.Cross_Fill)
        {
            Left_Ring(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);
        }
    }
    
    for (int i = Hightest; i < Image_H-1; i++)
    {
        Center_Line[i] = (L_Border[i] + R_Border[i]) >> 1;//求中线
    }
    Image_Erro = (Center_Line[69])*0.375f + (Center_Line[70])*0.5f + (Center_Line[71])*0.1f;
    Image_Flag.Cross_Fill = false;
}