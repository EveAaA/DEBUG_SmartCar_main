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
// #define Image_debug
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
    #ifdef Image_debug
    printf("Imu_Init\r\n");
    #endif
    #ifndef Image_debug
    tft180_show_string(Row_0, Line_0, "IMU Init ...");
    #endif
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
    #ifdef Image_debug
    printf("Mt9v03x_Init\r\n");
    #endif
    #ifndef Image_debug
    tft180_show_string(Row_0, Line_1, "Mt9v03x Init ...");
    #endif

}

void WIFI_SPI_Init()
{
    while(wifi_spi_init(WIFI_SSID_TEST, WIFI_PASSWORD_TEST))
    {
		printf("\r\n connect wifi failed. \r\n");
        system_delay_ms(100);                                                   // ��ʼ��ʧ�� �ȴ� 100ms
    }
    printf("\r\n module version:%s", wifi_spi_version);      					// ģ��̼��汾
    printf("\r\n module mac    :%s", wifi_spi_mac_addr);     					// ģ�� MAC ��Ϣ
    printf("\r\n module ip     :%s", wifi_spi_ip_addr_port); 					// ģ�� IP ��ַ
    if(1 != WIFI_SPI_AUTO_CONNECT)                                              // ���û�п����Զ����� ����Ҫ�ֶ�����Ŀ�� IP
    {
        while(wifi_spi_socket_connect(                                          // ��ָ��Ŀ�� IP �Ķ˿ڽ��� TCP ����
            "TCP",                                                              // ָ��ʹ��TCP��ʽͨѶ
            WIFI_SPI_TARGET_IP,                                                 // ָ��Զ�˵�IP��ַ����д��λ����IP��ַ
            WIFI_SPI_TARGET_PORT,                                               // ָ��Զ�˵Ķ˿ںţ���д��λ���Ķ˿ںţ�ͨ����λ��Ĭ����8080
            WIFI_SPI_LOCAL_PORT))                                               // ָ�������Ķ˿ں�
        {
            // ���һֱ����ʧ�� ����һ���ǲ���û�н�Ӳ����λ
            printf("\r\n Connect TCP Servers error, try again.");
            system_delay_ms(100);                                               // ��������ʧ�� �ȴ� 100ms
        }
    }
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, R_Border, L_Border, Center_Line, NULL, NULL ,NULL);
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
    #ifndef Image_debug
    tft180_init();
    #endif
    // bluetooth_ch9141_init();
    IMU_Init();
    Mt9v03x_Init();
    #ifdef Image_debug
    WIFI_SPI_Init();
    #endif
    All_Encoder_Init();
    // tft180_show_string(Row_0, Line_2, "Encoder Init ...");
    Rotary_Init();
    // tft180_show_string(Row_0, Line_3, "Rotary Init ...");
    Manipulator_Init();
    Motor_Init();
    // tft180_show_string(Row_0, Line_4, "Motor Init ...");
    Beep_Init();
    // dl1a_init();
    All_PID_Init();
    Flash_Init();
    mt9v03x_set_exposure_time(Menu.Ex_Time);
    UART_Init();
    #ifdef Image_debug
    printf("All_Init\r\n");
    #endif
    #ifndef Image_debug
    tft180_show_string(Row_0, Line_2, "All Init ...");
    #endif
    tft180_clear();
    system_delay_ms(1000);
    TIM_Init();
	interrupt_global_enable(0);
}

/**@brief   ������ѭ������
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/9/12
**/
void User_Loop()
{
    Get_Button_Value(0);
    // if(Receivedata.Start_Flag!=2)
    // {
    //     Start = Receivedata.Start_Flag;
    // }
    if(mt9v03x_finish_flag && Car.Image_Flag)
    {
        mt9v03x_finish_flag = 0;
        Image_Process();
        #ifdef Image_debug
            // �ڷ���ǰ��ͼ�񱸷��ٽ��з��ͣ��������Ա���ͼ�����˺�ѵ�����
            memcpy(image_copy[0], Bin_Image[0], MT9V03X_IMAGE_SIZE);
            // ����ͼ��
            seekfree_assistant_camera_send();
        #endif
    }
    // tft180_show_gray_image(0,0,(uint8*)Bin_Image,MT9V03X_W,MT9V03X_H,MT9V03X_W,MT9V03X_H,0);
    // tft180_show_float(10,100,Image_Erro,3,2);
    // for (int i = Hightest; i < Image_H-1; i++)
    // {
    //     tft180_draw_point(Center_Line[i], i, RGB565_BLACK);
    //     tft180_draw_point(L_Border[i], i, RGB565_BLUE);
    //     tft180_draw_point(R_Border[i], i, RGB565_RED);
    // }
#ifndef Image_debug
    if(Start!=1)
    {
        Menu_Display();
    }
#endif
}