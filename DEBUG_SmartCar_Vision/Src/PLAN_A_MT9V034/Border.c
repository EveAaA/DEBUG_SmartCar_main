/**
    @brief: �����汾�����ܱ�Ŀ���ļ��
    @author: ������
    @date: 2024-01-28
*/
#include "Border.h"
#define Threshold_Max 100 // �˲����ɸ����Լ����������
#define Threshold_Min 20 // �˲����ɸ����Լ����������

AT_SDRAM_SECTION_ALIGN(double M1[IMAGE_H][IMAGE_W], 64);
AT_SDRAM_SECTION_ALIGN(double M2[IMAGE_H][IMAGE_W], 64);
AT_SDRAM_SECTION_ALIGN(double M3[IMAGE_H][IMAGE_W], 64);

// ��˹ģ�� 3*3 ��
const int GaussianKernel[3 * 3] = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1};

/**
    @brief: ���ٿ�����
    @return: ���������
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
 * @brief : ���ƽǵ�(�����򶼸ı�)
 * @param : ͼ������, i, j
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
 * @brief ��ȡͼ�� (����Ҫ�ú���, ֱ��memcpy��������)
 *
 * �Ӹ�����ͼ�������л�ȡͼ�����ݣ�������洢��ָ����ͼ��ṹ���С�
 *
 * @param image ͼ��ṹ��ָ��
 * @param mt9v03x_image ͼ������ָ��
 */
/*
void Get_Image(ImageTypeDef *image, uint8_t (*mt9v03x_image)[IMAGE_W])
{
    uint8 i = 0, j = 0, Row = 0, Line = 0;
    for (i = 0; i < IMAGE_H; i++)
    {
        for (j = 0; j < IMAGE_W; j++)
        {
            // ��ȡͼ��
            image->GrayImage[i][j] = mt9v03x_image[i][j];
        }
    }
}
*/

/**
 * @brief : Harris��˹�˲�
 * @param : ��������R, kernel
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
            // ͼ��������
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
    brief: ��˹�˲� (3*3��)
    param: ͼ������ָ��
    param: ������
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
            // ͼ��������
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
    @brief: ˫��ֵ��һ������(canny)
    @param: ͼ��ṹ��, ��������ֵ
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
                // ����ֵ��Χ��
                // ������������
                if (image->OutImage[i - 1][j - 1] == 255 || image->OutImage[i - 1][j] == 255 || image->OutImage[i - 1][j + 1] == 255 || image->OutImage[i][j - 1] == 255 || image->OutImage[i][j + 1] == 255 ||
                    image->OutImage[i + 1][j - 1] == 255 || image->OutImage[i + 1][j] == 255 || image->OutImage[i + 1][j + 1] == 255)
                {
                    // ����Χ����Ϊǿ�߽�
                    image->OutImage[i][j] = 255;
                }
                else
                {
                    image->OutImage[i][j] = 0;
                }
            }
            else
            {
                // ������ֵ�Ǳ߽� Ϊ��ɫ
                image->OutImage[i][j] = 0;
            }
        }
    }
}

/**
  @brief: ��ͼ����߽�����ж�, ��ֹ�����򳬳�ͼ��Χ
  @param: ͼ��ǰ����i, j
  @param: ���������ش洢
  @param: ��ǰͼ��
  @return: none
*/
void writeIntoNeiborPixel(uint8_t *neiborPixel, uint8_t (*img)[IMAGE_W], uint8_t i, uint8_t j)
{
    // ���ϱ߽�
    if (i - 1 >= 0 && j - 1 >= 0)
        neiborPixel[0] = img[i - 1][j - 1];
    // ���б߽�
    if (i - 1 >= 0 && j >= 0)
        neiborPixel[1] = img[i - 1][j + 0];
    // ���ϱ߽�
    if (i - 1 >= 0 && j + 1 < IMAGE_W)
        neiborPixel[2] = img[i - 1][j + 1];
    // ���б߽�
    if (i >= 0 && j - 1 >= 0)
        neiborPixel[3] = img[i + 0][j - 1];
    // �м�
    if (j >= 0 && i >= 0)
        neiborPixel[4] = img[i + 0][j + 0];
    // ���б߽�
    if (i >= 0 && j + 1 < IMAGE_W)
        neiborPixel[5] = img[i + 0][j + 1];
    // ���±߼�
    if (i + 1 < IMAGE_H && j - 1 >= 0)
        neiborPixel[6] = img[i + 1][j - 1];
    // ���±߼�
    if (i + 1 < IMAGE_H && j >= 0)
        neiborPixel[7] = img[i + 1][j + 0];
    // ���±߽�
    if (i + 1 < IMAGE_H && j + 1 <= IMAGE_W)
        neiborPixel[8] = img[i + 1][j + 1];
}

