#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H
#define CH_COUNT 5

/* Includes ------------------------------------------------------------------*/
#include "zf_common_headfile.h"
/* ---------------------------------------------------------------------------*/
typedef enum{
    CH1 = 0,
    CH2,
    CH3,
    CH4,
    CH5,
}CH_NUM;

typedef struct{
    char address[3];   // 存储标志位
    float num;         // 存储数值
}ReceiveData;

// 定义哈希表节点结构
typedef struct {
    char *key;
    void (*function)();
}HashNode;

typedef struct
{
    uint8 RxBuffer[100];//存放数据的地方
    uint8 Receive_Num;//数据索引
    uint8 Equal_pos;//等号的位置
    uint8 Dot_pos;//小数点的位置
    uint8 Integer;//整数部分
    float Decimal;//小数部分
    float Real_Data;//实际的数值
    float P_Data;
    float I_Data;
    float D_Data;
    uint8 Start_Flag;
}ReceiveData_Handle;

extern HashNode hashTable[1024];
extern ReceiveData After_Analyze_Data;
extern float *Num_Address[CH_COUNT];
extern uint8 data_buffer[32];
extern uint8 data_len;
extern ReceiveData_Handle Receivedata;
extern float test_1;
extern float test_2;
extern float test_3;
extern float test_4;
/*函数声明--------------------------------------------------------------------*/
void Bluetooth_Init(void);
void Bluetooth_Send_Float(float *float_add[]);
void Bluetooth_Set_Watch_Variable(float *float_add[], CH_NUM CH, float *Set_Num);
void Bluetooth_Get_Message(void);
ReceiveData Bluetooth_Analyse_Data(void);
void Bluetooth_Store_Data(void);
void Process_Hash(char *inputAddress);
void Handle_LFP_Case(void);
void Handle_LFI_Case(void);
void Handle_RFP_Case(void);
void Handle_RFI_Case(void);
void Handle_RBP_Case(void);
void Handle_RBI_Case(void);
void Handle_LBP_Case(void);
void Handle_LBI_Case(void);
void Handle_GYP_Case(void);
void Handle_GYI_Case(void);
void Handle_GYD_Case(void);
void Handle_GYK_Case(void);
void Get_Message();
#endif