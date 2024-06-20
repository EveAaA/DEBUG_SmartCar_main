/**
  ******************************************************************************
  * @file    Bluetooth.c
  * @author  ��������ׯ�ı�
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
ReceiveData_Handle Receivedata = 
{
    .Receive_Num = 0,
    .Equal_pos = 0,
    .Dot_pos = 0,
    .Start_Flag = 2,
    .Servo_Rotary = 5,
};
float test_1 = 0.0;
float test_2 = 0.0;
float test_3 = 0.0;
float test_4 = 0.0;
/**@brief     ������ʼ��(ͳһ��װ��ʼ������)
-- @param     None
-- @author    ������
-- @date      2023/11/2
**/
void Bluetooth_Init(void)
{
    // while(1)
    // {
    //     if(!wireless_uart_init())
    //         break;
    // }
    wireless_uart_init();
//    Bluetooth_Set_Watch_Variable(Num_Address, CH1, &test_1);
//    Bluetooth_Set_Watch_Variable(Num_Address, CH2, &test_2);
//    Bluetooth_Set_Watch_Variable(Num_Address, CH3, &test_3);
//    Bluetooth_Set_Watch_Variable(Num_Address, CH4, &test_4);
}

/**@brief     printf�ض���
-- @param     None
-- @author    ׯ�ı�
-- @date      2023/12/6
**/
int32_t fputc (int32_t ch, FILE* f)
{
    bluetooth_ch9141_send_byte((ch & 0xFF));
    return ch;
}

/**@brief     ���͸�����(ͨ��Э������VOFA+ <JustFloat>, ��ͨ��)
-- @param     f_num:��Ҫ����ĸ�����
-- @author    ������
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
    // f_num_buffer[CH5] = *float_add[CH5];
    // ����f_num_buffer���д洢�����б���������ָ��ͨ��
    bluetooth_ch9141_send_buffer((uint8 *)(f_num_buffer), sizeof(float) * CH_COUNT);
    // ����֡β
    bluetooth_ch9141_send_buffer(tail, 4);
}

/**@brief     ������Ҫ�۲�����ݱ���
-- @param     *float_add[]:�洢������ַ������   CH:ѡ��ͨ��   Set_Num:��Ҫ�۲�ı���
-- @author    ������
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
-- @author    ������
-- @date      2023/11/3
**/
void Bluetooth_Get_Message(void)
{
    // memset(data_buffer, 0, 32);
    // ��ȡȡ�õ��ַ��������Լ������ݴ���data_buffer����
    data_len = bluetooth_ch9141_read_buffer(data_buffer, 32);
    if(data_len != 0)                                                       // �յ�����Ϣ ��ȡ�����᷵��ʵ�ʶ�ȡ�������ݸ���
    {
        bluetooth_ch9141_send_buffer(data_buffer, data_len);    
    }
}

