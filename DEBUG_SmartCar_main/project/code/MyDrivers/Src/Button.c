/**
  ******************************************************************************
  * @file    Button.c
  * @author  庄文标
  * @brief   按键驱动
  * @date    11/5/2023
    @verbatim
    轻触按键  拨码按键
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Button.h"

/* Define\Declare ------------------------------------------------------------*/
int8 Button_Value[5] = {0,0,0,0,0};//键值
int8 Switch_Button_Value[2] = {0,0};

//按键管脚定义
#define Button_0 B11
#define Button_1 B15
#define Button_2 B14
#define Button_3 B9
#define Button_4 B10
#define Switch_Button_0 B16
#define Switch_Button_1 B17

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
**/

/**@brief   所有按键初始化
-- @param   无
-- @auther  庄文标
-- @date    2023/11/5
**/
void All_Button_Init()
{
    gpio_init(Button_0,GPI,0,GPI_PULL_UP);
    gpio_init(Button_1,GPI,0,GPI_PULL_UP);
    gpio_init(Button_2,GPI,0,GPI_PULL_UP);
    gpio_init(Button_3,GPI,0,GPI_PULL_UP);
    gpio_init(Button_4,GPI,0,GPI_PULL_UP);
    gpio_init(Switch_Button_0,GPI,0,GPI_PULL_UP);
    gpio_init(Switch_Button_1,GPI,0,GPI_PULL_UP);
}

/**@brief    所有按键扫描
-- @param    无
-- @auther   庄文标
-- @verbatim 包括轻触按键和拨码按键，注意不可放在定时器中断中
-- @verbatim 需要具体获取哪个按键的状态就判断对应的键值，例如按键0就判断Button_Value[0],判断完记得清零
-- @date     2023/11/5
**/
void All_Button_Scan()
{
    for(int Button_Num = 0;Button_Num <= 4;Button_Num++)
    {
        Get_Button_Value(Button_Num);
    }

    for(int Switch_Button_Num = 0;Switch_Button_Num <= 1;Switch_Button_Num++)
    {
        Get_Switch_Button_Value(Switch_Button_Num);
    }
}


/**
 ******************************************************************************
 *  @defgroup 内部调用
 *  @brief
 *
**/

/**@brief    获取轻触按键键值
-- @param    无
-- @auther   庄文标
-- @date     2023/11/5
**/
void Get_Button_Value(int8 KeyNum)
{
    switch(KeyNum)
    {
        case 0:
            if(gpio_get_level(Button_0) == 0)
            {
                system_delay_ms(80);//消抖，使用的按键抖动有点久
                if(gpio_get_level(Button_0) == 0)//二次判断
                {
                    Button_Value[0] = 1;
                }
            }
        break;
        case 1:
            if(gpio_get_level(Button_1) == 0)
            {
                system_delay_ms(80);
                if(gpio_get_level(Button_1) == 0)
                {
                    Button_Value[1] = 1;
                }
            }
        break;
        case 2:
            if(gpio_get_level(Button_2) == 0)
            {
                system_delay_ms(80);
                if(gpio_get_level(Button_2) == 0)
                {
                    Button_Value[2] = 1;
                }
            }
        break;
        case 3:
            if(gpio_get_level(Button_3) == 0)
            {
                system_delay_ms(80);
                if(gpio_get_level(Button_3) == 0)
                {
                    Button_Value[3] = 1;
                }
            }
        break;
        case 4:
            if(gpio_get_level(Button_4) == 0)
            {
                system_delay_ms(80);
                if(gpio_get_level(Button_4) == 0)
                {
                    Button_Value[4] = 1;
                }
            }
        break;

    }
}


/**@brief    获取拨码按键键值
-- @param    无
-- @auther   庄文标
-- @date     2023/11/7
**/
void Get_Switch_Button_Value(int8 KeyNum)
{
    switch(KeyNum)
    {
        case 0:
            if(gpio_get_level(Switch_Button_0) == 0)
            {
                Switch_Button_Value[0] = 1;
            }
            else if (gpio_get_level(Switch_Button_0) == 1)
            {
                Switch_Button_Value[0] = 0;
            }
        break;
        case 1:
            if(gpio_get_level(Switch_Button_1) == 0)
            {
                Switch_Button_Value[1] = 1;
            }
            else if (gpio_get_level(Switch_Button_1) == 1)
            {
                Switch_Button_Value[1] = 0;
            }
        break;
    }
}