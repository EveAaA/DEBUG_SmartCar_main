/**
  ******************************************************************************
  * @file    Bluetooth.c
  * @author  ������
  * @brief   ����������Ϣ
  * @date    2/11/2023
    @verbatim 
    @endverbatim
  * @{
**/

#include "Bluetooth.h"

/**@brief     ������ʼ��(ͳһ��װ��ʼ������)
-- @param     None
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Init(void)
{
    bluetooth_ch9141_init();
}

/**@brief     ���͸�����(ͨ��Э������VOFA+ <JustFloat>)
-- @param     f_num:��Ҫ����ĸ�����
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Send_Float(const float f_num)
{
    //������������һλ����
    float f_buffer[1];
    //Э��֡β
    uint8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
    f_buffer[0] = f_num;
    //��������
    bluetooth_ch9141_send_buffer((uint8*)f_buffer, sizeof(float) * 1);
    //����֡β
    bluetooth_ch9141_send_buffer(tail, 4); 
}
