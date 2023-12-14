/**
  ******************************************************************************
  * @file    Image.c
  * @author  ׯ�ı�
  * @brief   ͼ�񲿷�
  *
    @verbatim

    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Image.h"


/* Define\Declare ------------------------------------------------------------*/
#define use_num     1   //1���ǲ�ѹ����2����ѹ��һ��
uint8 Original_Image[Image_H][Image_W];//ԭʼͼ������
uint8 Image_Thereshold;//ͼ��ָ���ֵ
uint8 Bin_Image[Image_H][Image_W];//��ֵ��ͼ������
uint8 Start_Point_L[2] = { 0 };//�������x��yֵ
uint8 Start_Point_R[2] = { 0 };//�ұ�����x��yֵ

#define USE_num Image_H*3   //�����ҵ�������Ա��������˵300�����ܷ��£�������Щ�������ȷʵ�Ѷ����ඨ����һ��
//��ŵ��x��y����
uint16 Points_L[(uint16)USE_num][2] = { {  0 } };//����
uint16 Points_R[(uint16)USE_num][2] = { {  0 } };//����
uint16 Dir_R[(uint16)USE_num] = { 0 };//�����洢�ұ���������
uint16 Dir_L[(uint16)USE_num] = { 0 };//�����洢�����������
uint16 Data_Stastics_L = 0;//ͳ������ҵ���ĸ���
uint16 Data_Stastics_R = 0;//ͳ���ұ��ҵ���ĸ���
uint8 Hightest = 0;//��ߵ�

uint8 L_Border[Image_H];//��������
uint8 R_Border[Image_H];//��������
uint8 Center_Line[Image_H];//��������

//�������ͺ͸�ʴ����ֵ����
#define Threshold_Max   255*5//�˲����ɸ����Լ����������
#define Threshold_Min   255*2//�˲����ɸ����Լ����������

double Image_Erro;
/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/


/**@brief    �����ֵ
-- @param    int value �����ֵ
-- @return   value ����ֵ
-- @auther   ׯ�ı�
-- @date     2023/10/2
**/
int My_Abs(int value)
{
    if(value>=0) return value;
    else return -value;
}


/**@brief    �޷�����
-- @param    int16 x ��Ҫ�޷���ֵ
-- @param    int a ���ֵ
-- @param    int b ��Сֵ
-- @return   x ���
-- @auther   ׯ�ı�
-- @date     2023/10/2
**/
int16 Limit_a_b(int16 x, int a, int b)
{
    if(x<a) x = a;
    if(x>b) x = b;
    return x;
}


/**@brief    ��x,y�е���Сֵ
-- @param    int16 x xֵ
-- @param    int16 y yֵ
-- @return   ������ֵ�е���Сֵ
-- @auther   ׯ�ı�
-- @date     2023/10/2
**/
int16 limit1(int16 x, int16 y)
{
    if (x > y)             return y;
    else if (x < -y)       return -y;
    else                return x;
}


