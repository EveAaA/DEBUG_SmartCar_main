/**
  ******************************************************************************
  * @file    Menu.c
  * @author  ׯ�ı�
  * @brief   �˵�
  * @date    11/13/2023
    @verbatim
    �˵���ʾ
    @endverbatim
  * @{
**/


/* Includes ------------------------------------------------------------------*/
#include "Menu.h"

/* Define\Declare ------------------------------------------------------------*/
Menu_Handle Menu_Mode = 8;
Menu_ Menu = {0,0,0};
int Show_Mode;
#define Exit_Dis tft180_show_string(Row_1,Line_7,"Exit")
#define FLASH_SECTION_INDEX (127)// �洢�����õ����� ������һ������
#define FLASH_PAGE_INDEX (FLASH_PAGE_3)// �洢�����õ�ҳ�� ������һ��ҳ��

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief   ��ʾ��ͷ
-- @param   int Row ��Ҫ��ʾ���к�
-- @auther  ׯ�ı�
-- @date    2023/11/13
**/
static void Arrow_Display(int Line)
{
    for(int Line_Num = 0;Line_Num <= 7;Line_Num++)
    {
        if(Line == Line_Num)
        {
            tft180_show_string(Row_0,Line_Num*16,">");
        }
        else
        {
            if(!Menu.Image_Show)
            {
                tft180_show_string(Row_0,Line_Num*16," ");
            }
        }

    }
}

/**@brief   ���л�
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/2/16
**/
static void Line_Change()
{
    if((Rotary.Clockwise) && (Menu.Set_Line < 7))//˳ʱ��ת
    {
        Rotary.Clockwise = 0;
        Menu.Set_Line++;
    }
    else if((Rotary.Anticlockwise) && (Menu.Set_Line > 0))//��ʱ��ת
    {
        Rotary.Anticlockwise = 0;
        Menu.Set_Line--;
    }
}


/**@brief   ҳ��ѡ��ģʽ
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/2/16
**/
static void Page_Select_Mode()
{
    Line_Change();//���л�
    Arrow_Display(Menu.Set_Line);//��ͷ��ʾ
    tft180_show_string(Row_1,Line_0,"Page0");
    tft180_show_string(Row_1,Line_1,"Page1"); 
    tft180_show_string(Row_1,Line_2,"Page2"); 
    tft180_show_string(Row_1,Line_3,"Page3"); 
    tft180_show_string(Row_1,Line_4,"Page4"); 
    tft180_show_string(Row_1,Line_5,"Page5");
    tft180_show_string(Row_1,Line_6,"Page6");
    tft180_show_string(Row_1,Line_7,"Page7");

    if(Rotary.Press)//��������
    {
        Rotary.Press = 0;
        Menu_Mode = Menu.Set_Line;
        Menu.Set_Line = 0;
        tft180_clear();
    }   
}

