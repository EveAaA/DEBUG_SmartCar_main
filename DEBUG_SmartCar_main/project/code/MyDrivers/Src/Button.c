/**
  ******************************************************************************
  * @file    Button.c
  * @author  ׯ�ı�
  * @brief   ��������
  * @date    11/5/2023
    @verbatim
    �ᴥ����  ���밴��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Button.h"

/* Define\Declare ------------------------------------------------------------*/
int8 Button_Value[5] = {0,0,0,0,0};//��ֵ
int8 Switch_Button_Value[2] = {0,0};

//�����ܽŶ���
#define Button_0 B11
#define Button_1 B15
#define Button_2 B14
#define Button_3 B9
#define Button_4 B10
#define Switch_Button_0 B16
#define Switch_Button_1 B17

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ���а�����ʼ��
-- @param   ��
-- @auther  ׯ�ı�
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

/**@brief    ���а���ɨ��
-- @param    ��
-- @auther   ׯ�ı�
-- @verbatim �����ᴥ�����Ͳ��밴����ע�ⲻ�ɷ��ڶ�ʱ���ж���
-- @verbatim ��Ҫ�����ȡ�ĸ�������״̬���ж϶�Ӧ�ļ�ֵ�����簴��0���ж�Button_Value[0],�ж���ǵ�����
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
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief    ��ȡ�ᴥ������ֵ
-- @param    ��
-- @auther   ׯ�ı�
-- @date     2023/11/5
**/
void Get_Button_Value(int8 KeyNum)
{
    switch(KeyNum)
    {
        case 0:
            if(gpio_get_level(Button_0) == 0)
            {
                system_delay_ms(80);//������ʹ�õİ��������е��
                if(gpio_get_level(Button_0) == 0)//�����ж�
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


/**@brief    ��ȡ���밴����ֵ
-- @param    ��
-- @auther   ׯ�ı�
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