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
int8 Button[5] = {B11,B15,B14,B9,B10};
int8 Button_Temp[5] = {0,0,0,0,0};
int8 Switch_Button_Value[2] = {0,0};
uint16 Key_Time[5] = {0,0,0,0,0};
#define Key_Sleep_Time 4
//按键管脚定义
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
    gpio_init(Button[0],GPI,0,GPI_PULL_UP);
    gpio_init(Button[1],GPI,0,GPI_PULL_UP);
    gpio_init(Button[2],GPI,0,GPI_PULL_UP);
    gpio_init(Button[3],GPI,0,GPI_PULL_UP);
    gpio_init(Button[4],GPI,0,GPI_PULL_UP);
    gpio_init(Switch_Button_0,GPI,0,GPI_PULL_UP);
    gpio_init(Switch_Button_1,GPI,0,GPI_PULL_UP);
}

/**@brief    所有按键扫描
-- @param    无
-- @auther   庄文标
-- @verbatim 包括轻触按键和拨码按键
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

/**@brief   所有按键消抖
-- @param   无
-- @auther  庄文标
-- @date    2024/1/21
**/
void Key_Delay()
{
    for(uint8 Num = 0;Num<=4;Num++)
    {
        if(Key_Time[Num] != 0)
        {
            Key_Time[Num]++;
        }
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
-- @verbatim 采用状态机的方法
-- @date     2023/11/5
**/
void Get_Button_Value(int8 KeyNum)
{
	switch(Button_Temp[KeyNum])
    {
        case 0://空闲状态
            if(gpio_get_level(Button[KeyNum]) == 0)
            {
                Button_Temp[KeyNum] = 1;//消抖
                Key_Time[KeyNum] = 1;//开始计时
            }
        break;
        case 1://消抖状态
            if((Key_Time[KeyNum] > Key_Sleep_Time) && (gpio_get_level(Button[KeyNum]) == 0))//大于消抖时间且仍为低电平
            {
                Button_Temp[KeyNum] = 2;//进入等待释放或长按计时
            }
        break;
        case 2://判断长按还是短按
            if(gpio_get_level(Button[KeyNum]) == 1)//如果按键抬起
            {
                Button_Temp[KeyNum] = 0;//进入等待释放或长按计时
                Button_Value[KeyNum] = 2;//判断为短按               
            }

            if((Key_Time[KeyNum] > Key_Sleep_Time*100) && (gpio_get_level(Button[KeyNum]) == 0))//大于长按时间
            {
                Button_Temp[KeyNum] = 3;//进入等待释放
                Button_Value[KeyNum] = 3;     
            }
        break;
        case 3://等待释放
            if(gpio_get_level(Button[KeyNum]) == 1)
            {
                Button_Temp[KeyNum] = 0;//进入等待释放或长按计时
                Button_Value[KeyNum] = 0;               
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