/**----------------------------------------------------��򷨲���------------------------------------------------------------------------**/
/**@brief    ��ȡһ���Ҷ�ͼ��
-- @param    uint8(*mt9v03x_image)[Image_W] ����ͷ�ɼ�ͼ���Ӧ��ָ��
-- @auther   ׯ�ı�
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

/**@brief    ��򷨻�ȡ��̬��ֵ
-- @param    uint8 *image ��Ҫ�����ͼ��
-- @param    uint16 col �г���
-- @param    uint16 row �г���
-- @auther   ׯ�ı�
-- @return   uint8 Threshold ��̬��ֵ
-- @date     2023/10/2
**/
uint8 Otsu_Threshold(uint8 *Image, uint16 col, uint16 row)
{
#define GrayScale 256
    uint16 Image_Width  = col;
    uint16 Image_Height = row;
    int X; uint16 Y;
    uint8* data = Image;
    int HistGram[GrayScale] = {0};//ֱ��ͼ

    uint32 Amount = 0;
    uint32 Pixel_Back = 0;//��������
    uint32 Pixel_Integral_Back = 0;//�������ص��ۻ��Ҷ�ֵ
    uint32 Pixel_Integral = 0;//�����ۼ�ֵ
    int32 Pixel_IntegralFore = 0;//ǰ�����ص��ۻ��Ҷ�ֵ
    int32 Pixel_Fore = 0;//ǰ�����أ�����Ϊ��Ŀ�������
    double Omega_Back=0, Omega_Fore=0, Micro_Back=0, Micro_Fore=0, SigmaB=0, Sigma=0; // ��䷽��ֱ�Ϊ�������ص����Ȩ�أ�ǰ�����ص����Ȩ�أ��������ص�ƽ���Ҷ�ֵ��ǰ�����ص�ƽ���Ҷ�ֵ;
    uint8 Min_Value=0, Max_Value=0;
    uint8 Threshold = 0;//��ֵ


    for (Y = 0; Y <Image_Height; Y++) //Y<Image_Height��ΪY =Image_Height���Ա���� �ж�ֵ��
    {
        //Y=Image_Height;
        for (X = 0; X < Image_Width; X++)
        {
            HistGram[(int)data[Y*Image_Width + X]]++; //ͳ��ÿ���Ҷ�ֵ�ĸ�����Ϣ
        }
    }




    for (Min_Value = 0; Min_Value < 255 && HistGram[Min_Value] == 0; Min_Value++) ;        //��ȡ��С�Ҷȵ�ֵ
    for (Max_Value = 255; Max_Value > Min_Value && HistGram[Min_Value] == 0; Max_Value--) ; //��ȡ���Ҷȵ�ֵ

    if (Max_Value == Min_Value)
    {
        return Max_Value;          // ͼ����ֻ��һ����ɫ
    }
    if (Min_Value + 1 == Max_Value)
    {
        return Min_Value;      // ͼ����ֻ�ж�����ɫ
    }

    for (Y = Min_Value; Y <= Max_Value; Y++)
    {
        Amount += HistGram[Y];        //  ��������
    }

    Pixel_Integral = 0;
    for (Y = Min_Value; Y <= Max_Value; Y++)
    {
        Pixel_Integral += HistGram[Y] * Y;//�Ҷ�ֵ����
    }
    SigmaB = -1;
    for (Y = Min_Value; Y < Max_Value; Y++)
    {
          Pixel_Back = Pixel_Back + HistGram[Y];    //ǰ�����ص���
          Pixel_Fore = Amount - Pixel_Back;         //�������ص���
          Omega_Back = (double)Pixel_Back / Amount;//ǰ�����ذٷֱ�
          Omega_Fore = (double)Pixel_Fore / Amount;//�������ذٷֱ�
          Pixel_Integral_Back += HistGram[Y] * Y;  //ǰ���Ҷ�ֵ
          Pixel_IntegralFore = Pixel_Integral - Pixel_Integral_Back;//�����Ҷ�ֵ
          Micro_Back = (double)Pixel_Integral_Back / Pixel_Back;//ǰ���ҶȰٷֱ�
          Micro_Fore = (double)Pixel_IntegralFore / Pixel_Fore;//�����ҶȰٷֱ�
          Sigma = Omega_Back * Omega_Fore * (Micro_Back - Micro_Fore) * (Micro_Back - Micro_Fore);//g
          if (Sigma > SigmaB)//����������䷽��g
          {
              SigmaB = Sigma;
              Threshold = (uint8)Y;
          }
    }
   return Threshold;
}


