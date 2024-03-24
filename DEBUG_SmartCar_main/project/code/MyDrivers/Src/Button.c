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
int8 Button[5] = {B11,B15,B14,B9,B10};
int8 Button_Temp[5] = {0,0,0,0,0};
int8 Switch_Button_Value[2] = {0,0};
uint16 Key_Time[5] = {0,0,0,0,0};
#define Key_Sleep_Time 4
//�����ܽŶ���
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
    gpio_init(Button[0],GPI,0,GPI_PULL_UP);
    gpio_init(Button[1],GPI,0,GPI_PULL_UP);
    gpio_init(Button[2],GPI,0,GPI_PULL_UP);
    gpio_init(Button[3],GPI,0,GPI_PULL_UP);
    gpio_init(Button[4],GPI,0,GPI_PULL_UP);
    gpio_init(Switch_Button_0,GPI,0,GPI_PULL_UP);
    gpio_init(Switch_Button_1,GPI,0,GPI_PULL_UP);
}

/**@brief    ���а���ɨ��
-- @param    ��
-- @auther   ׯ�ı�
-- @verbatim �����ᴥ�����Ͳ��밴��
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

/**@brief   ���а�������
-- @param   ��
-- @auther  ׯ�ı�
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
 *  @defgroup �ڲ�����
 *  @brief
 *
**/

/**@brief    ��ȡ�ᴥ������ֵ
-- @param    ��
-- @auther   ׯ�ı�
-- @verbatim ����״̬���ķ���
-- @date     2023/11/5
**/
void Get_Button_Value(int8 KeyNum)
{
	switch(Button_Temp[KeyNum])
    {
        case 0://����״̬
            if(gpio_get_level(Button[KeyNum]) == 0)
            {
                Button_Temp[KeyNum] = 1;//����
                Key_Time[KeyNum] = 1;//��ʼ��ʱ
            }
        break;
        case 1://����״̬
            if((Key_Time[KeyNum] > Key_Sleep_Time) && (gpio_get_level(Button[KeyNum]) == 0))//��������ʱ������Ϊ�͵�ƽ
            {
                Button_Temp[KeyNum] = 2;//����ȴ��ͷŻ򳤰���ʱ
            }
        break;
        case 2://�жϳ������Ƕ̰�
            if(gpio_get_level(Button[KeyNum]) == 1)//�������̧��
            {
                Button_Temp[KeyNum] = 0;//����ȴ��ͷŻ򳤰���ʱ
                Button_Value[KeyNum] = 2;//�ж�Ϊ�̰�               
            }

            if((Key_Time[KeyNum] > Key_Sleep_Time*100) && (gpio_get_level(Button[KeyNum]) == 0))//���ڳ���ʱ��
            {
                Button_Temp[KeyNum] = 3;//����ȴ��ͷ�
                Button_Value[KeyNum] = 3;     
            }
        break;
        case 3://�ȴ��ͷ�
            if(gpio_get_level(Button[KeyNum]) == 1)
            {
                Button_Temp[KeyNum] = 0;//����ȴ��ͷŻ򳤰���ʱ
                Button_Value[KeyNum] = 0;               
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