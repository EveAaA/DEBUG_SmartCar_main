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

float *Num_Address[CH_COUNT] = {NULL}; // �洢��Ҫ�۲�����ĵ�ַ
uint8 data_buffer[32];                 // �洢��λ�����յ�����
uint8 data_len;                        // �洢���յ������ݳ���
HashNode hashTable[1024];              
float test_1 = 0.0;
float test_2 = 0.0;
float test_3 = 0.0f;

/**@brief     ������ʼ��(ͳһ��װ��ʼ������)
-- @param     None
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Init(void)
{
    while(1)
    {
        if(!bluetooth_ch9141_init())
            break;
    }
    Bluetooth_Set_Watch_Variable(Num_Address, CH1, &test_1);
    Bluetooth_Set_Watch_Variable(Num_Address, CH2, &test_2);
}

/**@brief     printf�ض���
-- @param     None
-- @auther    ׯ�ı�
-- @date      2023/12/6
**/
int32_t fputc (int32_t ch, FILE* f)
{
    bluetooth_ch9141_send_byte((ch & 0xFF));
    return ch;
}

/**@brief     ���͸�����(ͨ��Э������VOFA+ <JustFloat>, ��ͨ��)
-- @param     f_num:��Ҫ����ĸ�����
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Send_Float(float *float_add[])
{
    // Э��֡β
    uint8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
    float f_num_buffer[CH_COUNT];
    // ��ʵʱ���µı������뵽f_num_buffer����
    f_num_buffer[CH1] = *float_add[CH1];
    f_num_buffer[CH2] = *float_add[CH2];
    f_num_buffer[CH3] = *float_add[CH3];
    f_num_buffer[CH4] = *float_add[CH4];
    f_num_buffer[CH5] = *float_add[CH5];
    // ����f_num_buffer���д洢�����б���������ָ��ͨ��
    bluetooth_ch9141_send_buffer((uint8 *)(f_num_buffer), sizeof(float) * CH_COUNT);
    // ����֡β
    bluetooth_ch9141_send_buffer(tail, 4);
}

/**@brief     ������Ҫ�۲�����ݱ���
-- @param     *float_add[]:�洢������ַ������   CH:ѡ��ͨ��   Set_Num:��Ҫ�۲�ı���
-- @auther    ������
-- @date      2023/11/2
**/
void Bluetooth_Set_Watch_Variable(float *float_add[], CH_NUM CH, float *Set_Num)
{
    // ��ֹ����Խ��
    if (CH < CH_COUNT)
    {
        // ������ݱ�����ַ����Ӧͨ��
        float_add[CH] = Set_Num;
    }
}

/**@brief     ��ȡ��λ�����������
-- @param     None
-- @auther    ������
-- @date      2023/11/3
**/
void Bluetooth_Get_Message(void)
{
    memset(data_buffer, 0, 32);
    // ��ȡȡ�õ��ַ��������Լ������ݴ���data_buffer����
    data_len = bluetooth_ch9141_read_buffer(data_buffer, 32);
}

/**@brief     ������ȡ��������
-- @param     None
-- @auther    ������
-- @date      2023/11/3
**/
ReceiveData Bluetooth_Analyse_Data(void)
{
    ReceiveData Data;
    // �ݴ�float��ֵ���ַ�������
    char float_buffer[sizeof(data_buffer) - 3];
    // �洢��ʶλ
    Data.address[0] = data_buffer[0];
    Data.address[1] = data_buffer[1];
    Data.address[2] = data_buffer[2];
    Data.num = 0.0f;
    // ��������
    for (uint16 index = 3; index < data_len; index++)
    {
        float_buffer[index - 3] = data_buffer[index];
    }
    Data.num = func_str_to_float(float_buffer);
    //////debug���Ӵ���/////////////////////////////////////////////////////
    /*
    char test[64] = {""};
    func_uint_to_str(test, Data.num);
    bluetooth_ch9141_send_buffer(data_buffer, data_len);
    bluetooth_ch9141_send_byte('\n');
    bluetooth_ch9141_send_buffer(Data.address, sizeof(Data.address));
    bluetooth_ch9141_send_byte('\n');
    bluetooth_ch9141_send_string("DataNum:");
    bluetooth_ch9141_send_string(test);
    */
    /////////////////////////////////////////////////////////////////////////
    return Data;
}

