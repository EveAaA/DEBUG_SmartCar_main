/**
  ******************************************************************************
  * @file    Bluetooth.c
  * @author  戴骐阳
  * @brief   蓝牙传输信息
  * @date    2/11/2023
    @verbatim 
    @endverbatim
  * @{
**/

#include "Bluetooth.h"

/**@brief     蓝牙初始化(统一封装初始化函数)
-- @param     None
-- @auther    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Init(void)
{
    bluetooth_ch9141_init();
}

/**@brief     发送浮点数(通信协议遵守VOFA+ <JustFloat>)
-- @param     f_num:需要传输的浮点数
-- @auther    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Send_Float(const float f_num)
{
    //将浮点数存入一位数组
    float f_buffer[1];
    //协议帧尾
    uint8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
    f_buffer[0] = f_num;
    //发送数据
    bluetooth_ch9141_send_buffer((uint8*)f_buffer, sizeof(float) * 1);
    //发送帧尾
    bluetooth_ch9141_send_buffer(tail, 4); 
}
