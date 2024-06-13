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
-- @author  ׯ�ı�
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
-- @author  ׯ�ı�
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
-- @author   ׯ�ı�
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
        tft180_show_string(Row_1,Line_2,"dx:");
        tft180_show_float(Row_4,Line_2,FINETUNING_DATA.dx/10.f,3,1);
        tft180_show_string(Row_1,Line_3,"dy:");
        tft180_show_float(Row_4,Line_3,FINETUNING_DATA.dy/10.f,3,1);
        tft180_show_string(Row_1,Line_4,"sp:");
        // tft180_show_float(Row_4,Line_4,Encoer_Speed[0],2,1);
        // tft180_show_float(Row_8,Line_4,Encoer_Speed[1],2,1);
        tft180_show_float(Row_4,Line_4,Encoer_Speed[0],2,1);
        tft180_show_float(Row_8,Line_4,Encoer_Speed[1],2,1);
        tft180_show_string(Row_1,Line_5,"Image");
        // tft180_show_string(Row_1,Line_6,"my:");
        // tft180_show_float(Row_4,Line_6,Get_Y_Distance(),3,1);
    }

    Exit_Dis;

    if((Menu.Set_Line == 7) && (Rotary.Press))//�˳�
    {
        Rotary.Press = 0;
        Menu_Mode = 8;
        Menu.Set_Line = 0;
        Menu.Image_Show = false;
        tft180_clear();
    } 

    if((Menu.Set_Line == 1) && (Rotary.Press))//����
    {
        Rotary.Press = 0;
        Start = 1;
        Menu.Image_Show = true;
    }
    
    // if((Menu.Set_Line == 2) && (Rotary.Press))//����
    // {
    //     Rotary.Press = 0;
    //     Start = 0;
    //     // Menu.Image_Show = true;
    // }

    if((Menu.Set_Line == 5) && (Rotary.Press))//��ʾͼ��
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
        tft180_show_float(Row_8, Line_6,Image_Erro, 3, 3);
        tft180_show_float(Row_8, Line_7,Image_Erro_Y, 3, 3);
    }
}

/**@brief   ��һҳ��ʾ
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/2/18
**/
static void Page1_Mode()
{
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
        if((Menu.Set_Line == 7) && (Rotary.Press))//�˳�
        {
            Rotary.Press = 0;
            Menu_Mode = 8;//�˳�����һҳ
            Menu.Set_Line = 0;
            tft180_clear();
        }
        else if((Menu.Set_Line != 7) && (Rotary.Press))
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
        flash_union_buffer[1].float_type = 0.03f;//΢��Y��P
        flash_union_buffer[2].float_type = 0.20f;//΢��X��D
        flash_union_buffer[3].float_type = 0.08f;//΢��Y��D
        flash_union_buffer[4].float_type = 0.13f;//�ǶȻ�P
        flash_union_buffer[5].float_type = 0.5f;//�ǶȻ�D
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
        flash_buffer_clear();
    }
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
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
    }
}

