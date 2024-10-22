/**
  ******************************************************************************
  * @file    Bluetooth.c
  * @author  戴骐阳、庄文标
  * @brief   蓝牙传输信息
  * @date    2/11/2023
    @verbatim
    @endverbatim
  * @{
**/

#include "Bluetooth.h"

float *Num_Address[CH_COUNT] = {NULL}; // 存储需要观察变量的地址
uint8 data_buffer[32];                 // 存储上位机接收到数据
uint8 data_len;                        // 存储接收到的数据长度
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
/**@brief     蓝牙初始化(统一封装初始化函数)
-- @param     None
-- @author    戴骐阳
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

/**@brief     printf重定向
-- @param     None
-- @author    庄文标
-- @date      2023/12/6
**/
int32_t fputc (int32_t ch, FILE* f)
{
    bluetooth_ch9141_send_byte((ch & 0xFF));
    return ch;
}

/**@brief     发送浮点数(通信协议遵守VOFA+ <JustFloat>, 多通道)
-- @param     f_num:需要传输的浮点数
-- @author    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Send_Float(float *float_add[])
{
    // 协议帧尾
    uint8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
    float f_num_buffer[CH_COUNT];
    // 将实时更新的变量存入到f_num_buffer当中
    f_num_buffer[CH1] = *float_add[CH1];
    f_num_buffer[CH2] = *float_add[CH2];
    f_num_buffer[CH3] = *float_add[CH3];
    f_num_buffer[CH4] = *float_add[CH4];
    // f_num_buffer[CH5] = *float_add[CH5];
    // 发送f_num_buffer当中存储的所有变量数据于指定通道
    bluetooth_ch9141_send_buffer((uint8 *)(f_num_buffer), sizeof(float) * CH_COUNT);
    // 发送帧尾
    bluetooth_ch9141_send_buffer(tail, 4);
}

/**@brief     设置需要观察的数据变量
-- @param     *float_add[]:存储变量地址的数组   CH:选择通道   Set_Num:需要观察的变量
-- @author    戴骐阳
-- @date      2023/11/2
**/
void Bluetooth_Set_Watch_Variable(float *float_add[], CH_NUM CH, float *Set_Num)
{
    // 防止数据越界
    if (CH < CH_COUNT)
    {
        // 添加数据变量地址到对应通道
        float_add[CH] = Set_Num;
    }
}

/**@brief     获取上位机传输的数据
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/3
**/
void Bluetooth_Get_Message(void)
{
    // memset(data_buffer, 0, 32);
    // 获取取得的字符串长度以及将数据存入data_buffer当中
    data_len = bluetooth_ch9141_read_buffer(data_buffer, 32);
    if(data_len != 0)                                                       // 收到了消息 读取函数会返回实际读取到的数据个数
    {
        bluetooth_ch9141_send_buffer(data_buffer, data_len);    
    }
}

