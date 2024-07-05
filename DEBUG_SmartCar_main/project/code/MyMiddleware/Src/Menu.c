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
uint8 Menu_Mode = Page_Select;
Menu_ Menu = {0,0,0};
int Show_Mode;
#define Exit_Dis tft180_show_string(Row_1,Line_9,"Exit")
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
-- @author  ׯ�ı�
-- @date    2023/11/13
**/
static void Arrow_Display(int Line)
{
    for(int Line_Num = 0;Line_Num <= 9;Line_Num++)
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
            else if(Line_Num >=4)
            {
                tft180_show_string(Row_0,Line_Num*16," ");
            }
        }

    }
}

/**@brief   ���л�
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/2/16
**/
static void Line_Change()
{
    if((Rotary.Clockwise) && (Menu.Set_Line < 9))//˳ʱ��ת
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
-- @author  ׯ�ı�
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
    tft180_show_string(Row_1,Line_8,"Page8");
    tft180_show_string(Row_1,Line_9,"Page9");
    if(Rotary.Press)//��������
    {
        Rotary.Press = 0;
        Menu_Mode = Menu.Set_Line;
        Menu.Set_Line = 0;
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        if(Menu_Mode==9)
        {
            Menu.Set_Line = 9;
            Menu.Image_Show = true;
            flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        }
        tft180_clear();
    }   
}

/**@brief    ����ҳ��ʾ
-- @param    ��
-- @verbatim ���ڲ鿴����������ֵ
-- @author   ׯ�ı�
-- @date     2024/2/16
**/
static void Page0_Mode()
{
    Line_Change();//���л�
    Arrow_Display(Menu.Set_Line);//��ͷ��ʾ
    if(!Menu.Image_Show)
    {
        tft180_show_string(Row_1,Line_0,"Yaw:");
        tft180_show_float(Row_5,Line_0,Gyro_YawAngle_Get(),3,1);
        tft180_show_string(Row_1,Line_1,"Car_Go:");
        tft180_show_int(Row_8,Line_1,Start,2);
    }

    Exit_Dis;

    if((Menu.Set_Line == 9) && (Rotary.Press))//�˳�
    {
        Rotary.Press = 0;
        Menu_Mode = Page_Select;
        Menu.Set_Line = 0;
        Menu.Image_Show = false;
        tft180_clear();
    } 

    if((Menu.Set_Line == 1) && (Rotary.Press))//����
    {
        Rotary.Press = 0;
        Start = 1;
    }
}