/**@brief    ���ͼ���ֵ��
-- @param    ��
-- @auther   ׯ�ı�
-- @date     2023/10/2
**/
void Turn_To_Bin(void)
{
  uint8 i,j;
  Image_Thereshold = Otsu_Threshold(Original_Image[0], Image_W, Image_H);//��ȡ�����ֵ
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


/**----------------------------------------------------�����򲿷�------------------------------------------------------------------------**/
/**@brief    Ѱ�������߽�ı߽����Ϊ������ѭ������ʼ��
-- @param    uint8 Start_Row ������������
-- @auther   ׯ�ı�
-- @date     2023/10/3
**/
uint8 Get_Start_Point(uint8 Start_Row)
{
    uint8 i = 0,L_Found = 0,R_Found = 0;
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

    if(L_Found&&R_Found)//��ߺ��ұ߶��ҵ����
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
-- @param    uint16 Break_Flag �����Ҫѭ���Ĵ���
-- @param    uint8(*image)[Image_W] ��Ҫ�����ҵ��ͼ�����飬�����Ƕ�ֵͼ,�����������Ƽ���
-- @param    uint16 *L_Stastic ͳ��������ݣ����������ʼ�����Ա����ź�ȡ��ѭ������
-- @param    uint16 *R_Stastic ͳ���ұ����ݣ����������ʼ�����Ա����ź�ȡ��ѭ������
-- @param    uint8 L_Start_X �����������
-- @param    uint8 L_Start_Y ������������
-- @param    uint8 R_Start_X �ұ���������
-- @param    uint8 R_Start_Y �ұ����������
-- @param    uint8 Hightest ѭ���������õ�����߸߶�
-- @auther   ׯ�ı�
-- @date     2023/10/3
**/
void Search_L_R(uint16 Break_Flag, uint8(*image)[Image_W], uint16 *L_Stastic, uint16 *R_Stastic, uint8 L_Start_X, uint8 L_Start_Y, uint8 R_Start_X, uint8 R_Start_Y, uint8*Hightest)
{

    uint8 i = 0, j = 0;

    //��߱���
    uint8 Search_Filds_L[8][2] = { {  0 } };//Ѱ�Ұ���������
    uint8 Index_L = 0;
    uint8 Temp_L[8][2] = { {  0 } };
    uint8 Center_Point_L[2] = {  0 };
    uint16 L_Data_Statics;//ͳ������ҵ��ĵ�
    //����˸�����
    static int8 Seeds_L[8][2] = { {0,  1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,  0},{1, 1}, };
    //{-1,-1},{0,-1},{+1,-1},
    //{-1, 0},       {+1, 0},
    //{-1,+1},{0,+1},{+1,+1},
    //�����˳ʱ��

    //�ұ߱���
    uint8 Search_Filds_R[8][2] = { {  0 } };
    uint8 Center_Point_R[2] = { 0 };//���������
    uint8 Index_R = 0;//�����±�
    uint8 Temp_R[8][2] = { {  0 } };
    uint16 R_Data_Statics;//ͳ���ұ�
    //����˸�����
    static int8 Seeds_R[8][2] = { {0,  1},{1,1},{1,0}, {1,-1},{0,-1},{-1,-1}, {-1,  0},{-1, 1}, };
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
        if ((Points_R[R_Data_Statics][0]== Points_R[R_Data_Statics-1][0]&& Points_R[R_Data_Statics][0] == Points_R[R_Data_Statics - 2][0]
            && Points_R[R_Data_Statics][1] == Points_R[R_Data_Statics - 1][1] && Points_R[R_Data_Statics][1] == Points_R[R_Data_Statics - 2][1])
            ||(Points_L[L_Data_Statics-1][0] == Points_L[L_Data_Statics - 2][0] && Points_L[L_Data_Statics-1][0] == Points_L[L_Data_Statics - 3][0]
                && Points_L[L_Data_Statics-1][1] == Points_L[L_Data_Statics - 2][1] && Points_L[L_Data_Statics-1][1] == Points_L[L_Data_Statics - 3][1]))
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
            printf("\n�����߱��ұ߸��ˣ���ߵȴ��ұ�\n");
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
                //printf("dir[%d]:%d\n", R_Data_Statics - 1, Dir_R[R_Data_Statics - 1]);
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
-- @param   uint16 Total_L �ҵ��ĵ������
-- @auther  ׯ�ı�
-- @date    2023/10/3
**/
void Get_Left(uint16 Total_L)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
    //��ʼ��
    for (i = 0;i<Image_H;i++)
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
            L_Border[h] = Points_L[j][0]+1;
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
-- @param   uint16 Total_R �ҵ��ĵ������
-- @auther  ׯ�ı�
-- @date    2023/10/3
**/
void Get_Right(uint16 Total_R)
{
    uint8 i = 0;
    uint16 j = 0;
    uint8 h = 0;
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
-- @param   uint8(*Bin_Image)[Image_W] ��ֵ��ͼ��
-- @auther  ׯ�ı�
-- @date    2023/10/3
**/
void Image_Filter(uint8(*Bin_Image)[Image_W])//��̬ѧ�˲�������˵�������ͺ͸�ʴ��˼��
{
    uint16 i, j;
    uint32 num = 0;


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
-- @param   uint8(*Image)[Image_w]  ͼ���׵�ַ
-- @auther  ׯ�ı�
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
        //image[image_h-1][i] = 0;

    }
}


/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ���յ��õ�ͼ����ĺ���
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/10/3
**/
void Image_Process()
{
    uint16 i;
    uint8 Hightest = 0;//����һ������У�tip����������ָ����yֵ����С

    Get_Image(mt9v03x_image);//��ȡһ��ͼ��
    Turn_To_Bin();//��ֵ��

    Image_Filter(Bin_Image);//�˲�
    Image_Draw_Rectan(Bin_Image);//Ԥ����

    //����
    Data_Stastics_L = 0;
    Data_Stastics_R = 0;

    if(Get_Start_Point(Image_H - 2))//��ͼ��������濪ʼ�ң��ҵ�����ˣ���ִ�а�����û�ҵ���һֱ��
    {
//        printf("���ڿ�ʼ������\n");
        Search_L_R((uint16)USE_num, Bin_Image, &Data_Stastics_L, &Data_Stastics_R, Start_Point_L[0], Start_Point_L[1], Start_Point_R[0], Start_Point_R[1], &Hightest);
//        printf("�������ѽ���\n");
        // ����ȡ�ı߽�������ȡ���� �� ��������������õı���
        Get_Left(Data_Stastics_L);
        Get_Right(Data_Stastics_R);
        //�������������Ҫ�ŵ�if����ȥ��
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
        Center_Line[i] = (L_Border[i] + R_Border[i]) >> 1;//������
        //�������������󣬲����ǲ��߻�����״̬����ȫ�����ʹ��һ����ߣ����������������ܸ����������
        if(Show_Mode == 2)
        {
            tft180_draw_point(Center_Line[i], i, RGB565_BLACK);//��ʾ��� ��ʾ����
            tft180_draw_point(L_Border[i], i, RGB565_BLUE);//��ʾ��� ��ʾ�����
            tft180_draw_point(R_Border[i], i, RGB565_RED);//��ʾ��� ��ʾ�ұ���
        }
    }
    Image_Erro = (Center_Line[34])*0.375 + (Center_Line[35])*0.5 + (Center_Line[37])*0.1;
}

