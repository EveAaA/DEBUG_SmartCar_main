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
Flag_Handle Image_Flag = { false };//Ԫ�ر�־λ
/* Define\Declare ------------------------------------------------------------*/
#define use_num     1   //1���ǲ�ѹ����2����ѹ��һ��
uint8_t Original_Image[Image_H][Image_W];//ԭʼͼ������
uint8_t Image_Thereshold;//ͼ��ָ���ֵ
uint8_t Bin_Image[Image_H][Image_W];//��ֵ��ͼ������
uint8_t Start_Point_L[2] = { 0 };//�������x��yֵ
uint8_t Start_Point_R[2] = { 0 };//�ұ�����x��yֵ

#define USE_num Image_H*3   //�����ҵ�������Ա��������˵300�����ܷ��£�������Щ�������ȷʵ�Ѷ����ඨ����һ��
//��ŵ��x��y����
uint16_t Points_L[(uint16_t)USE_num][2] = { {  0 } };//����
uint16_t Points_R[(uint16_t)USE_num][2] = { {  0 } };//����
uint16_t Dir_R[(uint16_t)USE_num] = { 0 };//�����洢�ұ���������
uint16_t Dir_L[(uint16_t)USE_num] = { 0 };//�����洢�����������
uint16_t Data_Stastics_L = 0;//ͳ������ҵ���ĸ���
uint16_t Data_Stastics_R = 0;//ͳ���ұ��ҵ���ĸ���
uint8_t Hightest = 0;//��ߵ�

uint8_t L_Border[Image_H];//��������
uint8_t R_Border[Image_H];//��������
uint8_t Center_Line[Image_H];//��������
uint32_t hashMapIndexL[Image_H];
uint32_t hashMapIndexR[Image_H];

//�������ͺ͸�ʴ����ֵ����
#define Threshold_Max   255*5//�˲����ɸ����Լ����������
#define Threshold_Min   255*2//�˲����ɸ����Լ����������

float Image_Erro;
uint8_t Image_Show = 0;
bool isLeftCircle(uint8_t(*Bin_Image)[Image_W], uint8_t* L_Border, uint8_t* R_Border, uint16_t Total_Num_L, uint16_t Total_Num_R, uint16_t* Dir_L, uint16_t* Dir_R, uint16_t(*Points_L)[2], uint16_t(*Points_R)[2]);
/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**----------------------------------------------------��ѧ��ʽ------------------------------------------------------------------------**/
/**@brief    �����ֵ
-- @param    int value �����ֵ
-- @return   value ����ֵ
-- @auther   none
-- @date     2023/10/2
**/
int My_Abs(int value)
{
    if (value >= 0) return value;
    else return -value;
}


/**@brief    �޷�����
-- @param    int16_t x ��Ҫ�޷���ֵ
-- @param    int a ���ֵ
-- @param    int b ��Сֵ
-- @return   x ���
-- @auther   none
-- @date     2023/10/2
**/
int16_t Limit_a_b(int16_t x, int a, int b)
{
    if (x < a) x = a;
    if (x > b) x = b;
    return x;
}


/**@brief    ��x,y�е���Сֵ
-- @param    int16_t x xֵ
-- @param    int16_t y yֵ
-- @return   ������ֵ�е���Сֵ
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
* @brief ��С���˷�
* @param uint8_t begin				�������
* @param uint8_t end					�����յ�
* @param uint8_t *border				������Ҫ����б�ʵı߽��׵�ַ
*  @see CTest		Slope_Calculate(start, end, border);//б��
* @return ����˵��
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
    if ((end - begin) * x2sum - xsum * xsum) //�жϳ����Ƿ�Ϊ��
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
* @brief ����б�ʽؾ�
* @param uint8_t start				�������
* @param uint8_t *border				������Ҫ����б�ʵı߽�
* @param uint8_t end					�����յ�
* @param float *slope_rate			����б�ʵ�ַ
* @param float *intercept			����ؾ��ַ
*  @see CTest		calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
* @return ����˵��
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

    //�������ƽ����
    if (num)
    {
        x_average = (float)(xsum / num);
        y_average = (float)(ysum / num);

    }

    /*����б��*/
    *slope_rate = Slope_Calculate(start, end, border);//б��
    *intercept = y_average - (*slope_rate) * x_average;//�ؾ�
}

/**@brief    ������б�ʺͽؾ�
-- @param    uint8 x1 
-- @param    int16 y yֵ
-- @return   ������ֵ�е���Сֵ
-- @auther   none
-- @date     2023/10/2
**/
bool Get_K_b(uint8 x1,uint8 y1,uint8 x2,uint8 y2, float* slope_rate, float* intercept)
{
    if (x1 == x2) {
        return false;
    }

    // ����б��
    *slope_rate = (float)(y2 - y1) / (x2 - x1);

    // ����ؾ�
    *intercept = y1 - (*slope_rate * x1);

    return true;
}

