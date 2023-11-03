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
    uint8 address;
    uint16 num;
}ReceiveData;

extern ReceiveData After_Analyze_Data;
extern float *Num_Address[CH_COUNT];
extern uint8 data_buffer[16];
extern uint8 data_len;
/*º¯ÊýÉùÃ÷--------------------------------------------------------------------*/
void Bluetooth_Init(void);
void Bluetooth_Send_Float(float *float_add[]);
void Bluetooth_Set_Watch_Variable(float *float_add[], CH_NUM CH, float *Set_Num);
void Bluetooth_Get_Message(void);
ReceiveData Bluetooth_Analyse_Data(void);

#endif