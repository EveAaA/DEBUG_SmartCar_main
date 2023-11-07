#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H
#define CH_COUNT 15

/* Includes ------------------------------------------------------------------*/
#include "zf_common_headfile.h"
#include "UserMain.h"
/* ---------------------------------------------------------------------------*/
typedef enum{
    CH1 = 0,
    CH2,
    CH3,
    CH4,
    CH5,
    CH6,
    CH7,
    CH8,
    CH9,
    CH10,
    CH11,
    CH12,
    CH13,
    CH14,
    CH15
}CH_NUM;

typedef struct{
    char address[3];   // �洢��־λ
    double num;         // �洢��ֵ
}ReceiveData;

// �����ϣ��ڵ�ṹ
typedef struct {
    char *key;
    void (*function)();
}HashNode;

extern HashNode hashTable[1024];
extern ReceiveData After_Analyze_Data;
extern float *Num_Address[CH_COUNT];
extern uint8 data_buffer[32];
extern uint8 data_len;

/*��������--------------------------------------------------------------------*/
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
#endif