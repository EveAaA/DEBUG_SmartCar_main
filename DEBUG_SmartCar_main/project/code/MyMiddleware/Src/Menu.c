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
#include "My_FSM.h"
/* Define\Declare ------------------------------------------------------------*/
uint8 Menu_Mode = Page9;
Menu_ Menu = 
{
    .Set_Line = 14,
    .Image_Show = true,
};
int Show_Mode;
#define Exit_Dis ips200_show_string(Row_1,Line_14,"Exit")
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
    for(int Line_Num = 0;Line_Num <= 14;Line_Num++)
    {
        if(Line == Line_Num)
        {
            ips200_show_string(Row_0,Line_Num*16,">");
        }
        else
        {
            if(!Menu.Image_Show)
            {
                ips200_show_string(Row_0,Line_Num*16," ");
            }
            else if(Line_Num >=6 && Menu.Image_Show)
            {
                ips200_show_string(Row_0,Line_Num*16," ");
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
    if((Button_Value[2] == 2) && (Menu.Set_Line < 14))//˳ʱ��ת
    {
        Button_Value[2] = 0;
        Menu.Set_Line++;
    }
    else if((Button_Value[0] == 2) && (Menu.Set_Line > 0))//��ʱ��ת
    {
        Button_Value[0] = 0;
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
    ips200_show_string(Row_1,Line_0,"Car_Go:");
    ips200_show_uint(Row_9,Line_0,Start,1);
    ips200_show_string(Row_1,Line_1,"Page1"); 
    ips200_show_string(Row_1,Line_2,"Page2"); 
    ips200_show_string(Row_1,Line_3,"Page3"); 
    ips200_show_string(Row_1,Line_4,"Page4"); 
    ips200_show_string(Row_1,Line_5,"Page5");
    ips200_show_string(Row_1,Line_6,"Page6");
    ips200_show_string(Row_1,Line_7,"Page7");
    ips200_show_string(Row_1,Line_8,"Page8");
    ips200_show_string(Row_1,Line_9,"Page9");
    if(Button_Value[1] == 2)//��������
    {
        Button_Value[1] = 0;
        Menu_Mode = Menu.Set_Line;
        Menu.Set_Line = 0;
        if(Menu_Mode==0)
        {
            Start = 1;
            ips200_clear();
            Set_Beepfreq(1);
            flash_buffer_clear();
        }
        if(Menu_Mode==9)
        {
            Menu.Set_Line = 9;
            Menu.Image_Show = true;
            // flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        }
        ips200_clear();
    }   
}

/**@brief    ����ҳ��ʾ
-- @param    ��
-- @verbatim ���ڲ鿴����������ֵ
-- @author   ׯ�ı�
-- @date     2024/2/16
**/
static void Page1_Mode()
{
    Line_Change();//���л�
    Arrow_Display(Menu.Set_Line);//��ͷ��ʾ
    ips200_show_string(Row_1,Line_0,"Down_Angle1:");
    ips200_show_uint(Row_14,Line_0,flash_union_buffer[0].uint16_type,5);
    ips200_show_string(Row_1,Line_1,"Down_Angle2:");
    ips200_show_uint(Row_14,Line_1,flash_union_buffer[1].uint16_type,5);
    ips200_show_string(Row_1,Line_2,"Down_Angle3:");
    ips200_show_uint(Row_14,Line_2,flash_union_buffer[2].uint16_type,5);
    ips200_show_string(Row_1,Line_3,"Down_Angle4:");
    ips200_show_uint(Row_14,Line_3,flash_union_buffer[3].uint16_type,5);
    ips200_show_string(Row_1,Line_4,"Down_Angle5:");
    ips200_show_uint(Row_14,Line_4,flash_union_buffer[4].uint16_type,5);
    ips200_show_string(Row_1,Line_5,"Down_Angle6:");
    ips200_show_uint(Row_14,Line_5,flash_union_buffer[5].uint16_type,5);

    Exit_Dis;
    if(Menu.Set_Mode == Normal_Mode)
    {
        if((Menu.Set_Line == 14) && (Button_Value[1] == 2))//�˳�
        {
            Button_Value[1] = 0;
            Menu_Mode = Page_Select;//�˳�����һҳ
            Menu.Set_Line = 0;
            ips200_clear();
        }
        else if((Menu.Set_Line != 14) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        Line_Change();//���л�
    }
    else if(Menu.Set_Mode == Flash_Mode)//���ò���
    {
        if(Button_Value[0] == 2)//˳ʱ��ת
        {
            Button_Value[0] = 0;
            flash_union_buffer[Menu.Set_Line].uint16_type+=1;
        }
        else if(Button_Value[2] == 2)//��ʱ��ת
        {
            Button_Value[2] = 0;
            flash_union_buffer[Menu.Set_Line].uint16_type-=1;
        }
        else if(Button_Value[1] == 2)//���ν���
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        }
    }

    if(Menu.Flash_Set)//���ν���
    {
        Menu.Flash_Set = 0;
        for(uint8 i = 0;i<=5;i++)
        {
            Down_Angle[i] = flash_union_buffer[i].uint16_type;
        }
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    }
}


/**@brief   ��һҳ��ʾ
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/2/18
**/
static void Page2_Mode()
{
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    ips200_show_string(Row_1,Line_0,"XKP:");
    ips200_show_float(Row_5,Line_0,flash_union_buffer[0].float_type,2,2);
    ips200_show_string(Row_1,Line_1,"YKP:");
    ips200_show_float(Row_5,Line_1,flash_union_buffer[1].float_type,2,2);
    ips200_show_string(Row_1,Line_2,"XKD:");
    ips200_show_float(Row_5,Line_2,flash_union_buffer[2].float_type,2,2);
    ips200_show_string(Row_1,Line_3,"YKD:");
    ips200_show_float(Row_5,Line_3,flash_union_buffer[3].float_type,2,2);
    ips200_show_string(Row_1,Line_4,"AngleP:");
    ips200_show_float(Row_8,Line_4,flash_union_buffer[4].float_type,2,2);
    ips200_show_string(Row_1,Line_5,"AngleD:");
    ips200_show_float(Row_8,Line_5,flash_union_buffer[5].float_type,2,2);
    Exit_Dis;
    if(Menu.Set_Mode == Normal_Mode)
    {
        if((Menu.Set_Line == 7) && (Button_Value[1] == 2))//�˳�
        {
            Button_Value[1] = 0;
            Menu_Mode = Page_Select;//�˳�����һҳ
            Menu.Set_Line = 0;
            ips200_clear();
        }
        else if((Menu.Set_Line != 7) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        Line_Change();//���л�
    }
    else if(Menu.Set_Mode == Flash_Mode)//���ò���
    {
        if(Button_Value[0] == 2)//˳ʱ��ת
        {
            Button_Value[0] = 0;
            flash_union_buffer[Menu.Set_Line].float_type+=0.01;
        }
        else if(Button_Value[2] == 2)//��ʱ��ת
        {
            Button_Value[2] = 0;
            flash_union_buffer[Menu.Set_Line].float_type-=0.01;
        }
        else if(Button_Value[1] == 2)//���ν���
        {
            Button_Value[1] = 0;
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

uint8 Bin_Image_Flag = 0;
/**@brief   ͼ����ʾ�Լ���ز���
-- @param   ��
-- @author  ׯ�ı�
-- @date    2024/7/1
**/
static void Image_Page()
{
    if(Car.Image_Flag == false)
    {
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
            Get_Bin_Image();
            if((Bin_Image_Flag))
            {
                ips200_show_gray_image(0,0,(uint8*)Bin_Image,MT9V03X_W,MT9V03X_H,MT9V03X_W,MT9V03X_H,0);
            }
            else
            {
                ips200_show_gray_image(0,0,(uint8*)mt9v03x_image,MT9V03X_W,MT9V03X_H,MT9V03X_W,MT9V03X_H,0);
            }
        }
    }
    // ips200_set_dir(ips200_CROSSWISE);
    // ips200_show_gray_image(0,0,(uint8*)Bin_Image,MT9V03X_W,MT9V03X_H,148,80,0);
    // ips200_show_gray_image(0,0,(uint8*)mt9v03x_image,MT9V03X_W,MT9V03X_H,148,80,0);
    // for (int i = Hightest; i < Image_H-1; i++)
    // {
    //     ips200_draw_point(Center_Line[i], i, RGB565_BLACK);
    //     ips200_draw_point(L_Border[i], i, RGB565_BLUE);
    //     ips200_draw_point(R_Border[i], i, RGB565_RED);
    // }
    ips200_show_string(Row_1,Line_6,"Bin:");
    ips200_show_uint(Row_6,Line_6,Bin_Image_Flag,2);

    ips200_show_string(Row_1,Line_7,"Ex_Time:");
    ips200_show_uint(Row_9,Line_7,flash_union_buffer[50].uint16_type,5);//�ع�ʱ��

    ips200_show_string(Row_1,Line_8,"T_P:");
    ips200_show_uint(Row_6,Line_8,flash_union_buffer[51].uint16_type,5);//�жϻص������ĵ�

    ips200_show_string(Row_1,Line_9,"Simple:");
    ips200_show_uint(Row_9,Line_9,flash_union_buffer[52].uint16_type,5);//�Ƿ�ֻ���·�Ŀ�Ƭ

    ips200_show_string(Row_1,Line_10,"Lose_Line_L:");
    ips200_show_uint(Row_14,Line_10,flash_union_buffer[53].uint16_type,5);//���Բ���������߼��

    ips200_show_string(Row_1,Line_11,"Lose_Line_R:");
    ips200_show_uint(Row_14,Line_11,flash_union_buffer[54].uint16_type,5);//�ұ�Բ���������߼��

    ips200_show_string(Row_1,Line_12,"Image_Flag:");
    ips200_show_uint(Row_12,Line_12,Car.Image_Flag,5);//ͼ�������


    Exit_Dis;

    if(Car.Image_Flag)
    {
        for (int i = Hightest; i < Image_H-1; i++)
        {
            ips200_draw_point(Center_Line[i], i, RGB565_BLACK);
            ips200_draw_point(L_Border[i], i, RGB565_BLUE);
            ips200_draw_point(R_Border[i], i, RGB565_RED);
        }

        ips200_show_gray_image(0,0,(uint8*)Bin_Image,MT9V03X_W,MT9V03X_H,MT9V03X_W,MT9V03X_H,0);
        ips200_show_string(Row_1,Line_13,"Image_Erro:");
        ips200_show_float(Row_12,Line_13,Image_Erro,3,3);//ͼ�������
    }

    if(Menu.Set_Mode == Normal_Mode)
    {
        if((Menu.Set_Line == 14) && (Button_Value[1] == 2))//�˳�
        {
            Button_Value[1] = 0;
            Menu_Mode = Page_Select;//�˳�����һҳ
            Menu.Set_Line = 0;
            Menu.Image_Show = false;
            ips200_clear();
        }
        else if((Menu.Set_Line == 7) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        else if((Menu.Set_Line == 8) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        else if((Menu.Set_Line == 9) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        else if((Menu.Set_Line == 10) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        else if((Menu.Set_Line == 11) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Flash_Mode;
        } 
        else if((Menu.Set_Line == 6) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Bin_Image_Flag = 1;
            ips200_clear();
        } 
        else if((Menu.Set_Line == 12) && (Button_Value[1] == 2))
        {
            Button_Value[1] = 0;
            Car.Image_Flag = 1;
            ips200_clear();
        } 
        Line_Change();//���л�
    }
    else if(Menu.Set_Mode == Flash_Mode)//���ò���
    {
        if((Button_Value[0] == 2)&&(Menu.Set_Line == 7))//˳ʱ��ת
        {
            Button_Value[0] = 0;
            flash_union_buffer[50].uint16_type+=10;
        }
        else if((Button_Value[2] == 2)&&(Menu.Set_Line == 7))//��ʱ��ת
        {
            Button_Value[2] = 0;
            flash_union_buffer[50].uint16_type-=10;
        }
        else if(Button_Value[1] == 2)//���ν���
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        }

        if((Button_Value[0] == 2)&&(Menu.Set_Line == 8))//˳ʱ��ת
        {
            Button_Value[0] = 0;
            flash_union_buffer[51].uint16_type+=1;
        }
        else if((Button_Value[2] == 2)&&(Menu.Set_Line == 8))//��ʱ��ת
        {
            Button_Value[2] = 0;
            flash_union_buffer[51].uint16_type-=1;
        }
        else if(Button_Value[1] == 2)//���ν���
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        }

        if((Button_Value[0] == 2)&&(Menu.Set_Line == 9))//˳ʱ��ת
        {
            Button_Value[0] = 0;
            flash_union_buffer[52].uint16_type+=1;
        }
        else if((Button_Value[2] == 2)&&(Menu.Set_Line == 9))//��ʱ��ת
        {
            Button_Value[2] = 0;
            flash_union_buffer[52].uint16_type-=1;
        }
        else if(Button_Value[1] == 2)//���ν���
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        }


        if((Button_Value[0] == 2)&&(Menu.Set_Line == 10))//˳ʱ��ת
        {
            Button_Value[0] = 0;
            flash_union_buffer[53].uint16_type+=1;
        }
        else if((Button_Value[2] == 2)&&(Menu.Set_Line == 10))//��ʱ��ת
        {
            Button_Value[2] = 0;
            flash_union_buffer[53].uint16_type-=1;
        }
        else if(Button_Value[1] == 2)//���ν���
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        }

        if((Button_Value[0] == 2)&&(Menu.Set_Line == 11))//˳ʱ��ת
        {
            Button_Value[0] = 0;
            flash_union_buffer[54].uint16_type+=1;
        }
        else if((Button_Value[2] == 2)&&(Menu.Set_Line == 11))//��ʱ��ת
        {
            Button_Value[2] = 0;
            flash_union_buffer[54].uint16_type-=1;
        }
        else if(Button_Value[1] == 2)//���ν���
        {
            Button_Value[1] = 0;
            Menu.Set_Mode = Normal_Mode;
            Menu.Flash_Set = 1;
        } 
    }

    if(Menu.Flash_Set)//���ν���
    {
        Menu.Flash_Set = 0;
        Menu.Ex_Time = flash_union_buffer[50].uint16_type;
        Menu.Turn_Point = flash_union_buffer[51].uint16_type;
        MyFSM.Simple_Flag = flash_union_buffer[52].uint16_type;
        Down_Angle[0] = flash_union_buffer[53].uint16_type;
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
uint8 Servo_Angle = 40;
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
        flash_union_buffer[0].uint16_type = 37;//��е���·ŽǶ�
        flash_union_buffer[1].uint16_type = 36;
        flash_union_buffer[2].uint16_type = 35;
        flash_union_buffer[3].uint16_type = 33;
        flash_union_buffer[4].uint16_type = 32;
        flash_union_buffer[5].uint16_type = 31;

        flash_union_buffer[50].uint16_type = 100;//�ع�ʱ��
        flash_union_buffer[51].uint16_type = 14;//�ع�ʱ��
        flash_union_buffer[52].uint16_type = 1;
        flash_union_buffer[53].uint16_type = 0;
        flash_union_buffer[54].uint16_type = 0;
        
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    }
    else
    {
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        Down_Angle[0] = flash_union_buffer[0].uint16_type;
        Down_Angle[1] = flash_union_buffer[1].uint16_type;
        Down_Angle[2] = flash_union_buffer[2].uint16_type;
        Down_Angle[3] = flash_union_buffer[3].uint16_type;
        Down_Angle[4] = flash_union_buffer[4].uint16_type;
        Down_Angle[5] = flash_union_buffer[5].uint16_type;

        Menu.Turn_Point = flash_union_buffer[51].uint16_type;
        Menu.Ex_Time = flash_union_buffer[50].uint16_type;
        MyFSM.Simple_Flag = flash_union_buffer[52].uint16_type;
        Image_Flag.Lose_Line_L = flash_union_buffer[53].uint16_type;
        Image_Flag.Lose_Line_R = flash_union_buffer[54].uint16_type;
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
        case Page1:
            Page1_Mode();
        break;
        // case Page2:
        //     Page2_Mode();
        // break;
        case Page9:
            Image_Page();
        break;
    }
}