/**@brief    ����ҳ��ʾ
-- @param    ��
-- @verbatim ���ڲ鿴����������ֵ
-- @auther   ׯ�ı�
-- @date     2024/2/16
**/
static void Page0_Mode()
{
    Line_Change();//���л�
    Arrow_Display(Menu.Set_Line);//��ͷ��ʾ
    if(!Menu.Image_Show)
    {
        tft180_show_string(Row_1,Line_0,"Yaw_Angle:");
        tft180_show_float(Row_11,Line_0,Gyro_YawAngle_Get(),3,1);
        tft180_show_string(Row_1,Line_1,"Start:");
        tft180_show_int(Row_7,Line_1,Start,2);
        tft180_show_string(Row_1,Line_2,"Stop:");
        tft180_show_string(Row_1,Line_3,"dx:");
        tft180_show_float(Row_4,Line_3,border.dx, 3, 3);
        tft180_show_string(Row_1,Line_4,"IsFind:");
        tft180_show_float(Row_8, Line_4, border.isFindBorder, 3, 3);
        tft180_show_string(Row_1,Line_5,"Image");
    }

    Exit_Dis;

    if(Menu.Set_Line == 7 && Rotary.Press)//�˳�
    {
        Rotary.Press = 0;
        Menu_Mode = 8;
        Menu.Set_Line = 0;
        Menu.Image_Show = false;
        tft180_clear();
    } 

    if(Menu.Set_Line == 1 && Rotary.Press)//����
    {
        Rotary.Press = 0;
        Start = 1;
        Menu.Image_Show = true;
    }
    if(Menu.Set_Line == 2 && Rotary.Press)//����
    {
        Rotary.Press = 0;
        Start = 3;
        // Menu.Image_Show = true;
    }

    if(Menu.Set_Line == 5 && Rotary.Press)//��ʾͼ��
    {
        Rotary.Press = 0;
        Menu.Set_Line = 6;
        Menu.Image_Show = true;
        tft180_clear();
    }

    if(Menu.Image_Show)
    {

        for (int i = Hightest; i < Image_H-1; i++)
        {
            // Center_Line[i] = (L_Border[i] + R_Border[i]) >> 1;//������
            //�������������󣬲����ǲ��߻�����״̬����ȫ�����ʹ��һ����ߣ����������������ܸ����������
            tft180_draw_point(Center_Line[i], i, RGB565_BLACK);//��ʾ��� ��ʾ����
            tft180_draw_point(L_Border[i], i, RGB565_BLUE);//��ʾ��� ��ʾ�����
            tft180_draw_point(R_Border[i], i, RGB565_RED);//��ʾ��� ��ʾ�ұ���
        }
        tft180_show_gray_image(0, 0, (const uint8 *)(Bin_Image), MT9V03X_W, MT9V03X_H, (Row_18), (Line_5), 0);
        tft180_show_float(Row_8, Line_6, Image_Erro, 3, 3);
    }
}

/**@brief   ��һҳ��ʾ
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/2/18
**/
static void Page1_Mode()
{
    tft180_show_string(Row_1,Line_0,"Test:");
    tft180_show_float(Row_7,Line_0,flash_union_buffer[0].float_type,2,2);

    Exit_Dis;
    if(Menu.Set_Mode == 0)
    {
        if(Menu.Set_Line == 7 && Rotary.Press)//�˳�
        {
            Rotary.Press = 0;
            Menu_Mode = 8;
            Menu.Set_Line = 0;
            tft180_clear();
        }
        else if(Menu.Set_Line != 7 && Rotary.Press)
        {
            Rotary.Press = 0;
            Menu.Set_Mode = 1;
        } 
        Line_Change();//���л�
    }
    else if(Menu.Set_Mode == 1)//���ò���
    {
        if(Rotary.Clockwise)//˳ʱ��ת
        {
            Rotary.Clockwise = 0;
            flash_union_buffer[Menu.Set_Line].float_type++;
        }
        else if(Rotary.Anticlockwise)//��ʱ��ת
        {
            Rotary.Anticlockwise = 0;
            flash_union_buffer[Menu.Set_Line].float_type--;
        }
        else if(Rotary.Press)//���ν���
        {
            Rotary.Press = 0;
            Menu.Set_Mode = 0;
            Menu.Flash_Set = 1;
        }
    }

    if(Menu.Flash_Set)//���ν���
    {
        Menu.Flash_Set = 0;
        Image_PID.Kp = flash_union_buffer[0].float_type;
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    }

    Arrow_Display(Menu.Set_Line);//��ͷ��ʾ
}

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   Flash��ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2024/2/18
**/
void Flash_Init()
{
    flash_init();//���flash��ʼ��
    if(!flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))//���û������                      // �ж��Ƿ�������
    {
        flash_union_buffer[0].float_type = Image_PID.Kp;
        flash_union_buffer[1].float_type = Image_PID.Kd;
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        flash_buffer_clear();
    }
    else
    {
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        Image_PID.Kp = flash_union_buffer[0].float_type;
        Image_PID.Kd = flash_union_buffer[1].float_type;
        flash_buffer_clear();
    }
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
}

/**@brief   ÿҳ��ʾ������
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/13
**/
void Menu_Display()
{
    switch(Menu_Mode)
    {
        case Page_Select:
            Page_Select_Mode();
        break;
        case Page0:
            Page0_Mode();
        break;
        case Page1:
            Page1_Mode();
        break;
    }
}

