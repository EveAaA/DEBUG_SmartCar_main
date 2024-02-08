/**
    @brief: 总钻风版本对于周边目标板的检测
    @author: 戴骐阳
    @date: 2024-01-28
*/
#include "Border.h"
#define Threshold_Max 100 // 此参数可根据自己的需求调节
#define Threshold_Min 20 // 此参数可根据自己的需求调节

AT_SDRAM_SECTION_ALIGN(double M1[IMAGE_H][IMAGE_W], 64);
AT_SDRAM_SECTION_ALIGN(double M2[IMAGE_H][IMAGE_W], 64);
AT_SDRAM_SECTION_ALIGN(double M3[IMAGE_H][IMAGE_W], 64);

// 高斯模糊 3*3 核
const int GaussianKernel[3 * 3] = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1};

/**
    @brief: 快速开方根
    @return: 开方根结果
*/
float Q_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;           // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1); // what the fuck?
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration
                                         // y = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
    return y;
}

/**
 * @brief : 绘制角点(八邻域都改变)
 * @param : 图像数组, i, j
 * @return : 0
 *
 */
void DrawCorner(uint8_t (*img)[IMAGE_W], int i, int j)
{
    for (int x = i - 1; x <= i + 1; x++)
    {
        for (int y = j - 1; y <= j + 1; y++)
        {
            img[x][y] = 255;
        }
    }
}

/**
 * @brief 获取图像 (不需要该函数, 直接memcpy函数更快)
 *
 * 从给定的图像数组中获取图像数据，并将其存储在指定的图像结构体中。
 *
 * @param image 图像结构体指针
 * @param mt9v03x_image 图像数组指针
 */
/*
void Get_Image(ImageTypeDef *image, uint8_t (*mt9v03x_image)[IMAGE_W])
{
    uint8 i = 0, j = 0, Row = 0, Line = 0;
    for (i = 0; i < IMAGE_H; i++)
    {
        for (j = 0; j < IMAGE_W; j++)
        {
            // 获取图像
            image->GrayImage[i][j] = mt9v03x_image[i][j];
        }
    }
}
*/

/**
 * @brief : Harris高斯滤波
 * @param : 特征矩阵R, kernel
 * @return: none
 *
 */
void HarrisGussianFilter(double (*M1)[IMAGE_W], double (*M2)[IMAGE_W], double (*M3)[IMAGE_W], const int *kernal)
{
    int16_t index;
    double sum1, sum2, sum3;
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            index = 0;
            sum1 = sum2 = sum3 = 0.0;
            // 图像卷积部分
            for (uint8_t m = i - 1; m < i + 2; m++)
            {
                for (uint8_t n = j - 1; n < j + 2; n++)
                {
                    sum1 += M1[m][n] * kernal[index];
                    sum2 += M2[m][n] * kernal[index];
                    sum3 += M3[m][n] * kernal[index];
                    index++;
                }
            }
            M1[i][j] = sum1 / 16.0;
            M2[i][j] = sum2 / 16.0;
            M3[i][j] = sum3 / 16.0;
        }
    }
}

/**
    brief: 高斯滤波 (3*3核)
    param: 图像数组指针
    param: 核数组
    return: none
*/
void GussianFilter(ImageTypeDef *image, const int *kernal)
{
    int16_t index, sum;
    uint8_t tmpdata[IMAGE_H][IMAGE_W];
    memcpy(tmpdata, image->GrayImage, sizeof(image->GrayImage));
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            index = sum = 0;
            // 图像卷积部分
            for (uint8_t m = i - 1; m < i + 2; m++)
            {
                for (uint8_t n = j - 1; n < j + 2; n++)
                {
                    sum += tmpdata[m][n] * kernal[index++];
                }
            }
            image->GrayImage[i][j] = sum / 16;
        }
    }
}

/**
    @brief: 双阈值进一步过滤(canny)
    @param: 图像结构体, 上下限阈值
    @return: none
*/
void DoubleThreshold(ImageTypeDef *image, uint8_t min, uint8_t max)
{
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            if (image->OutImage[i][j] >= max)
            {
                image->OutImage[i][j] = 255;
            }
            else if (image->OutImage[i][j] >= min)
            {
                // 在阈值范围内
                // 检测八邻域像素
                if (image->OutImage[i - 1][j - 1] == 255 || image->OutImage[i - 1][j] == 255 || image->OutImage[i - 1][j + 1] == 255 || image->OutImage[i][j - 1] == 255 || image->OutImage[i][j + 1] == 255 ||
                    image->OutImage[i + 1][j - 1] == 255 || image->OutImage[i + 1][j] == 255 || image->OutImage[i + 1][j + 1] == 255)
                {
                    // 若周围任意为强边界
                    image->OutImage[i][j] = 255;
                }
                else
                {
                    image->OutImage[i][j] = 0;
                }
            }
            else
            {
                // 低于阈值非边界 为黑色
                image->OutImage[i][j] = 0;
            }
        }
    }
}