/**----------------------------------------------------��򷨲���------------------------------------------------------------------------**/
/**@brief    ��ȡһ���Ҷ�ͼ��
-- @param    uint8_t(*mt9v03x_image)[Image_W] ����ͷ�ɼ�ͼ���Ӧ��ָ��
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

/**@brief    ��򷨻�ȡ��̬��ֵ
-- @param    uint8 *Image ��Ҫ�����ͼ��
-- @param    uint16 col �г���
-- @param    uint16 row �г���
-- @auther   none
-- @return   uint8_t Threshold ��̬��ֵ
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

/**@brief    ���ͼ���ֵ��
-- @param    ��
-- @auther   none
-- @date     2023/10/2
**/
void Turn_To_Bin(void)
{
    uint8_t i, j;
    Image_Thereshold = 1.075*Otsu_Threshold(Original_Image[0], Image_W, Image_H);//��ȡ�����ֵ
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


/**----------------------------------------------------�����򲿷�------------------------------------------------------------------------**/
/**@brief    Ѱ�������߽�ı߽����Ϊ������ѭ������ʼ��
-- @param    uint8_t Start_Row ������������
-- @auther   none
-- @date     2023/10/3
**/
uint8_t Get_Start_Point(uint8_t Start_Row)
{
    uint8_t i = 0, L_Found = 0, R_Found = 0;
    //����
    Start_Point_L[0] = 0;//x
    Start_Point_L[1] = 0;//y

    Start_Point_R[0] = 0;//x
    Start_Point_R[1] = 0;//y

    //���м�����ߣ��������
    for (i = Image_W / 2; i > Border_Min; i--)
    {
        Start_Point_L[0] = i;//x
        Start_Point_L[1] = Start_Row;//y
        if (Bin_Image[Start_Row][i] == 255 && Bin_Image[Start_Row][i - 1] == 0)//�ҵ��ڰ������
        {
            //printf("�ҵ�������image[%d][%d]\n", Start_Row,i);
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
            //printf("�ҵ��ұ����image[%d][%d]\n",Start_Row, i);
            R_Found = 1;
            break;
        }
    }

    if (L_Found && R_Found)//��ߺ��ұ߶��ҵ����
    {
        return 1;
    }
    else
    {
        //printf("δ�ҵ����\n");
        return 0;
    }
}


/**@brief    �����������ұߵ�ĺ���
-- @param    uint16_t_t Break_Flag �����Ҫѭ���Ĵ���
-- @param    uint8_t(*image)[Image_W] ��Ҫ�����ҵ��ͼ�����飬�����Ƕ�ֵͼ,�����������Ƽ���
-- @param    uint16_t_t *L_Stastic ͳ��������ݣ����������ʼ�����Ա����ź�ȡ��ѭ������
-- @param    uint16_t_t *R_Stastic ͳ���ұ����ݣ����������ʼ�����Ա����ź�ȡ��ѭ������
-- @param    uint8_t L_Start_X �����������
-- @param    uint8_t L_Start_Y ������������
-- @param    uint8_t R_Start_X �ұ���������
-- @param    uint8_t R_Start_Y �ұ����������
-- @param    uint8_t Hightest ѭ���������õ�����߸߶�
-- @auther   none
-- @date     2023/10/3
**/
void Search_L_R(uint16_t Break_Flag, uint8_t(*image)[Image_W], uint16_t* L_Stastic, uint16_t* R_Stastic, uint8_t L_Start_X, uint8_t L_Start_Y, uint8_t R_Start_X, uint8_t R_Start_Y, uint8_t* Hightest)
{

    uint8_t i = 0, j = 0;

    //��߱���
    uint8_t Search_Filds_L[8][2] = { {0} };//Ѱ�Ұ���������
    uint8_t Index_L = 0;
    uint8_t Temp_L[8][2] = { {0} };
    uint8_t Center_Point_L[2] = { 0 };
    uint16_t L_Data_Statics;//ͳ������ҵ��ĵ�
    //����˸�����
    static int8_t Seeds_L[8][2] = { {0,  1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,  0},{1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //�����˳ʱ��

    //�ұ߱���
    uint8_t Search_Filds_R[8][2] = { {  0 } };
    uint8_t Center_Point_R[2] = { 0 };//���������
    uint8_t Index_R = 0;//�����±�
    uint8_t Temp_R[8][2] = { {  0 } };
    uint16_t R_Data_Statics;//ͳ���ұ�
    //����˸�����
    static int8_t Seeds_R[8][2] = { {0,  1},{1,1},{1,0}, {1,-1},{0,-1},{-1,-1}, {-1,  0},{-1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //�������ʱ��

    L_Data_Statics = *L_Stastic;//ͳ���ҵ��˶��ٸ��㣬��������ѵ�ȫ��������
    R_Data_Statics = *R_Stastic;//ͳ���ҵ��˶��ٸ��㣬��������ѵ�ȫ��������

    //��һ�θ�������㣬��㣬����һ���ҵ������ֵ������
    Center_Point_L[0] = L_Start_X;//x
    Center_Point_L[1] = L_Start_Y;//y
    Center_Point_R[0] = R_Start_X;//x
    Center_Point_R[1] = R_Start_Y;//y

    //��������ѭ��
    while (Break_Flag--)
    {

        //���
        for (i = 0; i < 8; i++)//���ݰ���������
        {
            Search_Filds_L[i][0] = Center_Point_L[0] + Seeds_L[i][0];//x
            Search_Filds_L[i][1] = Center_Point_L[1] + Seeds_L[i][1];//y
        }
        //�����������䵽�Ѿ��ҵ��ĵ���
        Points_L[L_Data_Statics][0] = Center_Point_L[0];//x
        Points_L[L_Data_Statics][1] = Center_Point_L[1];//y
        L_Data_Statics++;//������һ

        //�ұ�
        for (i = 0; i < 8; i++)//���ݰ���������
        {
            Search_Filds_R[i][0] = Center_Point_R[0] + Seeds_R[i][0];//x
            Search_Filds_R[i][1] = Center_Point_R[1] + Seeds_R[i][1];//y
        }
        //�����������䵽�Ѿ��ҵ��ĵ���
        Points_R[R_Data_Statics][0] = Center_Point_R[0];//x
        Points_R[R_Data_Statics][1] = Center_Point_R[1];//y

        Index_L = 0;//�����㣬��ʹ��
        for (i = 0; i < 8; i++)
        {
            Temp_L[i][0] = 0;//�����㣬��ʹ��
            Temp_L[i][1] = 0;//�����㣬��ʹ��
        }

        //����ж�
        //Search_Filds_L[i][1]��ʾ��������Xֵ��Search_Filds_L[i][0]]��ʾ��������Yֵ
        for (i = 0; i < 8; i++)
        {
            if (image[Search_Filds_L[i][1]][Search_Filds_L[i][0]] == 0
                && image[Search_Filds_L[(i + 1) & 7][1]][Search_Filds_L[(i + 1) & 7][0]] == 255)//�������ں�λ���ںڰ������
            {
                Temp_L[Index_L][0] = Search_Filds_L[(i)][0];//��¼�µ�ǰ�ڵ�
                Temp_L[Index_L][1] = Search_Filds_L[(i)][1];
                Index_L++;
                Dir_L[L_Data_Statics - 1] = (i);//��¼��������
                // printf("dir[%d]:%d\n", L_Data_Statics - 1, Dir_L[L_Data_Statics - 1]);
            }

            if (Index_L)
            {
                //��������㣬���µ����ĵ㿪ʼ������
                Center_Point_L[0] = Temp_L[0][0];//x
                Center_Point_L[1] = Temp_L[0][1];//y
                for (j = 0; j < Index_L; j++)
                {
                    if (Center_Point_L[1] > Temp_L[j][1])//�ж�һ�±����ǲ��������ߵ�
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
            //printf("���ν���ͬһ���㣬�˳�\n");
            break;
        }
        if (My_Abs(Points_R[R_Data_Statics][0] - Points_L[L_Data_Statics - 1][0]) < 2
            && My_Abs(Points_R[R_Data_Statics][1] - Points_L[L_Data_Statics - 1][1] < 2)
            )
        {
            //printf("\n���������˳�\n");
            *Hightest = (Points_R[R_Data_Statics][1] + Points_L[L_Data_Statics - 1][1]) >> 1;//ȡ����ߵ�
            //printf("\n��y=%d���˳�\n",*Hightest);
            break;
        }
        if ((Points_R[R_Data_Statics][1] < Points_L[L_Data_Statics - 1][1]))
        {
            // printf("\n�����߱��ұ߸��ˣ���ߵȴ��ұ�\n");
            continue;//�����߱��ұ߸��ˣ���ߵȴ��ұ�
        }
        if (Dir_L[L_Data_Statics - 1] == 7
            && (Points_R[R_Data_Statics][1] > Points_L[L_Data_Statics - 1][1]))//��߱��ұ߸����Ѿ�����������
        {
            //printf("\n��߿�ʼ�����ˣ��ȴ��ұߣ��ȴ���... \n");
            Center_Point_L[0] = Points_L[L_Data_Statics - 1][0];//x
            Center_Point_L[1] = Points_L[L_Data_Statics - 1][1];//y
            L_Data_Statics--;
        }
        R_Data_Statics++;//������һ

        Index_R = 0;//�����㣬��ʹ��
        for (i = 0; i < 8; i++)
        {
            Temp_R[i][0] = 0;//�����㣬��ʹ��
            Temp_R[i][1] = 0;//�����㣬��ʹ��
        }

        //�ұ��ж�
        for (i = 0; i < 8; i++)
        {
            if (image[Search_Filds_R[i][1]][Search_Filds_R[i][0]] == 0
                && image[Search_Filds_R[(i + 1) & 7][1]][Search_Filds_R[(i + 1) & 7][0]] == 255)
            {
                Temp_R[Index_R][0] = Search_Filds_R[(i)][0];
                Temp_R[Index_R][1] = Search_Filds_R[(i)][1];
                Index_R++;//������һ
                Dir_R[R_Data_Statics - 1] = (i);//��¼��������
                // printf("dir[%d]:%d\n", R_Data_Statics - 1, Dir_R[R_Data_Statics - 1]);
            }
            if (Index_R)
            {

                //���������
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

    //ȡ��ѭ������
    *L_Stastic = L_Data_Statics;
    *R_Stastic = R_Data_Statics;

}

/**@brief   �Ӱ�����߽�����ȡ��Ҫ�������
-- @param   uint16_t_t Total_L �ҵ��ĵ������
-- @auther  none
-- @date    2023/10/3
**/
void Get_Left(uint16_t Total_L)
{
    uint8_t i = 0;
    uint16_t j = 0;
    uint8_t h = 0;
    //��ʼ��
    for (i = 0;i < Image_H;i++)
    {
        L_Border[i] = Border_Min;
    }
    h = Image_H - 2;
    //���
    for (j = 0; j < Total_L; j++)
    {
        //printf("%d\n", j);
        if (Points_L[j][1] == h)
        {
            L_Border[h] = Points_L[j][0] + 1;
            hashMapIndexL[h] = j; // ����Edge��Ե�����ϣ��
        }
        else
        {
            continue;//ÿ��ֻȡһ���㣬û����һ�оͲ���¼
        }
        h--;
        if (h == 0)
        {
            break;//�����һ���˳�
        }
    }
}

/**@brief   �Ӱ�����߽�����ȡ��Ҫ���ұ���
-- @param   uint16_t_t Total_R �ҵ��ĵ������
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
        R_Border[i] = Border_Max;//�ұ��߳�ʼ���ŵ����ұߣ�����߷ŵ�����ߣ�����������պ�����������߾ͻ����м䣬������ŵõ�������
    }
    h = Image_H - 2;
    //�ұ�
    for (j = 0; j < Total_R; j++)
    {
        if (Points_R[j][1] == h)
        {
            R_Border[h] = Points_R[j][0] - 1;
            hashMapIndexR[h] = j;
        }
        else
        {
            continue;//ÿ��ֻȡһ���㣬û����һ�оͲ���¼
        }
        h--;
        if (h == 0)
        {
            break;//�����һ���˳�
        }
    }
}


/**----------------------------------------------------ͼ���˲�����------------------------------------------------------------------------**/
/**@brief   ��̬ѧ�˲�
-- @param   uint8_t(*Bin_Image)[Image_W] ��ֵ��ͼ��
-- @auther  none
-- @date    2023/10/3
**/
void Image_Filter(uint8_t(*Bin_Image)[Image_W])//��̬ѧ�˲�������˵�������ͺ͸�ʴ��˼��
{
    uint16_t i, j;
    uint32_t num = 0;


    for (i = 1; i < Image_H - 1; i++)
    {
        for (j = 1; j < (Image_W - 1); j++)
        {
            //ͳ�ư˸����������ֵ
            num =
                Bin_Image[i - 1][j - 1] + Bin_Image[i - 1][j] + Bin_Image[i - 1][j + 1]
                + Bin_Image[i][j - 1] + Bin_Image[i][j + 1]
                + Bin_Image[i + 1][j - 1] + Bin_Image[i + 1][j] + Bin_Image[i + 1][j + 1];


            if (num >= Threshold_Max && Bin_Image[i][j] == 0)
            {

                Bin_Image[i][j] = 255;//��  ���Ը�ɺ궨�壬�������

            }
            if (num <= Threshold_Min && Bin_Image[i][j] == 255)
            {

                Bin_Image[i][j] = 0;//��

            }

        }
    }

}


/**@brief   ��ͼ��һ���ڿ�
-- @param   uint8_t(*Image)[Image_W]  ͼ���׵�ַ
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
    //for(i = 0; i < Compress_H; i++)//��ĸ��������Ա���ȫ��ͼ��
    for (i = yStart; i < yEnd; i++)   //�е����Ծ
    {
        //for(j = 0; j < Compress_W; j++)//��ĸ��������Ա���ȫ��ͼ��
        for (j = xStart; j < xEnd; j++)  //�е����Ծ
        {
            /* ���㲻ͬ�����ݶȷ�ֵ  */
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

/**----------------------------------------------------Ԫ�ز���------------------------------------------------------------------------**/
/**
* @brief ʮ�ֲ��ߺ���
* @param uint8_t(*Bin_Image)[Image_W]		�����ֵͼ��
* @param uint8_t *L_Border			������߽��׵�ַ
* @param uint8_t *R_Border			�����ұ߽��׵�ַ
* @param uint16_t_t Total_Num_L			�������ѭ���ܴ���
* @param uint16_t_t Total_Num_R			�����ұ�ѭ���ܴ���
* @param uint16_t_t *Dir_L				����������������׵�ַ
* @param uint16_t_t *Dir_R				�����ұ����������׵�ַ
* @param uint16_t_t(*Points_L)[2]		������������׵�ַ
* @param uint16_t_t(*Points_R)[2]		�����ұ������׵�ַ
* @see CTest		Cross_Fill(Bin_Image,L_Border, R_Border, data_statics_l, data_statics_r, Dir_L, Dir_R, Points_L, Points_R);
 */
void Cross_Fill(uint8_t(*Bin_Image)[Image_W],  uint8_t* L_Border, uint8_t* R_Border, uint16_t Total_Num_L, uint16_t Total_Num_R, uint16_t* Dir_L, uint16_t* Dir_R, uint16_t(*Points_L)[2], uint16_t(*Points_R)[2])
{
    uint8_t i;
    uint8_t Break_Num_L_UP = 0;//�յ�
    uint8_t Break_Num_R_UP = 0;
    uint8_t Break_Num_L_DOWN = 0;
    uint8_t Break_Num_R_DOWN = 0;
    uint8_t start, end;
    float slope_l_rate = 0, intercept_l = 0;

    for (i = 1; i < Total_Num_L; i++)
    {
        if (Dir_L[i - 1] == 2 && Dir_L[i] == 2 && Dir_L[i + 3] == 4 && Dir_L[i + 5] == 4 && Dir_L[i + 7] == 4)//2-4���䣬���¹յ�
        {
            Break_Num_L_DOWN = Points_L[i][1];//����y����
            // printf("find l_down\r\n");
            // printf("%d %d\r\n",i,Break_Num_L_DOWN);
            // if(Menu.Image_Show)
            // {
            //     tft180_Draw_ColorCircle(L_Border[Break_Num_L_DOWN],Break_Num_L_DOWN,5,RGB565_BLUE);//�����յ�
            // }
            break;
        }
    }
    for (i = 1; i < Total_Num_R; i++)
    {
        // printf("DIR_R[%d] = %d\r\n",i,Dir_R[i]);
        if (Dir_R[i - 1] == 2 && Dir_R[i] == 2 && Dir_R[i + 3] == 4 && Dir_R[i + 5] == 4 && Dir_R[i + 7] == 4)//2-4���䣬���¹յ�
        {
            Break_Num_R_DOWN = Points_R[i][1];//����y����
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
        if (Dir_L[i - 1] == 4 && Dir_L[i] == 4 && Dir_L[i + 3] == 6 && Dir_L[i + 5] == 6 && Dir_L[i + 7] == 6)//4-6���䣬���Ϲյ�
        {
            Break_Num_L_UP = Points_L[i][1];//����y����
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
        if (Dir_R[i - 1] == 4 && Dir_R[i] == 4 && Dir_R[i + 3] == 6 && Dir_R[i + 5] == 6 && Dir_R[i + 7] == 6)//4-6���䣬���Ϲյ�
        {
            Break_Num_R_UP = Points_R[i][1];//����y����
            // printf("find R_up\r\n");
            // printf("%d %d\r\n",R_Border[Break_Num_R_UP - 10],Break_Num_R_UP);
            // if(Menu.Image_Show)
            // {
            //     tft180_Draw_ColorCircle(R_Border[Break_Num_R_UP - 10],Break_Num_R_UP,5,RGB565_RED);
            // }
            break;
        }
    }

    if (Break_Num_L_DOWN && Break_Num_R_DOWN && (Bin_Image[Image_H - 10][4] == 0) && (Bin_Image[Image_H - 10][Image_W - 4] == 0))//ʮ��ǰ
    {
        Image_Flag.Cross_Fill = true;
        //����б��,���б��
        start = Break_Num_L_DOWN + 2;
        end = Break_Num_L_DOWN + 7;
        end = Limit_a_b(end, 0, Image_H);//�޷�

        calculate_s_i(start, end, L_Border, &slope_l_rate, &intercept_l);
        // printf("slope_l_rate:%f\nintercept_l:%f\n", slope_l_rate, intercept_l);
        for (i = Break_Num_L_DOWN; i < Image_H - 1; i--)
        {
            L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//�޷�
        }

        //����б��,�ұ�б��
        start = Break_Num_R_DOWN + 2;
        end = Break_Num_R_DOWN + 7;
        end = Limit_a_b(end, 0, Image_H);//�޷�
        calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
        //printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
        for (i = Break_Num_R_DOWN + 2; i < Image_H - 1; i--)
        {
            R_Border[i] = slope_l_rate * (i)+intercept_l;
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);
        }
        // tft180_show_string(Row_10,Line_7,"fuck");
    }
    else if (Break_Num_L_UP && Break_Num_R_UP && Bin_Image[Image_H - 10][4] && Bin_Image[Image_H - 10][Image_W - 4])//ʮ����
    {
        Image_Flag.Cross_Fill = true;
        //����б��
        start = Break_Num_L_UP - 15;
        start = Limit_a_b(start, 0, Image_H);
        end = Break_Num_L_UP - 5;
        calculate_s_i(start, end, L_Border, &slope_l_rate, &intercept_l);
        // printf("slope_l_rate:%f\nintercept_l:%f\n", slope_l_rate, intercept_l);
        for (i = Break_Num_L_UP - 5; i < Image_H - 1; i++)
        {
            L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
            L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//�޷�
        }

        //����б��
        start = Break_Num_R_UP - 15;//���
        start = Limit_a_b(start, 0, Image_H);//�޷�
        end = Break_Num_R_UP - 5;//�յ�
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


/**@brief    �ж��Ƿ�Ϊֱ��
-- @param    uint16 Border ��Ҫ�жϵ���
-- @param    uint16 Total_Num ����
-- @return   �Ƿ�Ϊֱ��
-- @auther   none
-- @date     2023/10/2
**/
bool Straight_Line_Judge(uint8* Border, uint16 Total_Num, lineTypeDef lineMode)
{
    uint8_t StraightPoint = 0; // ��¼��ֱ�ߵĵ�ĸ���
    // ������鳤��Ϊ0��1��ֱ�ӷ���1
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

    // �������飬����Ƿ񵥵��ݼ�
    for (int i = Image_H - 2; i > Hightest; i-= 1) {
        // �����ǰԪ��С�ڻ����ǰһ��Ԫ�أ����ǵ����ݼ���
        // uint8_t before = Limit_a_b(i - 1, Hightest, Image_H - 2);
        // uint8_t next = Limit_a_b(i - 1, Hightest, Image_H - 2);
       // printf("��ǰ: % d ��һ���� : % d \n", Border[i], Border[i - 1]);
        if (lineMode == RightLine)
        {             
            if (Border[i] - Border[i - 1] >= 0 && Border[i] - Border[i - 1] < 2)
            {
                StraightPoint += 1;
                //circle(frame, Point(Border[i], i), 0, Scalar(255, 0, 0), 1);
            }
            // ���ѵ�ֱ������
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
            //imshow("ԭͼ��", frame);
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
    // ���û�з���Υ�����ϵ����������true
    //return true;
}


uint16 Find_Salient_Point(uint8* Border, uint16 size)
{
    int maxIndex = 0;

    // �������飬�ҵ����ֵ������
    for (int i = size; i < Image_H - 1; i++) {
        if (Border[i] > Border[maxIndex]) {
            maxIndex = i;
        }
    }

    return maxIndex;
}


uint16_t findCircleOutPoint(uint8_t* L_Border)
{
    // Ѱ��Բ���Ҷ���
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
        //imshow("ԭͼ��", frame);
        //waitKey(5);
    }
    return NULL;
}

// ��Բ��
//void Right_Ring(uint8_t(*Bin_Image)[Image_W], uint8_t* L_Border, uint8* R_Border, uint16 Total_Num_L, uint16 Total_Num_R,
//    uint16* Dir_L, uint16* Dir_R, uint16(*Points_L)[2], uint16(*Points_R)[2])
//{
//    int i;
//    static uint8_t WhitePixel_Time = 0;
//    uint8_t hashKey[Image_H] = { 0 };
//    uint8_t offset = 0;
//    uint8_t StraightNum = 0;
//    uint8_t circlePoint[2];
//    uint16 Break_Num_R_UP = 0;//�յ�
//    uint16 Break_Num_R_DOWN = 0;
//    uint16 Salient_Point = 0;//Բ��͹��
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
    uint16 Break_Num_L_UP = 0;//�յ�
    uint16 Break_Num_L_DOWN = 0;
    uint16 Salient_Point = 0;//Բ��͹��
    uint16_t Break_Num_R_UP = 0;
    uint16 start, end;
    float slope_l_rate = 0, intercept_l = 0;
    static float Curanglg = 0;
    static float Lastanglg = 0;
    static float Angle_Offest = 0;
    //LeftRing.Ring_State = Enter_Ring_First;//�뻷��
    switch (LeftRing.Ring_State)
    {
    case Ring_Front:
        // printf("�뻷ǰ\r\n");
        //cout << "�뻷ǰ" << endl;
        LeftRing.Stright_Line = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//�ж��ұ��Ƿ�Ϊ��ֱ��
        Salient_Point = findCircleOutPoint(L_Border);
        // tft180_Draw_ColorCircle(L_Border[Salient_Point],Salient_Point,5,RGB565_RED);
        for (uint8_t i = Image_H - 2; i > Image_H / 2; --i)
        {
            // ����������������ܴ�
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
        // ��Բ��ͻ���� ��ֱ�� û�����½ǵ�
        if (Salient_Point && LeftRing.Stright_Line && !Break_Num_L_DOWN && LeftRing.Ring_Front_Flag) // �ж��Ƿ���Բ��ͻ���㲢���Ҳ�ֱ��
        {
            Get_K_b(L_Border[Salient_Point], Salient_Point, 0, Image_H - 1, &slope_l_rate, &intercept_l);
            for (i = Image_H - 1; i > 1; i--)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//�޷�
            }
        }
        //��Բ��ͻ����, ��ֱ��, �����½ǵ�
        else if (Break_Num_L_DOWN && LeftRing.Stright_Line && Salient_Point) //  && (!Bin_Image[Image_H - 10][4]) && (!Bin_Image[Image_H - 10][Image_W - 4])
        {
            //����б��,���б��
            Get_K_b(Salient_Point, L_Border[Salient_Point], Points_L[Break_Num_L_DOWN][1], Points_L[Break_Num_L_DOWN][0], &slope_l_rate, &intercept_l);
            for (i = Salient_Point; i < Points_L[Break_Num_L_DOWN][1]; i++)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//�޷�
            }
            LeftRing.Ring_Front_Flag = true;
        }
        // û��Բ��ͻ����, ��ֱ�ߣ� �����½ǵ�
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
        // ���¿հ� �Ҽ�⵽�����½ǵ�, ��ֱ��, ��Բ��ͻ����, �ұ�����Ϊ��ɫ, �ұ��·�Ϊ��ɫ
        if ((Bin_Image[Image_H - 5][4]) && (Bin_Image[Image_H - 10][8]) && LeftRing.Ring_Front_Flag &&
            (!Bin_Image[Image_H / 2][Image_W - 2]) && (!Bin_Image[Image_H-5][Image_W - 2]))//ͼ�����·�ΪһƬ�� (Bin_Image[Image_H - 5][4]) && (Bin_Image[Image_H - 10][8]) &&
        {
            LeftRing.Ring_Front_Flag = false;
            LeftRing.Ring = true;
            LeftRing.Ring_State = Enter_Ring_First;//�뻷�� Enter_Ring_First
        }
        //imshow("ԭͼ��", frame);
        // waitKey(200);
        break;
    case Enter_Ring_First:
        // printf("�뻷��\r\n");
        LeftRing.Stright_Line = Straight_Line_Judge(R_Border, Total_Num_R - 10, RightLine);//�ж��ұ��Ƿ�Ϊ��ֱ��
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
                Break_Num_L_UP = i;//����y����
                break;
            }
        }
        if (Salient_Point && LeftRing.Stright_Line) // �ж��Ƿ���Բ��ͻ���㲢���Ҳ�ֱ��
        {
            Get_K_b(L_Border[Salient_Point], Salient_Point, 0, Image_H - 1, &slope_l_rate, &intercept_l);
            for (i = Image_H - 1; i > 1; i--)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//�޷�
            }
            LeftRing.Enter_Ring_First_Flag = true;
        }
        // ��һ���뻷��־λ, �ҵ��Ͻǵ�, �Ҳ���Բ��ͻ����, ��ֱ��
        if (LeftRing.Enter_Ring_First_Flag && Salient_Point == NULL && Break_Num_L_UP)    //&& (Bin_Image[20][4]) && (Bin_Image[20][8]) && (Bin_Image[Image_H - 2][4]) && (Bin_Image[Image_H - 2][8])
        {
            LeftRing.Enter_Ring_First_Flag = false;
            LeftRing.Ring_State = Leave_Ring_First;//��һ���뿪��
        }
        break;
    case Leave_Ring_First:
        // printf("��һ���뿪��\r\n");
        // cout << "��һ���뿪��" << endl;
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
                Break_Num_L_UP = i;//����y����
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
                R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//�޷�
            }
            LeftRing.Leave_Ring_First_Flag = true;
        }
        // ����һƬ�հ�, �Ҳ������Ͻǵ�
        if (Bin_Image[Image_H - 10][20] && Bin_Image[Image_H - 10][25] && !Break_Num_L_UP)
        {
            LeftRing.Leave_Ring_First_Flag = true;
            LeftRing.Ring_State = In_Ring;
        }
        break;
    case In_Ring:
        // printf("����\r\n");
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
                Break_Num_R_UP = i;//����y����
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
                R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//�޷�
            }
        }
        Lastanglg = Curanglg;
        // �������඼Ϊ��ɫ, ׼��Ҫ����
        if (Bin_Image[Image_H - 4][4] == White_Pixel && Bin_Image[Image_H - 4][Image_W - 4] == White_Pixel && fabs(Angle_Offest) >= 50)
        {
            LeftRing.Ring_State = Ready_Out_Ring;
            Angle_Offest = 0;
            Lastanglg = 0;
        }
        
        break;
    case Ready_Out_Ring:
        // printf("׼������\r\n");
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
            R_Border[i] = Limit_a_b(R_Border[i], Border_Min, Border_Max);//�޷�
        }
        Lastanglg = Curanglg;
        // �Ҳ������߳����������������ֱ��, ׼������
        if (Data_Stastics_R > Data_Stastics_L && fabs(Angle_Offest) > 35)
        {
            LeftRing.Ring_State = Leave_Ring;
            Angle_Offest = 0;
            Lastanglg = 0;
        }
        
        break;
    case Leave_Ring:
        // printf("����\r\n");
        Break_Num_L_UP = 0;
        //cout << "������" << ;
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
                Break_Num_L_UP = i;//����y����
                break;
            }
        }

        if (LeftRing.Leave_Ring_First_Flag && Break_Num_L_UP)
        {
            Get_K_b(Points_L[Break_Num_L_UP][1], Points_L[Break_Num_L_UP][0], Image_H - 1, 2, &slope_l_rate, &intercept_l);
            for (i = Points_L[Break_Num_L_UP][1]; i <= Image_H - 1; ++i)
            {
                L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
                L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//�޷�
            }
        }
        // ��⵽�Ѿ����������ı�־λ, ���½�Ϊ��ɫ, ���ҿ������Ͻǵ�, �����Ѿ�����
        else if (LeftRing.Leave_Ring_First_Flag && Bin_Image[Image_H - 4][4] == Black_Pixel && Bin_Image[Image_H - 2][Image_W - 4] == Black_Pixel 
                && !Break_Num_L_UP)
        {
            LeftRing.Ring_Front_Flag = false; // ����
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
    Get_Image(mt9v03x_image);//��ȡһ��ͼ��
    // timer_start(GPT_TIM_1);
    // my_sobel(Original_Image,Bin_Image);
    Turn_To_Bin();//��ֵ��
}

/**@brief   ���յ��õ�ͼ����ĺ���
-- @param   ��
-- @auther  none
-- @date    2023/10/3
**/
void Image_Process(void)
{
    uint16_t i;
    // timer_start(GPT_TIM_1);
    // my_sobel(Original_Image,Bin_Image);
    Get_Image(mt9v03x_image);
    Turn_To_Bin();//��ֵ��

    Image_Filter(Bin_Image);//�˲�
    Image_Draw_Rectan(Bin_Image);//Ԥ����

    //����
    Data_Stastics_L = 0;
    Data_Stastics_R = 0;

    if (Get_Start_Point(Image_H - 2))//��ͼ��������濪ʼ�ң��ҵ�����ˣ���ִ�а�����û�ҵ���һֱ��
    {
        //        printf("���ڿ�ʼ������\n");
        Search_L_R((uint16_t)USE_num, Bin_Image, &Data_Stastics_L, &Data_Stastics_R, Start_Point_L[0], Start_Point_L[1], Start_Point_R[0], Start_Point_R[1], &Hightest);
        //        printf("�������ѽ���\n");
                // ����ȡ�ı߽�������ȡ���� �� ��������������õı���
        Get_Left(Data_Stastics_L);
        Get_Right(Data_Stastics_R);
        // �����ж��Ƿ���ʮ�������ʮ���򲻶�Բ���ж�
        if (!LeftRing.Ring)
        {
            Cross_Fill(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);//ʮ�ֲ���
        }
        // ͬ��
        //Cross_Fill(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);//ʮ�ֲ���
        if (!Image_Flag.Cross_Fill)
        {
            Left_Ring(Bin_Image, L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);
        }
    }
    
    for (int i = Hightest; i < Image_H-1; i++)
    {
        Center_Line[i] = (L_Border[i] + R_Border[i]) >> 1;//������
    }
    Image_Erro = (Center_Line[69])*0.375f + (Center_Line[70])*0.5f + (Center_Line[71])*0.1f;
    Image_Flag.Cross_Fill = false;
}