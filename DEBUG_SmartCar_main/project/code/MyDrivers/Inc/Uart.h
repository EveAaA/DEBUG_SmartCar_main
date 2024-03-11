#ifndef _UART_H
#define _UART_H
#include "zf_common_headfile.h"
#define DEBUG_UART 0  //测试串口收发   1开启debug  0关闭debug (因为不想接USB-TTL线)
#define UART_BAUDRATE (DEBUG_UART_BAUDRATE)
#define UART_FINDBORDER_CREW (LPUART1_IRQn)
#define UART_FINE_TUNING_CREW (LPUART2_IRQn)
#define UART_RECOGNIZE_PLACE_CREW (LPUART5_IRQn)
#define UART_FINDBORDER  (UART_1)
#define UART_FINE_TUNING (UART_2)
#define UART_RECOGNIZE_PLACE (UART_5)

typedef struct Uart
{
    uart_index_enum UART_INDEX;        // 注冊串口编号
    int8 uart_get_data[64]; // 串口接收缓存区
    int8 fifo_get_data[64]; // fifo 输出读出缓冲区

    uint8 get_data;         // 接收数据变量
    uint32 fifo_data_count; // fifo 数据个数
} UART;

// 和串口有关对卡片处理的结构体 (找到卡片标志位, 开始微调标志位, 开始识别标志位, 偏移量x,偏移量y, 识别卡片种类) 
typedef struct OpenartBorder
{
    bool isFindBorder;
    bool isFineTuning;
    bool isRecognizeBorder;
    float dx;
    float dy;
    uint8_t borderType; // uint8够放所有类型
}borderTypeDef;


/*
    外部调用串口结构体
*/
extern borderTypeDef border;
extern UART _UART_FINDBORDER;
extern UART _UART_FINE_TUNING;
extern UART _UART_RECOGNIZE_PLACE;

void UART_Init(void);
void UART_init(UART *uart, IRQn_Type UART_PRIORITY, uart_index_enum UART_INDEX);
double UART_ReadBuffer(UART *uart);

#endif