/**
  @brief: 对图像最边界进行判断, 防止八邻域超出图像范围
  @param: 图像当前坐标i, j
  @param: 八邻域像素存储
  @param: 当前图像
  @return: none
*/
void writeIntoNeiborPixel(uint8_t *neiborPixel, uint8_t (*img)[IMAGE_W], uint8_t i, uint8_t j)
{
    // 左上边界
    if (i - 1 >= 0 && j - 1 >= 0)
        neiborPixel[0] = img[i - 1][j - 1];
    // 上中边界
    if (i - 1 >= 0 && j >= 0)
        neiborPixel[1] = img[i - 1][j + 0];
    // 右上边界
    if (i - 1 >= 0 && j + 1 < IMAGE_W)
        neiborPixel[2] = img[i - 1][j + 1];
    // 左中边界
    if (i >= 0 && j - 1 >= 0)
        neiborPixel[3] = img[i + 0][j - 1];
    // 中间
    if (j >= 0 && i >= 0)
        neiborPixel[4] = img[i + 0][j + 0];
    // 右中边界
    if (i >= 0 && j + 1 < IMAGE_W)
        neiborPixel[5] = img[i + 0][j + 1];
    // 左下边间
    if (i + 1 < IMAGE_H && j - 1 >= 0)
        neiborPixel[6] = img[i + 1][j - 1];
    // 中下边间
    if (i + 1 < IMAGE_H && j >= 0)
        neiborPixel[7] = img[i + 1][j + 0];
    // 右下边界
    if (i + 1 < IMAGE_H && j + 1 <= IMAGE_W)
        neiborPixel[8] = img[i + 1][j + 1];
}

/**
    @brief: 非极大值抑制
    @param: 图像结构体指针，方向角数组, 边界阈值
    @return: none
*/
void NMS(uint8_t (*img)[IMAGE_W], double (*Theata)[IMAGE_W])
{
    double Ga, Gb;             // 亚像素
    double Ga1, Ga2, Gb1, Gb2; // 邻域内像素
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            double Dir = Theata[i][j]; // 获取当前像素方向角
            double weight;
            if (RadToAngle(Dir) >= 0 && RadToAngle(Dir) <= 45)
            {
                Ga1 = img[i + 1][j - 1];
                Ga2 = img[i + 0][j - 1];
                Gb1 = img[i - 0][j + 1];
                Gb2 = img[i - 1][j + 1];
                weight = tan(Dir);
            }
            else if (RadToAngle(Dir) > 45 && RadToAngle(Dir) < 90)
            {
                Ga1 = img[i - 1][j + 1];
                Ga2 = img[i - 1][j + 0];
                Gb1 = img[i + 1][j - 0];
                Gb2 = img[i + 1][j - 1];
                weight = 1.0f / tan(Dir);
            }
            else if (RadToAngle(Dir) > 90 && RadToAngle(Dir) <= 135)
            {
                Ga1 = img[i - 1][j - 1];
                Ga2 = img[i - 1][j + 0];
                Gb1 = img[i + 1][j - 0];
                Gb2 = img[i + 1][j + 1];
                weight = 1.0f / tan(Dir);
            }
            else if (RadToAngle(Dir) > 135 && RadToAngle(Dir) <= 180)
            {
                Ga1 = img[i + 1][j + 1];
                Ga2 = img[i + 1][j - 0];
                Gb1 = img[i - 0][j - 1];
                Gb2 = img[i - 1][j - 1];
                weight = tan(Dir);
            }
            Ga = weight * Ga1 + (1 - weight) * Ga2;
            Gb = weight * Gb1 + (1 - weight) * Gb2;
            // 判断是否为极大值
            if (img[i][j] < Ga || img[i][j] < Gb)
            {
                img[i][j] = 0;
            }
        }
    }
}

/**
 * @brief : 形态学腐蚀算法
 * @param : 图像数组
 * @param : 腐蚀核大小
 * @return: none
 */
