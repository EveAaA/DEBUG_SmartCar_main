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


float *Num_Address[CH_COUNT] = {NULL};  // �洢��Ҫ�۲�����ĵ�ַ
uint8 data_buffer[16];                  // �洢��λ�����յ�����
uint8 data_len;                         // �洢���յ������ݳ���

/**@brief     ������ʼ��(ͳһ��װ��ʼ������)
-- @param     None
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Init(void)
{
    bluetooth_ch9141_init();
}

/**@brief     ���͸�����(ͨ��Э������VOFA+ <JustFloat>, ��ͨ��)
-- @param     f_num:��Ҫ����ĸ�����
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Send_Float(float *float_add[])
{
    //Э��֡β
    uint8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
    float f_num_buffer[CH_COUNT];
    //��ʵʱ���µı������뵽f_num_buffer����
    f_num_buffer[CH1] = *float_add[CH1];
    f_num_buffer[CH2] = *float_add[CH2];
    f_num_buffer[CH3] = *float_add[CH3];
    f_num_buffer[CH4] = *float_add[CH4];
    f_num_buffer[CH5] = *float_add[CH5];
    //����f_num_buffer���д洢�����б���������ָ��ͨ��
    bluetooth_ch9141_send_buffer((uint8*)(f_num_buffer), sizeof(float) * CH_COUNT);
    //����֡β
    bluetooth_ch9141_send_buffer(tail, 4); 
}

/**@brief     ������Ҫ�۲�����ݱ���
-- @param     *float_add[]:�洢������ַ������   CH:ѡ��ͨ��   Set_Num:��Ҫ�۲�ı���
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Set_Watch_Variable(float *float_add[], CH_NUM CH, float *Set_Num)
{
    //��ֹ����Խ��
    if (CH < CH_COUNT)
    {   
        //������ݱ�����ַ����Ӧͨ��
        float_add[CH] = Set_Num;
    }
}

/**@brief     ��ȡ��λ�����������
-- @param     None
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Get_Message(void)
{
    //��ȡȡ�õ��ַ��������Լ������ݴ���data_buffer����
    data_len = bluetooth_ch9141_read_buffer(data_buffer, 16);
}

/**@brief     ������ȡ��������
-- @param     None
-- @auther    ������
-- @date      2023/11/2
**/
ReceiveData Bluetooth_Analyse_Data(void)
{
    ReceiveData Data;
    //�洢��ʶλ
    Data.address = data_buffer[0];
    Data.num = 0;
    //��������
    for (uint16 index = 0; index < data_len; index++)
    {
        if (data_buffer[index] >= '0' && data_buffer[index] <= '9')
        {
            Data.num = Data.num * 10 + (data_buffer[index] - '0');
        }
    }
    //0�ŵ�ַ�洢��ʶ����1֮��洢��Ҫ��������
    return Data
}
