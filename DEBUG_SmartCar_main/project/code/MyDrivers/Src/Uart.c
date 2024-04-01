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
UART _UART_FINDBORDER;
UART _UART_FINE_TUNING;
UART _UART_RECOGNIZE_PLACE;
FINETUNINGtypeDef FINETUNING_DATA = {false, false, 0.0f, 0.0f};
FINDBORDERtypeDef FINDBORDER_DATA = {false, 0.0f, STRAIGHT};  
UnpackDataTypeDef UnpackFlag = {false, false, false}; // �ж�֡ͷ֡β�Ƿ񵽴�

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
	fifo_init(&uart->uart_data_fifo, FIFO_DATA_8BIT, uart->uart_get_data, 64); // ��ʼ�� fifo ���ػ�����
    switch (UART_INDEX)
    {
    case UART_FINDBORDER:
        uart_init(UART_INDEX, UART_BAUDRATE, UART1_TX_B12, UART1_RX_B13); // ��ʼ����������
		interrupt_set_priority(UART_PRIORITY, 0); // ���ö�Ӧ UART_INDEX ���ж����ȼ�Ϊ 0
		uart_rx_interrupt(UART_INDEX, ZF_ENABLE); // ���� UART_INDEX �Ľ����ж�
        break;
    case UART_FINE_TUNING:
        uart_init(UART_INDEX, UART_BAUDRATE, UART2_TX_B18, UART2_RX_B19);
		interrupt_set_priority(UART_PRIORITY, 1); // ���ö�Ӧ UART_INDEX ���ж����ȼ�Ϊ 0
		uart_rx_interrupt(UART_INDEX, ZF_ENABLE); // ���� UART_INDEX �Ľ����ж�        
		break;
    case UART_RECOGNIZE_PLACE:
        uart_init(UART_INDEX, UART_BAUDRATE, UART5_TX_C28, UART5_RX_C29);
        break;
    default:
        break;
    }
	
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
    uart->fifo_data_count = fifo_used(&uart->uart_data_fifo);
    if (uart->fifo_data_count != 0)
    {
        fifo_read_buffer(&uart->uart_data_fifo, uart->fifo_get_data, &uart->fifo_data_count, FIFO_READ_AND_CLEAN);
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

/**
 * @brief: �����ݽ�� (���÷������ڶ�ʱ���жϺ�������)
 * @param: ���ڽṹ��, �������ݽ���ṹ��
 * @return: �Ƿ�������ݽ��
 *
 */
void UART_UnpackData(UART *uart)
{

    uart->fifo_data_count = fifo_used(&uart->uart_data_fifo);
    if (uart->fifo_data_count != 0)
    {
        fifo_read_buffer(&uart->uart_data_fifo, uart->fifo_get_data, &uart->fifo_data_count, FIFO_READ_AND_CLEAN);
            // ͨ�������������ж��ǽ�����ĸ���������
        switch (uart->UART_INDEX)
        {
        case UART_FINDBORDER: // �ҵ�Ŀ���ʱ���ж�
		     if (uart->fifo_get_data[0] == 0x01)
			 {
				FINDBORDER_DATA.FINDBORDER_FLAG = true;
			 }
             else if (uart->fifo_get_data[0] == 0x00)
             {
                FINDBORDER_DATA.FINDBORDER_FLAG = false;
             }
             FINDBORDER_DATA.dx = (float)uart->fifo_get_data[1];
             if (uart->fifo_get_data[2] == 0x10)
             {
                FINDBORDER_DATA.dir = LEFT;
             }
             else if(uart->fifo_get_data[2] == 0x11)
             {
                FINDBORDER_DATA.dir = RIGHT;
             }
             else if(uart->fifo_get_data[2] == 0x12)
             {
                FINDBORDER_DATA.dir = STRAIGHT;
             }
             break;
             

        case UART_FINE_TUNING:
            FINETUNING_DATA.dx = (float)uart->fifo_get_data[0];
            FINETUNING_DATA.dy = (float)uart->fifo_get_data[1];
            if (uart->fifo_get_data[2] == 0x01)
            {
                FINETUNING_DATA.FINETUNING_FINISH_FLAG = false;
				FINETUNING_DATA.IS_BORDER_ALIVE = true;
            }
            else if (uart->fifo_get_data[2] == 0x00)
            {
                FINETUNING_DATA.FINETUNING_FINISH_FLAG = true;
				FINETUNING_DATA.IS_BORDER_ALIVE = true;
            }
			else
			{
				FINETUNING_DATA.IS_BORDER_ALIVE = false;
			}
			break;
        default:
             break;
        }
    }
}

/**
 * @brief : �°汾���н������
 * @param : ���봫�����ݽṹ��
 * @return : None
 * 
 */
void UART_UnpackDataV2(UnpackDataTypeDef* UnpackFlag)
{
    if(UnpackFlag->FINDBORDER_DATA_FLAG)
    {
        UART_UnpackData(&_UART_FINDBORDER);
    }
    if(UnpackFlag->FINETUNING_DATA_FLAG)
    {
        UART_UnpackData(&_UART_FINE_TUNING);
    } 
}

/**
 * @brief : ��������־λ
 * @param : �����־λ�ṹ��
 * @return : None
 * 
 */
void UART_ResetUnpackFlag(UnpackDataTypeDef *UnpackFlag)
{
    UnpackFlag->FINDBORDER_DATA_FLAG = false;
    UnpackFlag->FINETUNING_DATA_FLAG = false;
	// UnpackFlag->FINETUNING_DATA_START_FLAG = false;
}