/**@brief     获取上位机传输的数据，并解包
-- @param     None
-- @author    庄文标
-- @date      2024/4/16
**/
void Get_Message()
{
    uint8 i = 0;
    bluetooth_ch9141_read_buffer(data_buffer, 32);
    Receivedata.RxBuffer[Receivedata.Receive_Num] = data_buffer[0];
    if(Receivedata.RxBuffer[Receivedata.Receive_Num] == '!')//获取到结束位置
    {
        for(i = 0;i<=Receivedata.Receive_Num;i++)
        {
            if(Receivedata.RxBuffer[i] == '=')//寻找等号位置
            {
                Receivedata.Equal_pos = i;
                break;
            }
            else
            {
                Receivedata.Equal_pos = 0;//非法值
            }
        }
        if(Receivedata.Equal_pos!=0)//存在等号
        {
            for(i = 0;i<=Receivedata.Receive_Num;i++)
            {
                if(Receivedata.RxBuffer[i] == '.')//寻找小数点位置
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
            }
            else if(Receivedata.RxBuffer[Receivedata.Equal_pos - 1] == 'I')
            {
                Receivedata.I_Data = Receivedata.Real_Data;
            }
            else
            {
                Receivedata.D_Data = Receivedata.Real_Data;
            }
            printf("%f,%f,%f\r\n",Receivedata.P_Data,Receivedata.I_Data,Receivedata.D_Data);
        }
        else//没有找到等号，说明为发车指令
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

/**@brief     解析获取到的数据
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/3
**/
ReceiveData Bluetooth_Analyse_Data(void)
{
    ReceiveData Data;
    // 暂存float数值的字符串缓存
    char float_buffer[sizeof(data_buffer) - 3];
    // 存储标识位
    Data.address[0] = data_buffer[0];
    Data.address[1] = data_buffer[1];
    Data.address[2] = data_buffer[2];
    Data.num = 0.0f;
    // 解析数据
    for (uint16 index = 3; index < data_len; index++)
    {
        float_buffer[index - 3] = data_buffer[index];
    }
    Data.num = func_str_to_float(float_buffer);
    //////debug监视代码/////////////////////////////////////////////////////
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

/**@brief     储存上位机输出获得到的数据
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/3
**/
void Bluetooth_Store_Data(void)
{
    system_delay_ms(10);
    // 获取缓存当中得数据存入data_buffer当中
    Bluetooth_Get_Message();
    // 如果有数据则解析数据
    if (data_len != 0)
    {
        // 创建After_Analyze_Data存储标志位变量，以及数值
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

/**@brief     获取哈希值
-- @param     str: 输入字符串转换为哈希值
-- @author    戴骐阳
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

/**@brief     创建哈希表
-- @param     inputAddress: 输入需要访问的地址
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Process_Hash(char *inputAddress)
{
    memset(hashTable, 0, sizeof(hashTable));

    // 添加相应的键-值对到哈希表中
    /*******************速度环部分*******************/
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
    /*******************角度环部分*******************/
    hashTable[Hash_Function("Gyp")].key = "Gyp";
    hashTable[Hash_Function("Gyp")].function = &Handle_GYP_Case;
    hashTable[Hash_Function("Gyi")].key = "Gyi";
    hashTable[Hash_Function("Gyi")].function = &Handle_GYI_Case;
    hashTable[Hash_Function("Gyd")].key = "Gyd";
    hashTable[Hash_Function("Gyd")].function = &Handle_GYD_Case;
    hashTable[Hash_Function("Gyk")].key = "Gyk";
    hashTable[Hash_Function("Gyk")].function = &Handle_GYK_Case;
    /***********************************************/
    // 根据输入地址查找对应的哈希表项并执行相应函数
    HashNode *resultNode = &hashTable[Hash_Function(inputAddress)];
    if (resultNode->key != NULL && strcmp(resultNode->key, inputAddress) == 0)
    {
        resultNode->function();
    }
}

/**@brief     处理LFP函数(改变LF轮kp数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_LFP_Case(void)
{

}

/**@brief     处理LFI函数(改变LF轮ki数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_LFI_Case(void)
{
}

/**@brief     处理RFP函数(改变RF轮kp数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_RFP_Case(void)
{
}

/**@brief     处理RFI函数(改变RF轮kI数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_RFI_Case(void)
{
}

/**@brief     处理RBP函数(改变RB轮kp数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_RBP_Case(void)
{

}

/**@brief     处理RBI函数(改变RB轮ki数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_RBI_Case(void)
{
}

/**@brief     处理LBP函数(改变LB轮kp数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_LBP_Case(void)
{

}

/**@brief     处理LBI函数(改变LB轮ki数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_LBI_Case(void)
{

}

/**@brief     处理GYP函数(改变角度环kp数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_GYP_Case(void)
{

}

/**@brief     处理GYI函数(改变角度环ki数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_GYI_Case(void)
{

}

/**@brief     处理GYD函数(改变角度环kd数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_GYD_Case(void)
{

}

/**@brief     处理GYK函数(改变角度环K数值)
-- @param     None
-- @author    戴骐阳
-- @date      2023/11/4
**/
void Handle_GYK_Case(void)
{

}