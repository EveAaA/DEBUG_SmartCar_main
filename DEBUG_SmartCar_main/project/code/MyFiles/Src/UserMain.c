/**
  ******************************************************************************
  * @file    UserMain.c
  * @author  庄文标
  * @brief   主程序
  *
    @verbatim
    全部初始化内容写在UserInit中，主程序while的内容写在UserLoop中
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
 *  @defgroup 内部调用
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
        system_delay_ms(1000); // 闪灯表示异常
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
        system_delay_ms(100);                                                   // 初始化失败 等待 100ms
    }
    printf("\r\n module version:%s", wifi_spi_version);      					// 模块固件版本
    printf("\r\n module mac    :%s", wifi_spi_mac_addr);     					// 模块 MAC 信息
    printf("\r\n module ip     :%s", wifi_spi_ip_addr_port); 					// 模块 IP 地址
    if(1 != WIFI_SPI_AUTO_CONNECT)                                              // 如果没有开启自动连接 就需要手动连接目标 IP
    {
        while(wifi_spi_socket_connect(                                          // 向指定目标 IP 的端口建立 TCP 连接
            "TCP",                                                              // 指定使用TCP方式通讯
            WIFI_SPI_TARGET_IP,                                                 // 指定远端的IP地址，填写上位机的IP地址
            WIFI_SPI_TARGET_PORT,                                               // 指定远端的端口号，填写上位机的端口号，通常上位机默认是8080
            WIFI_SPI_LOCAL_PORT))                                               // 指定本机的端口号
        {
            // 如果一直建立失败 考虑一下是不是没有接硬件复位
            printf("\r\n Connect TCP Servers error, try again.");
            system_delay_ms(100);                                               // 建立连接失败 等待 100ms
        }
    }
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, R_Border, L_Border, Center_Line, NULL, NULL ,NULL);
}

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
 **/

/**@brief   判断时间
-- @param   bool Cond 计时条件
-- @param   uint16 Cnt 计时时间
-- @author  庄文标
-- @date    2024/5/23
**/
bool Bufcnt(bool Cond,uint16 Cnt)
{
    static uint16 Cnt_Last = 0;
    if(Cnt!=Cnt_Last)
    {
        Time_Cnt = 0;
    }
    if(Cond)//满足条件
    {
        if(Time_Cnt == 0)
        {
            Time_Cnt = 1;//开始计时
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

/**@brief   所有初始化内容
-- @param   无
-- @author  庄文标
-- @date    2023/11/4
**/
void User_Init()
{
    system_delay_ms(100);//延时一会等待所有外设上电
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

/**@brief   所有主循环内容
-- @param   无
-- @author  庄文标
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
            // 在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
            memcpy(image_copy[0], Bin_Image[0], MT9V03X_IMAGE_SIZE);
            // 发送图像
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