/**@brief     ��ȡ��λ����������ݣ������
-- @param     None
-- @author    ׯ�ı�
-- @date      2024/4/16
**/
void Get_Message()
{
    uint8 i = 0;
    bluetooth_ch9141_read_buffer(data_buffer, 32);
    Receivedata.RxBuffer[Receivedata.Receive_Num] = data_buffer[0];
    if(Receivedata.RxBuffer[Receivedata.Receive_Num] == '!')//��ȡ������λ��
    {
        for(i = 0;i<=Receivedata.Receive_Num;i++)
        {
            if(Receivedata.RxBuffer[i] == '=')//Ѱ�ҵȺ�λ��
            {
                Receivedata.Equal_pos = i;
                break;
            }
            else
            {
                Receivedata.Equal_pos = 0;//�Ƿ�ֵ
            }
        }
        if(Receivedata.Equal_pos!=0)//���ڵȺ�
        {
            for(i = 0;i<=Receivedata.Receive_Num;i++)
            {
                if(Receivedata.RxBuffer[i] == '.')//Ѱ��С����λ��
                {
                    Receivedata.Dot_pos = i;
                    break;
                }
            }
            Receivedata.Integer = (Receivedata.RxBuffer[Receivedata.Equal_pos + 1] - '0');
            Receivedata.Decimal = (Receivedata.RxBuffer[Receivedata.Dot_pos + 1] - '0')*0.1f+(Receivedata.RxBuffer[Receivedata.Dot_pos + 2] - '0')*0.01f;
            Receivedata.Real_Data = Receivedata.Integer + Receivedata.Decimal; 

            if(Receivedata.RxBuffer[Receivedata.Equal_pos - 1] == 'P')
            {
                Receivedata.P_Data = Receivedata.Real_Data;
                Receivedata.I_Data = 0;
                Receivedata.D_Data = 0;
            }
            else if(Receivedata.RxBuffer[Receivedata.Equal_pos - 1] == 'I')
            {
                Receivedata.P_Data = 0;
                Receivedata.I_Data = Receivedata.Real_Data;
                Receivedata.D_Data = 0;
            }
            else
            {
                Receivedata.P_Data = 0;
                Receivedata.I_Data = 0;
                Receivedata.D_Data = Receivedata.Real_Data;
            }
            // printf("%f,%f,%f\r\n",Receivedata.P_Data,Receivedata.I_Data,Receivedata.D_Data);
        }
        else//û���ҵ��Ⱥţ�˵��Ϊ����ָ��
        {
            if(Receivedata.RxBuffer[0] == 'S' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 't')
            {
                Receivedata.Start_Flag = 1;
            }
            else if(Receivedata.RxBuffer[0] == 'S' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 'p')
            {
                Receivedata.Start_Flag = 0;
            }
            else if(Receivedata.RxBuffer[0] == 'P' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 't')
            {
                Receivedata.Start_Flag = 3;
            }
            else if(Receivedata.RxBuffer[0] == 'I' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 't')
            {
                Receivedata.Start_Flag = 4;
            }
            else if(Receivedata.RxBuffer[0] == 'W' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 'e')
            {
                Receivedata.Servo_Rotary = 0;
            }
            else if(Receivedata.RxBuffer[0] == 'B' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 'k')
            {
                Receivedata.Servo_Rotary = 1;
            }
            else if(Receivedata.RxBuffer[0] == 'R' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 'd')
            {
                Receivedata.Servo_Rotary = 2;
            }
            else if(Receivedata.RxBuffer[0] == 'Y' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 'w')
            {
                Receivedata.Servo_Rotary = 3;
            }
            // else if(Receivedata.RxBuffer[0] == 'W' && Receivedata.RxBuffer[Receivedata.Receive_Num - 1] == 'e')
            // {
            //     Receivedata.Servo_Rotary = 4;
            // }
            // printf("%d\r\n",Receivedata.Start_Flag);
        }
        Receivedata.Receive_Num=0;
    }
    else
    {
        Receivedata.Receive_Num++;
    }
}

/**@brief     ������ȡ��������
-- @param     None
-- @author    ������
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
-- @author    ������
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
-- @author    ������
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
-- @author    ������
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
-- @author    ������
-- @date      2023/11/4
**/
void Handle_LFP_Case(void)
{

}

/**@brief     ����LFI����(�ı�LF��ki��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_LFI_Case(void)
{
}

/**@brief     ����RFP����(�ı�RF��kp��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_RFP_Case(void)
{
}

/**@brief     ����RFI����(�ı�RF��kI��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_RFI_Case(void)
{
}

/**@brief     ����RBP����(�ı�RB��kp��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_RBP_Case(void)
{

}

/**@brief     ����RBI����(�ı�RB��ki��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_RBI_Case(void)
{
}

/**@brief     ����LBP����(�ı�LB��kp��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_LBP_Case(void)
{

}

/**@brief     ����LBI����(�ı�LB��ki��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_LBI_Case(void)
{

}

/**@brief     ����GYP����(�ı�ǶȻ�kp��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_GYP_Case(void)
{

}

/**@brief     ����GYI����(�ı�ǶȻ�ki��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_GYI_Case(void)
{

}

/**@brief     ����GYD����(�ı�ǶȻ�kd��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_GYD_Case(void)
{

}

/**@brief     ����GYK����(�ı�ǶȻ�K��ֵ)
-- @param     None
-- @author    ������
-- @date      2023/11/4
**/
void Handle_GYK_Case(void)
{

}