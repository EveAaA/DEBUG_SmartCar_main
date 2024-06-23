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
#define UART_CLASSIFY_PIC (0x06) // ��ʼʶ������ͼ��
#define UART_CLASSIFY_SMALLPLACE (0x07) // ��ʼʶ��С���������
#define UART_CLASSIFY_BIGPLACE  (0x08)  // ��ʼʶ������������
#define UART_FINDBORDER_GETBIGPLACE (0x19) // ��ʼ��ȡ�����������
#define UART_FINDBORDER_GETSMALLPLACE (0x20) // ��ʼ��ȡС���������
#define UART_FINDBORDER_GETBORDER (0x21)   // ��ʼ��ȡ��·��Ե��Ŀ���
#define UART_STARTFINETUNING_PLACE (0x10)  // ��ʼ�����ְ�΢����
#define UART_MOVEVOLUMEUP_FLAG (0x11)

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
    WEAPONS,        // ����
    MATERIAL,       // ����
    TRANSPORTATION  // ��ͨ����
}MainType_t;

typedef enum Dir{
    LEFT = 0,
    RIGHT,
    STRAIGHT,
}Dir_t;

// �ʹ����йضԿ�Ƭ����Ľṹ�� (�ҵ���Ƭ��־λ, ��ʼ΢����־λ, ��ʼʶ���־λ, ƫ����x,ƫ����y, ʶ��Ƭ����) 
typedef struct FINDBORDER
{
    bool FINDBORDER_FLAG; // ֻ��Ҫ�ж��Ƿ��ҵ���Ƭ��־λ
    bool FINDBIGPLACE_FLAG; // ֻ��Ҫ�ж��Ƿ��ҵ�������ñ�־λ
    float dx;
    Dir_t dir;
}FINDBORDERtypeDef;

typedef struct FINETUNING
{
    uint8_t FINETUNING_FINISH_FLAG; // ֻ��Ҫ�ж��Ƿ�ʼ΢����־λ
    /**
     * 0x00:  ��ȫδ��ɵ��α�־λ 
     * 0x01:  ��ȫ��ɵ��α�־λ
     * 0x02:  X����ɱ�־λ
     */
	bool IS_BORDER_ALIVE;
    float dx;
    float dy;
    float dz;
}FINETUNINGtypeDef;

// ����λ�ýṹ��
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
    // ����������ͷ���õ�ʱ����д
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
    �ⲿ���ô��ڽṹ��
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