/**@brief     ������λ�������õ�������
-- @param     None
-- @auther    ������
-- @date      2023/11/3
**/
void Bluetooth_Store_Data(void)
{
    system_delay_ms(10);
    // ��ȡ���浱�е����ݴ���data_buffer����
    Bluetooth_Get_Message();
    // ������������������
    if (data_len != 0)
    {
        // ����After_Analyze_Data�洢��־λ�������Լ���ֵ
        ReceiveData After_Analyze_Data;
        After_Analyze_Data = Bluetooth_Analyse_Data();
        /*
        bluetooth_ch9141_send_byte('\n');
        char test2[64] = {""};
        func_uint_to_str(test2, After_Analyze_Data.num);
        bluetooth_ch9141_send_buffer(After_Analyze_Data.address, sizeof(After_Analyze_Data.address));
        bluetooth_ch9141_send_byte('\n');
        bluetooth_ch9141_send_string(test2);
        */
        Process_Hash(After_Analyze_Data.address);
    }
}

/**@brief     ��ȡ��ϣֵ
-- @param     str: �����ַ���ת��Ϊ��ϣֵ
-- @auther    ������
-- @date      2023/11/4
**/
int Hash_Function(char *str)
{
    int hash = 0;
    for (int i = 0; i < 3; i++)
    {
        hash += str[i];
    }
    return hash;
}

/**@brief     ������ϣ��
-- @param     inputAddress: ������Ҫ���ʵĵ�ַ
-- @auther    ������
-- @date      2023/11/4
**/
void Process_Hash(char *inputAddress)
{
    memset(hashTable, 0, sizeof(hashTable));

    // �����Ӧ�ļ�-ֵ�Ե���ϣ����
    /*******************�ٶȻ�����*******************/
    hashTable[Hash_Function("lfp")].key = "lfp";
    hashTable[Hash_Function("lfp")].function = &Handle_LFP_Case;
    hashTable[Hash_Function("lfi")].key = "lfi";
    hashTable[Hash_Function("lfi")].function = &Handle_LFI_Case;
    hashTable[Hash_Function("rfp")].key = "rfp";
    hashTable[Hash_Function("rfp")].function = &Handle_RFP_Case;
    hashTable[Hash_Function("rfi")].key = "rfi";
    hashTable[Hash_Function("rfi")].function = &Handle_RFI_Case;
    hashTable[Hash_Function("lbp")].key = "lbp";
    hashTable[Hash_Function("lbp")].function = &Handle_LBP_Case;
    hashTable[Hash_Function("lbi")].key = "lbi";
    hashTable[Hash_Function("lbi")].function = &Handle_LBI_Case;
    hashTable[Hash_Function("rbp")].key = "rbp";
    hashTable[Hash_Function("rbp")].function = &Handle_RBP_Case;
    hashTable[Hash_Function("rbi")].key = "rbi";
    hashTable[Hash_Function("rbi")].function = &Handle_RBI_Case;
    /*******************�ǶȻ�����*******************/
    hashTable[Hash_Function("Gyp")].key = "Gyp";
    hashTable[Hash_Function("Gyp")].function = &Handle_GYP_Case;
    hashTable[Hash_Function("Gyi")].key = "Gyi";
    hashTable[Hash_Function("Gyi")].function = &Handle_GYI_Case;
    hashTable[Hash_Function("Gyd")].key = "Gyd";
    hashTable[Hash_Function("Gyd")].function = &Handle_GYD_Case;
    hashTable[Hash_Function("Gyk")].key = "Gyk";
    hashTable[Hash_Function("Gyk")].function = &Handle_GYK_Case;
    /***********************************************/
    // ���������ַ���Ҷ�Ӧ�Ĺ�ϣ���ִ����Ӧ����
    HashNode *resultNode = &hashTable[Hash_Function(inputAddress)];
    if (resultNode->key != NULL && strcmp(resultNode->key, inputAddress) == 0)
    {
        resultNode->function();
    }
}

/**@brief     ����LFP����(�ı�LF��kp��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_LFP_Case(void)
{

}

/**@brief     ����LFI����(�ı�LF��ki��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_LFI_Case(void)
{
}

/**@brief     ����RFP����(�ı�RF��kp��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_RFP_Case(void)
{
}

/**@brief     ����RFI����(�ı�RF��kI��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_RFI_Case(void)
{
}

/**@brief     ����RBP����(�ı�RB��kp��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_RBP_Case(void)
{

}

/**@brief     ����RBI����(�ı�RB��ki��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_RBI_Case(void)
{
}

/**@brief     ����LBP����(�ı�LB��kp��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_LBP_Case(void)
{

}

/**@brief     ����LBI����(�ı�LB��ki��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_LBI_Case(void)
{

}

/**@brief     ����GYP����(�ı�ǶȻ�kp��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_GYP_Case(void)
{

}

/**@brief     ����GYI����(�ı�ǶȻ�ki��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_GYI_Case(void)
{

}

/**@brief     ����GYD����(�ı�ǶȻ�kd��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_GYD_Case(void)
{

}

/**@brief     ����GYK����(�ı�ǶȻ�K��ֵ)
-- @param     None
-- @auther    ������
-- @date      2023/11/4
**/
void Handle_GYK_Case(void)
{

}