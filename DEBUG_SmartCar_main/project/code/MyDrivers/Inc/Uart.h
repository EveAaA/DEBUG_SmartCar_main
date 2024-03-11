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

typedef struct Uart
{
    uart_index_enum UART_INDEX;        // ע�Դ��ڱ��
    int8 uart_get_data[64]; // ���ڽ��ջ�����
    int8 fifo_get_data[64]; // fifo �������������

    uint8 get_data;         // �������ݱ���
    uint32 fifo_data_count; // fifo ���ݸ���
} UART;

// �ʹ����йضԿ�Ƭ����Ľṹ�� (�ҵ���Ƭ��־λ, ��ʼ΢����־λ, ��ʼʶ���־λ, ƫ����x,ƫ����y, ʶ��Ƭ����) 
typedef struct OpenartBorder
{
    bool isFindBorder;
    bool isFineTuning;
    bool isRecognizeBorder;
    float dx;
    float dy;
    uint8_t borderType; // uint8������������
}borderTypeDef;


/*
    �ⲿ���ô��ڽṹ��
*/
extern borderTypeDef border;
extern UART _UART_FINDBORDER;
extern UART _UART_FINE_TUNING;
extern UART _UART_RECOGNIZE_PLACE;

void UART_Init(void);
void UART_init(UART *uart, IRQn_Type UART_PRIORITY, uart_index_enum UART_INDEX);
double UART_ReadBuffer(UART *uart);

#endif
