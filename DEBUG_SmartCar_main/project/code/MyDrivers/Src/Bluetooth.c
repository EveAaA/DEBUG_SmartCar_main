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


float *Num_Address[CH_COUNT] = {NULL};  // 存储需要观察变量的地址
uint8 data_buffer[16];                  // 存储上位机接收到数据
uint8 data_len;                         // 存储接收到的数据长度

/**@brief     蓝牙初始化(统一封装初始化函数)
-- @param     None
-- @auther    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Init(void)
{
    bluetooth_ch9141_init();
}

/**@brief     发送浮点数(通信协议遵守VOFA+ <JustFloat>, 多通道)
-- @param     f_num:需要传输的浮点数
-- @auther    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Send_Float(float *float_add[])
{
    //协议帧尾
    uint8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
    float f_num_buffer[CH_COUNT];
    //将实时更新的变量存入到f_num_buffer当中
    f_num_buffer[CH1] = *float_add[CH1];
    f_num_buffer[CH2] = *float_add[CH2];
    f_num_buffer[CH3] = *float_add[CH3];
    f_num_buffer[CH4] = *float_add[CH4];
    f_num_buffer[CH5] = *float_add[CH5];
    //发送f_num_buffer当中存储的所有变量数据于指定通道
    bluetooth_ch9141_send_buffer((uint8*)(f_num_buffer), sizeof(float) * CH_COUNT);
    //发送帧尾
    bluetooth_ch9141_send_buffer(tail, 4); 
}

/**@brief     设置需要观察的数据变量
-- @param     *float_add[]:存储变量地址的数组   CH:选择通道   Set_Num:需要观察的变量
-- @auther    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Set_Watch_Variable(float *float_add[], CH_NUM CH, float *Set_Num)
{
    //防止数据越界
    if (CH < CH_COUNT)
    {   
        //添加数据变量地址到对应通道
        float_add[CH] = Set_Num;
    }
}

/**@brief     获取上位机传输的数据
-- @param     None
-- @auther    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Get_Message(void)
{
    //获取取得的字符串长度以及将数据存入data_buffer当中
    data_len = bluetooth_ch9141_read_buffer(data_buffer, 16);
}

/**@brief     解析获取到的数据
-- @param     None
-- @auther    戴骐阳
-- @date      2023/11/2
**/
ReceiveData Bluetooth_Analyse_Data(void)
{
    ReceiveData Data;
    //存储标识位
    Data.address = data_buffer[0];
    Data.num = 0;
    //解析数据
    for (uint16 index = 0; index < data_len; index++)
    {
        if (data_buffer[index] >= '0' && data_buffer[index] <= '9')
        {
            Data.num = Data.num * 10 + (data_buffer[index] - '0');
        }
    }
    //0号地址存储标识符，1之后存储需要传入数据
    return Data
}
