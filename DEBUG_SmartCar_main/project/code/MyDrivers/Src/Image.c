/**
  ******************************************************************************
  * @file    Image.c
  * @author  none
  * @brief   ͼ�񲿷�
  *
    @verbatim
    
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Image.h"
#include "UserMain.h"

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

float Image_Erro;

Flag_Handle Image_Flag;//Ԫ�ر�־λ

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
    if(value>=0) return value;
    else return -value;
}


/**@brief    �޷�����
-- @param    int16 x ��Ҫ�޷���ֵ
-- @param    int a ���ֵ
-- @param    int b ��Сֵ
-- @return   x ���
-- @auther   none
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
* @brief ��С���˷�
* @param uint8 begin				�������
* @param uint8 end					�����յ�
* @param uint8 *border				������Ҫ����б�ʵı߽��׵�ַ
*  @see CTest		Slope_Calculate(start, end, border);//б��
* @return ����˵��
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
	if ((end - begin)*x2sum - xsum * xsum) //�жϳ����Ƿ�Ϊ��
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
* @brief ����б�ʽؾ�
* @param uint8 start				�������
* @param uint8 end					�����յ�
* @param uint8 *border				������Ҫ����б�ʵı߽�
* @param float *slope_rate			����б�ʵ�ַ
* @param float *intercept			����ؾ��ַ
*  @see CTest		calculate_s_i(start, end, R_Border, &slope_l_rate, &intercept_l);
* @return ����˵��
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

	//�������ƽ����
	if (num)
	{
		x_average = (float)(xsum / num);
		y_average = (float)(ysum / num);

	}

	/*����б��*/
	*slope_rate = Slope_Calculate(start, end, border);//б��
	*intercept = y_average - (*slope_rate)*x_average;//�ؾ�
}

