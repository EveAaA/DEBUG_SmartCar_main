/**
  ******************************************************************************
  * @file    Uart.c
  * @author  ������
  * @brief   ����ͨѶ
  *
    @verbatim

    @endverbatim
  * @{
**/

/*****************include**********************************************************/
#include "Uart.h"

/*************************************************************************************/

/*****************define*********************************************************/
extern fifo_struct uart_data_fifo;
UART _UART_FINDBORDER;
UART _UART_FINE_TUNING;
UART _UART_RECOGNIZE_PLACE;

/**@brief    ��ʼ������(�ⲿ����)
-- @param    None
-- @return   None
-- @auther   ������
-- @date     2023/12/23
**/
void UART_Init(void)
{
    UART_init(&_UART_FINDBORDER, UART_FINDBORDER_CREW, UART_FINDBORDER);
    UART_init(&_UART_FINE_TUNING, UART_FINE_TUNING_CREW, UART_FINE_TUNING);
    UART_init(&_UART_RECOGNIZE_PLACE, UART_RECOGNIZE_PLACE_CREW, UART_RECOGNIZE_PLACE);
}

/**@brief    ��ʼ������(�ڲ�����)
-- @param    ����ṹ���ַ, �����ж���
-- @return   None
-- @auther   ������
-- @date     2023/12/23
**/
void UART_init(UART *uart, IRQn_Type UART_PRIORITY, uart_index_enum UART_INDEX)
{
    uart->UART_INDEX = UART_INDEX;
    fifo_init(&uart_data_fifo, FIFO_DATA_8BIT, uart->uart_get_data, 64); // ��ʼ�� fifo ���ػ�����
    switch (UART_INDEX)
    {
    case UART_FINDBORDER:
        uart_init(UART_INDEX, UART_BAUDRATE, UART1_TX_B12, UART1_RX_B13); // ��ʼ����������
        break;
    case UART_FINE_TUNING:
        uart_init(UART_INDEX, UART_BAUDRATE, UART2_TX_B18, UART2_RX_B19);
        break;
    case UART_RECOGNIZE_PLACE:
        uart_init(UART_INDEX, UART_BAUDRATE, UART5_TX_C28, UART5_RX_C29);
        break;
    default:
        break;
    }
    uart_rx_interrupt(UART_INDEX, ZF_ENABLE); // ���� UART_INDEX �Ľ����ж�
    interrupt_set_priority(UART_PRIORITY, 0); // ���ö�Ӧ UART_INDEX ���ж����ȼ�Ϊ 0
    uart_write_byte(UART_INDEX, '\r');        // ����س�
    uart_write_byte(UART_INDEX, '\n');        // �������
}

/**@brief    ��ȡ���浱�е�����
-- @param    ������Ҫ��ȡ���ݵĽṹ���ַ
-- @return   ����double��������
-- @auther   ������
-- @date     2023/12/23
**/
double UART_ReadBuffer(UART *uart)
{
    uart->fifo_data_count = fifo_used(&uart_data_fifo);
    if (uart->fifo_data_count != 0)
    {
        fifo_read_buffer(&uart_data_fifo, uart->fifo_get_data, &uart->fifo_data_count, FIFO_READ_AND_CLEAN);
        #if DEBUG_UART
            uart_write_buffer(uart->UART_INDEX, uart->fifo_get_data, uart->fifo_data_count);
        #endif
        // ��Ϊ�Ӵ��ڶ�ȡ������Ϊʮ�����ƣ����Կ���ֱ��ǿתdouble(�����Ų�Ӱ��)
        return (double)uart->fifo_get_data[0];
    }
    else
    {
        return NULL;
    }
}