static void dilate(uint8_t (*image)[IMAGE_W], uint8_t size)
{
    uint8_t structElmentHalf = size / 2;
    uint8_t tempImage[IMAGE_H][IMAGE_W]; // 临时图像
    for (uint8_t i = structElmentHalf; i < IMAGE_H - structElmentHalf; i++)
    {
        for (uint8_t j = structElmentHalf; j < IMAGE_W - structElmentHalf; j++)
        {
            uint8_t min = 255;
            for (uint8_t k = i - structElmentHalf; k < i + structElmentHalf; k++)
            {
                for (uint8_t l = j - structElmentHalf; l < j + structElmentHalf; l++)
                {
                    if (image[k][l] < min)
                    {
                        min = image[k][l];
                    }
                }
            }
            tempImage[i][j] = min;
        }
    }
    memcpy(image, tempImage, sizeof(tempImage));
}

/**
    @brief: sobel算子边缘检测
    @param: 图像结构体指针
    @param: 上下限阈值
*/
void SobelfindBorderEdge(ImageTypeDef *image, double (*M1)[IMAGE_W], double (*M2)[IMAGE_W], double (*M3)[IMAGE_W], int16_t k)
{
    double P; // x方向偏导
    double Q; // y方向偏导
    double Gradient;
    uint8_t neiborPixel[9]; // 八邻域像素存储
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            // 获取八邻域像素
            writeIntoNeiborPixel(neiborPixel, image->GrayImage, i, j);
            // 求x方向偏导
            P = (double)abs((neiborPixel[2] + neiborPixel[5] * 2 + neiborPixel[8] - neiborPixel[0] - neiborPixel[3] * 2 - neiborPixel[6]));
            // 求y方向偏导
            Q = (double)abs((neiborPixel[6] + neiborPixel[7] * 2 + neiborPixel[8] - neiborPixel[0] - neiborPixel[1] * 2 - neiborPixel[2]));
            // 存储各个方向梯度相对于乘积
            M1[i][j] = P * P;
            M2[i][j] = P * Q;
            M3[i][j] = Q * Q;
            // 获取方向角
            // Theata[i][j] = atan(Q / P);
            // 梯度
            Gradient = 1.0f / Q_rsqrt(P * P + Q * Q);
            if (Gradient < k)
            {
                image->OutImage[i][j] = 0;
            }
            else
            {
                image->OutImage[i][j] = Gradient * 1.5;
            }
        }
    }
}

/**
 * @brief Harris响应数值的非极大值抑制
 * @param R 特征矩阵，用于存储角点响应值
 * @param rows 图像行数
 * @param cols 图像列数
 * @param radius 非极大值抑制局部窗口半径
 * @param threshold 角点响应值阈值
 */
void HarrisNMS(ImageTypeDef *image, double R[IMAGE_H][IMAGE_W], int rows, int cols, int radius, double threshold)
{
    for (int i = radius; i < rows - radius; i++)
    {
        for (int j = radius; j < cols - radius; j++)
        {
            double local_max = 0.0;
            for (int x = -radius; x <= radius; x++)
            {
                for (int y = -radius; y <= radius; y++)
                {
                    local_max = fmax(local_max, R[i + x][j + y]);
                }
            }
            if (R[i][j] == local_max && local_max > threshold)
            {
                // 非极大值抑制
                // image->GrayImage[i][j] = 255; // 为了可视化，可以设置一个非零值
                DrawCorner(image->GrayImage, i, j);
            }
        }
    }
}

/**
 * @brief: Harris角点检测
 * @param: 角点响应数组R, 梯度计算后存储数组M
 * @return: none
 */
void GetHarrisPoint(double (*R)[IMAGE_W], double (*M1)[IMAGE_W], double (*M2)[IMAGE_W], double (*M3)[IMAGE_W], double k)
{
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            double A = M1[i][j]; // Ixx
            double C = M2[i][j]; // Ixy
            double B = M3[i][j]; // Iyy
            double Idet = A * B - C * C;
            double Itrace = A + B;
            // 获取角点响应值
            R[i][j] = Idet - k * (Itrace * Itrace);
            // tft180_show_float(60, 50, M1[i][j], 3, 3);
            // tft180_show_float(60, 65, M2[i][j], 3, 3);
            // tft180_show_float(60, 80, M3[i][j], 3, 3);
            // tft180_show_float(10, 95, Idet, 7, 2);
            // tft180_show_float(60, 105, Itrace, 5, 3);
            // system_delay_ms(500);
        }
    }
}

/**
 * @brief : FAST角点检测
 * @param : 图像数组
 * @return : none
 *
 */