/**
    @brief: �Ǽ���ֵ����
    @param: ͼ��ṹ��ָ�룬���������, �߽���ֵ
    @return: none
*/
void NMS(uint8_t (*img)[IMAGE_W], double (*Theata)[IMAGE_W])
{
    double Ga, Gb;             // ������
    double Ga1, Ga2, Gb1, Gb2; // ����������
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            double Dir = Theata[i][j]; // ��ȡ��ǰ���ط����
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
            // �ж��Ƿ�Ϊ����ֵ
            if (img[i][j] < Ga || img[i][j] < Gb)
            {
                img[i][j] = 0;
            }
        }
    }
}

/**
 * @brief : ��̬ѧ��ʴ�㷨
 * @param : ͼ������
 * @param : ��ʴ�˴�С
 * @return: none
 */
static void dilate(uint8_t (*image)[IMAGE_W], uint8_t size)
{
    uint8_t structElmentHalf = size / 2;
    uint8_t tempImage[IMAGE_H][IMAGE_W]; // ��ʱͼ��
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
    @brief: sobel���ӱ�Ե���
    @param: ͼ��ṹ��ָ��
    @param: ��������ֵ
*/
void SobelfindBorderEdge(ImageTypeDef *image, double (*M1)[IMAGE_W], double (*M2)[IMAGE_W], double (*M3)[IMAGE_W], int16_t k)
{
    double P; // x����ƫ��
    double Q; // y����ƫ��
    double Gradient;
    uint8_t neiborPixel[9]; // ���������ش洢
    for (uint8_t i = 1; i < IMAGE_H - 1; i++)
    {
        for (uint8_t j = 1; j < IMAGE_W - 1; j++)
        {
            // ��ȡ����������
            writeIntoNeiborPixel(neiborPixel, image->GrayImage, i, j);
            // ��x����ƫ��
            P = (double)abs((neiborPixel[2] + neiborPixel[5] * 2 + neiborPixel[8] - neiborPixel[0] - neiborPixel[3] * 2 - neiborPixel[6]));
            // ��y����ƫ��
            Q = (double)abs((neiborPixel[6] + neiborPixel[7] * 2 + neiborPixel[8] - neiborPixel[0] - neiborPixel[1] * 2 - neiborPixel[2]));
            // �洢���������ݶ�����ڳ˻�
            M1[i][j] = P * P;
            M2[i][j] = P * Q;
            M3[i][j] = Q * Q;
            // ��ȡ�����
            // Theata[i][j] = atan(Q / P);
            // �ݶ�
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
 * @brief Harris��Ӧ��ֵ�ķǼ���ֵ����
 * @param R �����������ڴ洢�ǵ���Ӧֵ
 * @param rows ͼ������
 * @param cols ͼ������
 * @param radius �Ǽ���ֵ���ƾֲ����ڰ뾶
 * @param threshold �ǵ���Ӧֵ��ֵ
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
                // �Ǽ���ֵ����
                // image->GrayImage[i][j] = 255; // Ϊ�˿��ӻ�����������һ������ֵ
                DrawCorner(image->GrayImage, i, j);
            }
        }
    }
}

