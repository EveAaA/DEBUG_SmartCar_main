#ifndef _UART_H
#define _UART_H
#include "zf_common_headfile.h"
#define DEBUG_UART 0  //���Դ����շ�   1����debug  0�ر�debug (��Ϊ�����USB-TTL��)
#define UART_BAUDRATE (DEBUG_UART_BAUDRATE)
#define UART_FINDBORDER_CREW (LPUART1_IRQn)
#define UART_FINE_TUNING_CREW (LPUART2_IRQn)
#define UART_RECOGNIZE_PLACE_CREW (LPUART5_IRQn)
#define UART_FINDBORDER  (UART_1)
#define UART_FINE_TUNING (UART_2)
#define UART_RECOGNIZE_PLACE (UART_5)
#define START_FINETUNING (0x05) // ��ʼ��ȡ΢����Ϣ������

typedef struct Uart
{
    uart_index_enum UART_INDEX;        // ע�Դ��ڱ��
    uint8_t uart_get_data[1024]; // ���ڽ��ջ�����
    uint8_t fifo_get_data[1024]; // fifo �������������
	uint32_t index;
    uint8_t get_data;         // �������ݱ���
    uint32 fifo_data_count; // fifo ���ݸ���
	fifo_struct uart_data_fifo;
}UART;

typedef enum Dir{
    LEFT = 0,
    RIGHT,
    STRAIGHT,
}Dir_t;

// �ʹ����йضԿ�Ƭ����Ľṹ�� (�ҵ���Ƭ��־λ, ��ʼ΢����־λ, ��ʼʶ���־λ, ƫ����x,ƫ����y, ʶ��Ƭ����) 
typedef struct FINDBORDER
{
    bool FINDBORDER_FLAG; // ֻ��Ҫ�ж��Ƿ��ҵ���Ƭ��־λ
    float dx;
    Dir_t dir;
}FINDBORDERtypeDef;

typedef struct FINETUNING
{
    bool FINETUNING_FINISH_FLAG; // ֻ��Ҫ�ж��Ƿ�ʼ΢����־λ
	bool IS_BORDER_ALIVE;
    float dx;
    float dy;
    float dz;
}FINETUNINGtypeDef;

typedef struct UnpackData
{
    bool FINDBORDER_DATA_FLAG;
    bool FINETUNING_DATA_FLAG;
	bool FINETUNING_DATA_START_FLAG;
    // ����������ͷ���õ�ʱ����д
}UnpackDataTypeDef;

/*
    �ⲿ���ô��ڽṹ��
*/
extern UnpackDataTypeDef UnpackFlag;
extern FINETUNINGtypeDef FINETUNING_DATA;
extern FINDBORDERtypeDef FINDBORDER_DATA;
extern UART _UART_FINDBORDER;
extern UART _UART_FINE_TUNING;
extern UART _UART_RECOGNIZE_PLACE;

void UART_Init(void);
void UART_init(UART *uart, IRQn_Type UART_PRIORITY, uart_index_enum UART_INDEX);
double UART_ReadBuffer(UART *uart);
void UART_UnpackDataV2(UnpackDataTypeDef* UnpackFlag);
void UART_ResetUnpackFlag(UnpackDataTypeDef *UnpackFlag);
void UART_SendByte(UART* uart, uint8_t data);

#endif