bool FASTCorner(uint8_t (*image)[IMAGE_W], uint8_t i, uint8_t j, int16_t Threshold)
{
    uint8_t count = 0;
    int16_t xx, yy;
    int offsets[16][2] = {
        {-3, 0}, {-3, 1}, {-2, 2}, {-1, 3}, {0, 3}, {1, 3}, {2, 2}, {3, 1}, {3, 0}, {3, -1}, {2, -2}, {1, -3}, {0, -3}, {-1, -3}, {-2, -2}, {-3, -1}};

    for (uint8_t m = 0; m < 16; m++)
    {
        xx = i + offsets[m][0];
        yy = j + offsets[m][1];
        if (xx < 0 || xx >= IMAGE_H || yy < 0 || yy >= IMAGE_W)
        {
            return false;
        }
        else
        {
            uint8_t pixel = image[xx][yy];
            if (pixel < image[i][j] - Threshold || pixel > image[i][j] + Threshold)
            {
                count++;
            }
            else
            {
                return false;
            }
        }
        xx = 0;
        yy = 0;
    }

    // 是否周围有大于等于9个较亮的点
    if (count >= 9)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**@brief   形态学滤波
-- @param   uint8(*Bin_Image)[Image_W] 二值化图像
-- @auther  none
-- @date    2023/10/3
**/
void Image_Filter(uint8 (*Bin_Image)[IMAGE_W]) // 形态学滤波，简单来说就是膨胀和腐蚀的思想
{
    uint16 i, j;
    uint32 num = 0;

    for (i = 1; i < IMAGE_H - 1; i++)
    {
        for (j = 1; j < (IMAGE_W - 1); j++)
        {
            // 统计八个方向的像素值
            num =
                Bin_Image[i - 1][j - 1] + Bin_Image[i - 1][j] + Bin_Image[i - 1][j + 1] + Bin_Image[i][j - 1] + Bin_Image[i][j + 1] + Bin_Image[i + 1][j - 1] + Bin_Image[i + 1][j] + Bin_Image[i + 1][j + 1];

            if (num >= Threshold_Max && Bin_Image[i][j] == 0)
            {
                Bin_Image[i][j] = 255; // 白  可以搞成宏定义，方便更改
            }
            if (num <= Threshold_Min && Bin_Image[i][j] == 255)
            {

                Bin_Image[i][j] = 0; // 黑
            }
        }
    }
}

/*************外部调用函数************/
/*
    @brief: 寻找目标板
    @param: 总钻风图像数组接口
    @param: 双阈值min, max
    @param: 边缘阈值edge_threshold
*/
void FindBorder(double k, double Threshold)
{
    ImageTypeDef img;
    double R[IMAGE_H][IMAGE_W]; // 获取角点响应值
    // double Theata[IMAGE_H][IMAGE_W]; // 方向角
    // 1. 获取灰度图像
    memcpy(img.GrayImage, mt9v03x_image, sizeof(img.GrayImage));
    // 2. 对图像高斯模糊
    GussianFilter(&img, GaussianKernel);
    // 3. 边缘检测
    SobelfindBorderEdge(&img, M1, M2, M3, k);
    // 4. 腐蚀
#if FAST_CORNER
    for (uint8_t i = 0; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 0; j < IMAGE_W - 1; j++)
        {
            if (FASTCorner(img.OutImage, i, j, Threshold))
            {
                DrawCorner(img.GrayImage, i, j);
            }
        }
    }
#endif

#if CannyMode == 1
    // 4. 非极大值抑制
    NMS(&img, Theata);
    // 5. 双阈值过滤
    DoubleThreshold(&img, min, max);
#endif

#if HARRS_CORNER
    // 4. 对M矩阵高斯滤波
    HarrisGussianFilter(M1, M2, M3, GaussianKernel);
    // // 5. 获取Harris响应数值
    GetHarrisPoint(R, M1, M2, M3, k);
    // 6. 非极大值抑制
    HarrisNMS(&img, R, IMAGE_H, IMAGE_W, 3, Threshold);
#endif

    //
    tft180_displayimage03x((const uint8 *)img.GrayImage, IMAGE_W, IMAGE_H);
}

/*****测试函数(方便直接CV)******/
// tft180_show_float(60, 20, P, 3, 3);
// tft180_show_float(60, 35, Q, 3, 3);
// tft180_show_float(60, 50, M1[i][j], 3, 3);
// tft180_show_float(60, 65, M2[i][j], 3, 3);
// tft180_show_float(60, 80, M3[i][j], 3, 3);
// system_delay_ms(500);
/********************************/