/**----------------------------------------------------��򷨲���------------------------------------------------------------------------**/
/**@brief    ��ȡһ���Ҷ�ͼ��
-- @param    uint8(*mt9v03x_image)[Image_W] ����ͷ�ɼ�ͼ���Ӧ��ָ��
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

/**@brief    ��򷨻�ȡ��̬��ֵ
-- @param    uint8 *image ��Ҫ�����ͼ��
-- @param    uint16 col �г���
-- @param    uint16 row �г���
-- @auther   none
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
    float Omega_Back=0, Omega_Fore=0, Micro_Back=0, Micro_Fore=0, SigmaB=0, Sigma=0; // ��䷽��ֱ�Ϊ�������ص����Ȩ�أ�ǰ�����ص����Ȩ�أ��������ص�ƽ���Ҷ�ֵ��ǰ�����ص�ƽ���Ҷ�ֵ;
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
          Omega_Back = (float)Pixel_Back / Amount;//ǰ�����ذٷֱ�
          Omega_Fore = (float)Pixel_Fore / Amount;//�������ذٷֱ�
          Pixel_Integral_Back += HistGram[Y] * Y;  //ǰ���Ҷ�ֵ
          Pixel_IntegralFore = Pixel_Integral - Pixel_Integral_Back;//�����Ҷ�ֵ
          Micro_Back = (float)Pixel_Integral_Back / Pixel_Back;//ǰ���ҶȰٷֱ�
          Micro_Fore = (float)Pixel_IntegralFore / Pixel_Fore;//�����ҶȰٷֱ�
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
-- @auther   none
-- @date     2023/10/2
**/
void Turn_To_Bin(void)
{
  uint8 i,j;
  Image_Thereshold = 1.075 * Otsu_Threshold(Original_Image[0], Image_W, Image_H);//��ȡ�����ֵ
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
-- @auther   none
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
-- @auther   none
-- @date     2023/10/3
**/
void Search_L_R(uint16 Break_Flag, uint8(*image)[Image_W], uint16 *L_Stastic, uint16 *R_Stastic, uint8 L_Start_X, uint8 L_Start_Y, uint8 R_Start_X, uint8 R_Start_Y, uint8*Hightest)
{

    uint8 i = 0, j = 0;

    //��߱���
    uint8 Search_Filds_L[8][2] = {{0}};//Ѱ�Ұ���������
    uint8 Index_L = 0;
    uint8 Temp_L[8][2] = {{0}};
    uint8 Center_Point_L[2] = {0};
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
-- @auther  none
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
-- @auther  none
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
-- @param   uint8(*Image)[Image_W]  ͼ���׵�ַ
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

/**----------------------------------------------------Ԫ�ز���------------------------------------------------------------------------**/
/** 
* @brief ʮ�ֲ��ߺ���
* @param uint8(*Bin_Image)[Image_W]		�����ֵͼ��
* @param uint8 *L_Border			������߽��׵�ַ
* @param uint8 *R_Border			�����ұ߽��׵�ַ
* @param uint16 Total_Num_L			�������ѭ���ܴ���
* @param uint16 Total_Num_R			�����ұ�ѭ���ܴ���
* @param uint16 *Dir_L				����������������׵�ַ
* @param uint16 *Dir_R				�����ұ����������׵�ַ
* @param uint16(*Points_L)[2]		������������׵�ַ
* @param uint16(*Points_R)[2]		�����ұ������׵�ַ
*  @see CTest		Cross_Fill(Bin_Image,L_Border, R_Border, data_statics_l, data_statics_r, Dir_L, Dir_R, Points_L, Points_R);
* @return ����˵��
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
	//��ʮ��
	for (i = 1; i < Total_Num_L; i++)
	{
		if (Dir_L[i - 1] == 4 && Dir_L[i] == 4 && Dir_L[i + 3] == 6 && Dir_L[i + 5] == 6 && Dir_L[i + 7] == 6)
		{
			Break_Num_L = Points_L[i][1];//����y����
			// printf("brea_knum-L:%d\n", Break_Num_L);
			// printf("I:%d\n", i);
			// printf("ʮ�ֱ�־λ��1\n");
			break;
		}
	}
	for (i = 1; i < Total_Num_R; i++)
	{
		if (Dir_R[i - 1] == 4 && Dir_R[i] == 4 && Dir_R[i + 3] == 6 && Dir_R[i + 5] == 6 && Dir_R[i + 7] == 6)
		{
			Break_Num_R = Points_R[i][1];//����y����
			// printf("brea_knum-R:%d\n", Break_Num_R);
			// printf("I:%d\n", i);
			// printf("ʮ�ֱ�־λ��1\n");
			break;
		}
	}
	if (Break_Num_L && Break_Num_R && Bin_Image[Image_H - 1][4] && Bin_Image[Image_H - 1][Image_W - 4])//�����������򶼷�������
	{
		//����б��
		start = Break_Num_L - 15;
		start = Limit_a_b(start, 0, Image_H);
		end = Break_Num_L - 5;
		calculate_s_i(start, end, L_Border, &slope_l_rate, &intercept_l);
		//printf("slope_l_rate:%d\nintercept_l:%d\n", slope_l_rate, intercept_l);
		for (i = Break_Num_L - 5; i < Image_H - 1; i++)
		{
			L_Border[i] = slope_l_rate * (i)+intercept_l;//y = kx+b
			L_Border[i] = Limit_a_b(L_Border[i], Border_Min, Border_Max);//�޷�
		}

		//����б��
		start = Break_Num_R - 15;//���
		start = Limit_a_b(start, 0, Image_H);//�޷�
		end = Break_Num_R - 5;//�յ�
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
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ���յ��õ�ͼ����ĺ���
-- @param   ��
-- @auther  none
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
        //Ԫ�ش�����������
        Cross_Fill(Bin_Image,L_Border, R_Border, Data_Stastics_L, Data_Stastics_R, Dir_L, Dir_R, Points_L, Points_R);//ʮ�ֲ���
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
        tft180_draw_point(Center_Line[i], i, RGB565_BLACK);//��ʾ��� ��ʾ����
        tft180_draw_point(L_Border[i], i, RGB565_BLUE);//��ʾ��� ��ʾ�����
        tft180_draw_point(R_Border[i], i, RGB565_RED);//��ʾ��� ��ʾ�ұ���
    }
    Image_Erro = (Center_Line[59])*0.375 + (Center_Line[60])*0.5 + (Center_Line[61])*0.1;
}

