/**
  ******************************************************************************
  * @file    Meau.c
  * @author  庄文标
  * @brief   菜单
  * @date    11/13/2023
    @verbatim
    菜单显示
    @endverbatim
  * @{
**/


/* Includes ------------------------------------------------------------------*/
#include "Meau.h"

/* Define\Declare ------------------------------------------------------------*/
int Set_Line = 0;
int Show_Mode = -1;
/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/


/**@brief   显示箭头
-- @param   int Row 需要显示的行号
-- @auther  庄文标
-- @date    2023/11/13
**/
void Arrow_Display(int Line)
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

/**@brief   每页显示的内容
-- @param   无
-- @auther  庄文标
-- @date    2023/11/13
**/
void Meau_Display()
{
    static uint8_t Page;
    static uint8_t Page_Last;
    static uint8_t Line;
    if((Button_Value[4]) && (Set_Line < 7))
    {
        Button_Value[4] = 0;
        Set_Line++;
    }
    if((Button_Value[0]) && (Set_Line > 0))
    {
        Button_Value[0] = 0;
        Set_Line--;
    }

    if((Switch_Button_Value[0] == 0) && (Switch_Button_Value[1] == 0))
    {
        Page = 0;
    }
    else if((Switch_Button_Value[0] == 1) && (Switch_Button_Value[1] == 0))
    {
        Page = 1;
    }
    else if((Switch_Button_Value[0] == 1) && (Switch_Button_Value[1] == 1))
    {
        Page = 2;
    }

    Arrow_Display(Set_Line);

    switch(Page)
    {
        case 0:
            if((Show_Mode == -1) && (Page == 0))
            {
                tft180_show_string(Row_1,Line_0,"Gyro_Value");//陀螺仪的值
                tft180_show_string(Row_1,Line_1,"Encoder_Value");//编码器的值
                tft180_show_string(Row_1,Line_2,"Image_Value");//图像的误差
                tft180_show_string(Row_1,Line_3,"Image_Picture");//灰度图
                tft180_show_string(Row_1,Line_4,"Image_Bin_Picture");//二值化图
            }
            if(Button_Value[2])
            {
                Button_Value[2] = 0;
                switch (Set_Line)
                {
                    case 0:
                        Show_Mode = 0;
                    break;
                    case 1:
                        Show_Mode = 1;
                    break;
                }
                tft180_clear();
            }
            switch (Show_Mode)
            {
                case 0:
                    tft180_show_string(Row_1,Line_0,"Yaw_Angle:");
                    tft180_show_float(Row_11,Line_0,Gyro_YawAngle_Get(),3,1);
                break;
                case 1:
                    tft180_show_string(Row_1,Line_0,"LF_Speed:");
                    tft180_show_float(Row_11,Line_0,Get_LF_Speed(),3,1);
                    tft180_show_string(Row_1,Line_1,"LB_Speed:");
                    tft180_show_float(Row_11,Line_1,Get_LB_Speed(),3,1);
                    tft180_show_string(Row_1,Line_2,"RF_Speed:");
                    tft180_show_float(Row_11,Line_2,Get_RF_Speed(),3,1);
                    tft180_show_string(Row_1,Line_3,"RB_Speed:");
                    tft180_show_float(Row_11,Line_3,Get_RB_Speed(),3,1);
                break;
            }
        break;
        case 1:
            switch (Line)
            {
                case 0:
                break;
                case 1:
                break;
                case 2:
                break;
                case 3:
                break;
                case 4:
                break;
                case 5:
                break;
                case 6:
                break;
            }
        break;
        case 2:
            switch (Line)
            {
                case 0:
                break;
                case 1:
                break;
                case 2:
                break;
                case 3:
                break;
                case 4:
                break;
                case 5:
                break;
                case 6:
                break;
            }
        break;
    }
    Page_Last = Page;
    if(Page_Last != Page)
    {
        tft180_clear();
    }
}

