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
#define START_FINETUNING (0x05) // 开始获取微调信息的数据
#define UART_CLASSIFY_PIC (0x06) // 开始识别分类的图像
#define UART_CLASSIFY_SMALLPLACE (0x07) // 开始识别小类放置区域
#define UART_CLASSIFY_BIGPLACE  (0x08)  // 开始识别大类放置区域
#define UART_FINDBORDER_GETBIGPLACE (0x19) // 开始获取大类放置区域
#define UART_FINDBORDER_GETSMALLPLACE (0x20) // 开始获取小类放置区域
#define UART_FINDBORDER_GETBORDER (0x21)   // 开始获取道路边缘的目标板
#define UART_STARTFINETUNING_PLACE (0x10)  // 开始对数字版微调整
#define UART_MOVEVOLUMEUP_FLAG (0x11)

typedef struct Uart
{
    uart_index_enum UART_INDEX;        // 注冊串口编号
    uint8_t uart_get_data[1024]; // 串口接收缓存区
    uint8_t fifo_get_data[1024]; // fifo 输出读出缓冲区
	uint32_t index;
    uint8_t get_data;         // 接收数据变量
    uint32 fifo_data_count; // fifo 数据个数
	fifo_struct uart_data_fifo;
}UART;

typedef enum Place
{
    nil = -2,
    A = 0,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
}Place_t;

typedef enum MainType
{
    None = -1,
    WEAPONS,        // 武器
    MATERIAL,       // 物资
    TRANSPORTATION  // 交通工具
}MainType_t;

typedef enum Dir{
    LEFT = 0,
    RIGHT,
    STRAIGHT,
}Dir_t;

// 和串口有关对卡片处理的结构体 (找到卡片标志位, 开始微调标志位, 开始识别标志位, 偏移量x,偏移量y, 识别卡片种类) 
typedef struct FINDBORDER
{
    bool FINDBORDER_FLAG; // 只需要判断是否找到卡片标志位
    bool FINDBIGPLACE_FLAG; // 只需要判断是否找到大类放置标志位
    float dx;
    Dir_t dir;
}FINDBORDERtypeDef;

typedef struct FINETUNING
{
    uint8_t FINETUNING_FINISH_FLAG; // 只需要判断是否开始微调标志位
    /**
     * 0x00:  完全未完成调参标志位 
     * 0x01:  完全完成调参标志位
     * 0x02:  X轴完成标志位
     */
	bool IS_BORDER_ALIVE;
    float dx;
    float dy;
    float dz;
}FINETUNINGtypeDef;

// 放置位置结构体
typedef struct PLACE
{
    bool IS_PLACE;
    Place_t place;
    MainType_t Big_Place;
}PLACEtypeDef;

typedef struct CLASSIFY
{
    bool IS_CLASSIFY;
    Place_t place;
    MainType_t type;
}CLASSIFYtypeDef;

typedef struct UnpackData
{
    bool FINDBORDER_DATA_FLAG;
    bool FINETUNING_DATA_FLAG;
	bool FINETUNING_DATA_START_FLAG;
    // 第三个摄像头启用的时候再写
}UnpackDataTypeDef;


typedef struct wareHouse
{
    PLACEtypeDef  SMALL_PLACE;
    PLACEtypeDef  BIG_PLACE;
}wareHouse_t;

typedef struct VOLUMEUP
{
    float AngleErr;
    float HeightErr;
}VOLUMEUPTypeDef;


/*
    外部调用串口结构体
*/
extern UnpackDataTypeDef UnpackFlag;
extern FINETUNINGtypeDef FINETUNING_DATA;
extern FINDBORDERtypeDef FINDBORDER_DATA;
extern CLASSIFYtypeDef   CLASSIFY_DATA;
extern PLACEtypeDef      SMALL_PLACE_DATA;
extern PLACEtypeDef      BIG_PLACE_DATA;
extern VOLUMEUPTypeDef   VOLUMEUP_DATA;
extern UART _UART_FINDBORDER;
extern UART _UART_FINE_TUNING;
extern UART _UART_RECOGNIZE_PLACE;
extern const Place_t PLACE_TABLE[16];
extern const MainType_t  MAIN_TABLE[5];
extern const char* HUGE_PLACE_STR[3];
extern const char* PLACE_TABLE_STR[16];


void UART_Init(void);
void UART_init(UART *uart, IRQn_Type UART_PRIORITY, uart_index_enum UART_INDEX);
double UART_ReadBuffer(UART *uart);
void UART_UnpackDataV2(UnpackDataTypeDef* UnpackFlag);
void UART_ResetUnpackFlag(UnpackDataTypeDef *UnpackFlag);
void UART_SendByte(UART* uart, uint8_t data);

#endif