/**@brief   ��һҳ��ʾ
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/2/18
**/
static void Page1_Mode()
{
    // flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    tft180_show_string(Row_1,Line_0,"XKP:");
    tft180_show_float(Row_5,Line_0,flash_union_buffer[0].float_type,2,2);
    tft180_show_string(Row_1,Line_1,"YKP:");
    tft180_show_float(Row_5,Line_1,flash_union_buffer[1].float_type,2,2);
    tft180_show_string(Row_1,Line_2,"XKD:");
    tft180_show_float(Row_5,Line_2,flash_union_buffer[2].float_type,2,2);
    tft180_show_string(Row_1,Line_3,"YKD:");
    tft180_show_float(Row_5,Line_3,flash_union_buffer[3].float_type,2,2);
    tft180_show_string(Row_1,Line_4,"AngleP:");
    tft180_show_float(Row_8,Line_4,flash_union_buffer[4].float_type,2,2);
    tft180_show_string(Row_1,Line_5,"AngleD:");
    tft180_show_float(Row_8,Line_5,flash_union_buffer[5].float_type,2,2);
    Exit_Dis;
    if(Menu.Set_Mode == Normal_Mode)
    {
        if((Menu.Set_Line == 9) && (Rotary.Press))//�˳�
        {
            Rotary.Press = 0;
            Menu_Mode = Page_Select;//�˳�����һҳ
            Menu.Set_Line = 0;
            tft180_clear();
        }
        else if((Menu.Set_Line != 9) && (Rotary.Press))
        {
            Rotary.Press = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        Line_Change();//���л�
    }
    else if(Menu.Set_Mode == Flash_Mode)//���ò���
    {
        if(Rotary.Clockwise)//˳ʱ��ת
        {
            Rotary.Clockwise = 0;
            flash_union_buffer[Menu.Set_Line].float_type+=0.01;
        }
        else if(Rotary.Anticlockwise)//��ʱ��ת
        {
            Rotary.Anticlockwise = 0;
            flash_union_buffer[Menu.Set_Line].float_type-=0.01;
        }
        else if(Rotary.Press)//���ν���
        {
            Rotary.Press = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        }
    }

    if(Menu.Flash_Set)//���ν���
    {
        Menu.Flash_Set = 0;
        DistanceX_PID.Kp = flash_union_buffer[0].float_type;
		DistanceY_PID.Kp = flash_union_buffer[1].float_type;
        DistanceX_PID.Kd = flash_union_buffer[2].float_type;
		DistanceY_PID.Kd = flash_union_buffer[3].float_type;
        AngleControl_PID.Kp =  flash_union_buffer[4].float_type;
        AngleControl_PID.Kd =  flash_union_buffer[5].float_type;
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    }
    Arrow_Display(Menu.Set_Line);//��ͷ��ʾ
}

/**@brief   ͼ����ʾ�Լ���ز���
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/7/1
**/
static void Image_Page()
{
    // if(mt9v03x_finish_flag)
    // {
    //     mt9v03x_finish_flag = 0;
    //     Get_Bin_Image();
    //     tft180_show_gray_image(0,0,(uint8*)Bin_Image,MT9V03X_W,MT9V03X_H,128,60,0);
    // }
    // tft180_set_dir(TFT180_CROSSWISE);
    tft180_show_gray_image(0,0,(uint8*)Bin_Image,MT9V03X_W,MT9V03X_H,148,80,0);
    // tft180_show_gray_image(0,0,(uint8*)mt9v03x_image,MT9V03X_W,MT9V03X_H,148,80,0);
    for (int i = Hightest; i < Image_H-1; i++)
    {
        tft180_draw_point(Center_Line[i], i, RGB565_BLACK);
        tft180_draw_point(L_Border[i], i, RGB565_BLUE);
        tft180_draw_point(R_Border[i], i, RGB565_RED);
    }
    // tft180_show_string(Row_1,Line_4,"I_Erro:");
    // tft180_show_float(Row_8,Line_4,Image_Erro,3,2);
    // tft180_show_string(Row_1,Line_5,"Ex_Time:");
    // tft180_show_uint(Row_9,Line_5,flash_union_buffer[50].uint16_type,5);//�ع�ʱ��
    // Exit_Dis;
    if((Menu.Set_Line == 9) && (Rotary.Press))//�˳�
    {
        Rotary.Press = 0;
        Menu_Mode = Page_Select;
        Menu.Image_Show = false;
        Menu.Set_Line = 0;
        tft180_clear();
    }

    if((Menu.Set_Line == 4) && (Rotary.Press))//�˳�
    {
        Rotary.Press = 0;
        Menu.Image_Show = true;
        tft180_clear();
    }

    if(Menu.Set_Mode == Normal_Mode)
    {
        if((Menu.Set_Line == 9) && (Rotary.Press))//�˳�
        {
            Rotary.Press = 0;
            Menu_Mode = Page_Select;//�˳�����һҳ
            Menu.Set_Line = 0;
            tft180_clear();
        }
        else if((Menu.Set_Line == 5) && (Rotary.Press))
        {
            Rotary.Press = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        Line_Change();//���л�
    }
    else if(Menu.Set_Mode == Flash_Mode)//���ò���
    {
        if(Rotary.Clockwise)//˳ʱ��ת
        {
            Rotary.Clockwise = 0;
            flash_union_buffer[50].uint16_type+=10;
        }
        else if(Rotary.Anticlockwise)//��ʱ��ת
        {
            Rotary.Anticlockwise = 0;
            flash_union_buffer[50].uint16_type-=10;
        }
        else if(Rotary.Press)//���ν���
        {
            Rotary.Press = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        }
    }

    if(Menu.Flash_Set)//���ν���
    {
        Menu.Flash_Set = 0;
        Menu.Ex_Time = flash_union_buffer[50].uint16_type;
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    }
    // Arrow_Display(Menu.Set_Line);//��ͷ��ʾ
}

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   Flash��ʼ��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/2/18
**/
void Flash_Init()
{
    flash_init();//���flash��ʼ��
    if(!flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))//�ж��Ƿ������ݣ����û������
    {
        flash_union_buffer[0].float_type = 0.15f;//΢��X��P
        flash_union_buffer[1].float_type = 0.28f;//΢��Y��P
        flash_union_buffer[2].float_type = 0.20f;//΢��X��D
        flash_union_buffer[3].float_type = 0.0f;//΢��Y��D
        flash_union_buffer[4].float_type = 0.13f;//�ǶȻ�P
        flash_union_buffer[5].float_type = 0.5f;//�ǶȻ�D
        flash_union_buffer[50].uint16_type = 150;//�ع�ʱ��
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        flash_buffer_clear();
    }
    else
    {
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        DistanceX_PID.Kp = flash_union_buffer[0].float_type;
        DistanceY_PID.Kp = flash_union_buffer[1].float_type;
        DistanceX_PID.Kd = flash_union_buffer[2].float_type;
        DistanceY_PID.Kd = flash_union_buffer[3].float_type;
        AngleControl_PID.Kp = flash_union_buffer[4].float_type;
        AngleControl_PID.Kd = flash_union_buffer[5].float_type;
        Menu.Ex_Time = flash_union_buffer[50].uint16_type;
        flash_buffer_clear();
    }
}

/**@brief   ÿҳ��ʾ������
-- @param   ��
-- @author  ׯ�ı�
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
        case Page9:
            Image_Page();
        break;
    }
}

