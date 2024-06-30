/**
  ******************************************************************************
  * @file    UserMain.c
  * @author  ׯ�ı�
  * @brief   ������
  *
    @verbatim
    ȫ����ʼ������д��UserInit�У�������while������д��UserLoop��
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "UserMain.h"
#include "zf_common_headfile.h"
#include "User_FSM.h"
#include "My_FSM.h"
/* Define\Declare ------------------------------------------------------------*/
uint16 Start = 2;
uint16 Time_Cnt = 0;
uint8 image_copy[MT9V03X_H][MT9V03X_W];

/**
 ******************************************************************************
 *  @defgroup �ڲ�����
 *  @brief
 *
 **/
void IMU_Init()
{
    while (1)
    {
        if (imu660ra_init())
            system_delay_ms(1000);
        else
            break;
        system_delay_ms(1000);
    }
    system_delay_ms(100);
    tft180_show_string(Row_0, Line_0, "IMU Init ...");
    Gyro_Offset_Init();
}

void Mt9v03x_Init()
{
    while (1)
    {
        if (mt9v03x_init())
            system_delay_ms(1000);
            // tft180_show_string(0, 16, "mt9v03x reinit.");
        else
            break;
        system_delay_ms(1000); // ���Ʊ�ʾ�쳣
    }
    tft180_show_string(Row_0, Line_1, "Mt9v03x Init ...");
}

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
 **/

/**@brief   �ж�ʱ��
-- @param   bool Cond ��ʱ����
-- @param   uint16 Cnt ��ʱʱ��
-- @author  ׯ�ı�
-- @date    2024/5/23
**/
bool Bufcnt(bool Cond,uint16 Cnt)
{
    static uint16 Cnt_Last = 0;
    if(Cnt!=Cnt_Last)
    {
        Time_Cnt = 0;
    }
    if(Cond)//��������
    {
        if(Time_Cnt == 0)
        {
            Time_Cnt = 1;//��ʼ��ʱ
        }
    }
    else
    {
        Time_Cnt = 0;
    }
    Cnt_Last = Cnt;
    if(Time_Cnt >= Cnt)
    {
        Time_Cnt = 0;
        return true;
    }
    else
    {
        return false;
    }
}

/**@brief   ���г�ʼ������
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/4
**/
void User_Init()
{
    system_delay_ms(100);//��ʱһ��ȴ����������ϵ�
    tft180_init();
    IMU_Init();
    Mt9v03x_Init();
    All_Encoder_Init();
    tft180_show_string(Row_0, Line_2, "Encoder Init ...");
    Rotary_Init();
    bluetooth_ch9141_init();
    tft180_show_string(Row_0, Line_3, "Rotary Init ...");
    Manipulator_Init();
    Motor_Init();
    tft180_show_string(Row_0, Line_4, "Motor Init ...");
    Beep_Init();
    // dl1a_init();
    All_PID_Init();
    Flash_Init();
    UART_Init();
    // seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIRELESS_UART);
    // seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
    tft180_show_string(Row_0, Line_5, "Soft Init ...");
    tft180_clear();
    system_delay_ms(1000);
    TIM_Init();
    // Beep(On);
    // Beep(Off);
	interrupt_global_enable(0);
}
/**@brief   ������ѭ������
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/9/12
**/
void User_Loop()
{
    // printf("fuck\r\n");
    // printf("speed = %f\r\n",Encoer_Speed[0]);
    // if(mt9v03x_finish_flag)
    // {
    //     memcpy(image_copy[0], Bin_Image[0], MT9V03X_IMAGE_SIZE);
    //     seekfree_assistant_camera_send();
    // }
    if(Receivedata.Start_Flag==0)
    {
        Start = Receivedata.Start_Flag;
    }
    Menu_Display();
}