/**
 * @brief: Harris�ǵ���
 * @param: �ǵ���Ӧ����R, �ݶȼ����洢����M
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
            // ��ȡ�ǵ���Ӧֵ
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
 * @brief : FAST�ǵ���
 * @param : ͼ������
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

    // �Ƿ���Χ�д��ڵ���9�������ĵ�
    if (count >= 9)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**@brief   ��̬ѧ�˲�
-- @param   uint8(*Bin_Image)[Image_W] ��ֵ��ͼ��
-- @auther  none
-- @date    2023/10/3
**/
void Image_Filter(uint8 (*Bin_Image)[IMAGE_W]) // ��̬ѧ�˲�������˵�������ͺ͸�ʴ��˼��
{
    uint16 i, j;
    uint32 num = 0;

    for (i = 1; i < IMAGE_H - 1; i++)
    {
        for (j = 1; j < (IMAGE_W - 1); j++)
        {
            // ͳ�ư˸����������ֵ
            num =
                Bin_Image[i - 1][j - 1] + Bin_Image[i - 1][j] + Bin_Image[i - 1][j + 1] + Bin_Image[i][j - 1] + Bin_Image[i][j + 1] + Bin_Image[i + 1][j - 1] + Bin_Image[i + 1][j] + Bin_Image[i + 1][j + 1];

            if (num >= Threshold_Max && Bin_Image[i][j] == 0)
            {
                Bin_Image[i][j] = 255; // ��  ���Ը�ɺ궨�壬�������
            }
            if (num <= Threshold_Min && Bin_Image[i][j] == 255)
            {

                Bin_Image[i][j] = 0; // ��
            }
        }
    }
}

/*************�ⲿ���ú���************/
/*
    @brief: Ѱ��Ŀ���
    @param: �����ͼ������ӿ�
    @param: ˫��ֵmin, max
    @param: ��Ե��ֵedge_threshold
*/
void FindBorder(double k, double Threshold)
{
    ImageTypeDef img;
    double R[IMAGE_H][IMAGE_W]; // ��ȡ�ǵ���Ӧֵ
    // double Theata[IMAGE_H][IMAGE_W]; // �����
    // 1. ��ȡ�Ҷ�ͼ��
    memcpy(img.GrayImage, mt9v03x_image, sizeof(img.GrayImage));
    // 2. ��ͼ���˹ģ��
    GussianFilter(&img, GaussianKernel);
    // 3. ��Ե���
    SobelfindBorderEdge(&img, M1, M2, M3, k);
    // 4. ��ʴ
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
    // 4. �Ǽ���ֵ����
    NMS(&img, Theata);
    // 5. ˫��ֵ����
    DoubleThreshold(&img, min, max);
#endif

#if HARRS_CORNER
    // 4. ��M�����˹�˲�
    HarrisGussianFilter(M1, M2, M3, GaussianKernel);
    // // 5. ��ȡHarris��Ӧ��ֵ
    GetHarrisPoint(R, M1, M2, M3, k);
    // 6. �Ǽ���ֵ����
    HarrisNMS(&img, R, IMAGE_H, IMAGE_W, 3, Threshold);
#endif

    //
    tft180_displayimage03x((const uint8 *)img.GrayImage, IMAGE_W, IMAGE_H);
}

/*****���Ժ���(����ֱ��CV)******/
// tft180_show_float(60, 20, P, 3, 3);
// tft180_show_float(60, 35, Q, 3, 3);
// tft180_show_float(60, 50, M1[i][j], 3, 3);
// tft180_show_float(60, 65, M2[i][j], 3, 3);
// tft180_show_float(60, 80, M3[i][j], 3, 3);
// system_delay_ms(500);
/********************************/