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
            tft180_show_string(Row_0,Line_Num*16," ");
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
    tft180_show_string(Row_1,Line_0,"Yaw_Angle:");
    tft180_show_float(Row_11,Line_0,Gyro_YawAngle_Get(),3,1);
    Exit_Dis;

    if(Menu.Set_Line == 7 && Rotary.Press)//退出
    {
        Rotary.Press = 0;
        Menu_Mode = 8;
        Menu.Set_Line = 0;
        tft180_clear();
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

