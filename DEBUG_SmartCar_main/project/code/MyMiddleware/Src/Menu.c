/**
  ******************************************************************************
  * @file    Menu.c
  * @author  庄文标
  * @brief   菜单
  * @date    11/13/2023
    @verbatim
    菜单显示
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
#define FLASH_SECTION_INDEX (127)// 存储数据用的扇区 倒数第一个扇区
#define FLASH_PAGE_INDEX (FLASH_PAGE_3)// 存储数据用的页码 倒数第一个页码

/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief   显示箭头
-- @param   int Row 需要显示的行号
-- @auther  庄文标
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

/**@brief   行切换
-- @param   无
-- @auther  庄文标
-- @date    2024/2/16
**/
static void Line_Change()
{
    if((Rotary.Clockwise) && (Menu.Set_Line < 7))//顺时针转
    {
        Rotary.Clockwise = 0;
        Menu.Set_Line++;
    }
    else if((Rotary.Anticlockwise) && (Menu.Set_Line > 0))//逆时针转
    {
        Rotary.Anticlockwise = 0;
        Menu.Set_Line--;
    }
}


/**@brief   页面选择模式
-- @param   无
-- @auther  庄文标
-- @date    2024/2/16
**/
static void Page_Select_Mode()
{
    Line_Change();//行切换
    Arrow_Display(Menu.Set_Line);//箭头显示
    tft180_show_string(Row_1,Line_0,"Page0");
    tft180_show_string(Row_1,Line_1,"Page1"); 
    tft180_show_string(Row_1,Line_2,"Page2"); 
    tft180_show_string(Row_1,Line_3,"Page3"); 
    tft180_show_string(Row_1,Line_4,"Page4"); 
    tft180_show_string(Row_1,Line_5,"Page5");
    tft180_show_string(Row_1,Line_6,"Page6");
    tft180_show_string(Row_1,Line_7,"Page7");

    if(Rotary.Press)//按键按下
    {
        Rotary.Press = 0;
        Menu_Mode = Menu.Set_Line;
        Menu.Set_Line = 0;
        tft180_clear();
    }   
}

/**@brief    第零页显示
-- @param    无
-- @verbatim 用于查看传感器的数值
-- @auther   庄文标
-- @date     2024/2/16
**/
static void Page0_Mode()
{
    Line_Change();//行切换
    Arrow_Display(Menu.Set_Line);//箭头显示
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

    if(Menu.Set_Line == 7 && Rotary.Press)//退出
    {
        Rotary.Press = 0;
        Menu_Mode = 8;
        Menu.Set_Line = 0;
        Menu.Image_Show = false;
        tft180_clear();
    } 

    if(Menu.Set_Line == 1 && Rotary.Press)//发车
    {
        Rotary.Press = 0;
        Start = 1;
        Menu.Image_Show = true;
    }
    if(Menu.Set_Line == 2 && Rotary.Press)//发车
    {
        Rotary.Press = 0;
        Start = 3;
        // Menu.Image_Show = true;
    }

    if(Menu.Set_Line == 5 && Rotary.Press)//显示图像
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
            // Center_Line[i] = (L_Border[i] + R_Border[i]) >> 1;//求中线
            //求中线最好最后求，不管是补线还是做状态机，全程最好使用一组边线，中线最后求出，不能干扰最后的输出
            tft180_draw_point(Center_Line[i], i, RGB565_BLACK);//显示起点 显示中线
            tft180_draw_point(L_Border[i], i, RGB565_BLUE);//显示起点 显示左边线
            tft180_draw_point(R_Border[i], i, RGB565_RED);//显示起点 显示右边线
        }
        tft180_show_gray_image(0, 0, (const uint8 *)(Bin_Image), MT9V03X_W, MT9V03X_H, (Row_18), (Line_5), 0);
        tft180_show_float(Row_8, Line_6, Image_Erro, 3, 3);
    }
}

/**@brief   第一页显示
-- @param   无
-- @auther  庄文标
-- @date    2024/2/18
**/
static void Page1_Mode()
{
    tft180_show_string(Row_1,Line_0,"Test:");
    tft180_show_float(Row_7,Line_0,flash_union_buffer[0].float_type,2,2);

    Exit_Dis;
    if(Menu.Set_Mode == 0)
    {
        if(Menu.Set_Line == 7 && Rotary.Press)//退出
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
        Line_Change();//行切换
    }
    else if(Menu.Set_Mode == 1)//设置参数
    {
        if(Rotary.Clockwise)//顺时针转
        {
            Rotary.Clockwise = 0;
            flash_union_buffer[Menu.Set_Line].float_type++;
        }
        else if(Rotary.Anticlockwise)//逆时针转
        {
            Rotary.Anticlockwise = 0;
            flash_union_buffer[Menu.Set_Line].float_type--;
        }
        else if(Rotary.Press)//调参结束
        {
            Rotary.Press = 0;
            Menu.Set_Mode = 0;
            Menu.Flash_Set = 1;
        }
    }

    if(Menu.Flash_Set)//调参结束
    {
        Menu.Flash_Set = 0;
        Image_PID.Kp = flash_union_buffer[0].float_type;
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    }

    Arrow_Display(Menu.Set_Line);//箭头显示
}

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   Flash初始化
-- @param   无
-- @auther  庄文标
-- @date    2024/2/18
**/
void Flash_Init()
{
    flash_init();//逐飞flash初始化
    if(!flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))//如果没有数据                      // 判断是否有数据
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

/**@brief   每页显示的内容
-- @param   无
-- @auther  庄文标
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

