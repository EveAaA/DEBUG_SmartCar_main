/**
  ******************************************************************************
  * @file    Callback_Action.c
  * @author  ׯ�ı�
  * @brief   �ص�����
  * @date    11/4/2023
  * 
    @verbatim
    ������Ҫ���ж�ִ�еĺ�����������һ�ļ�
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Call_Back_Action.h"
#include "zf_common_headfile.h"
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define Sensor_CH                  (PIT_CH0 )// ʹ�õ������жϱ�� ����޸� ��Ҫͬ����Ӧ�޸������жϱ���� isr.c �еĵ���
#define Sensor_PRIORITY            (PIT_IRQn)// ��Ӧ�����жϵ��жϱ�� 
fifo_struct uart_data_fifo;

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
**/

/**@brief   ��ʱ����ʼ��
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
void TIM_Init()
{
    pit_ms_init(Sensor_CH, 5);                                                  // ��ʼ�� PIT_CH0 Ϊ�����ж� 5ms ����
    interrupt_set_priority(Sensor_PRIORITY, 0); 
    pit_ms_init(PIT_CH1, 10);                                                  // ��ʼ�� PIT_CH1 Ϊ�����ж� 20ms ����
    pit_ms_init(PIT_CH2, 10);                                                  // ��ʼ�� PIT_CH1 Ϊ�����ж� 20ms ����
}


/**@brief   �������жϺ���
-- @param   ��
-- @auther  ׯ�ı�
-- @date    2023/11/4
**/
void Sensor_Handler()
{
    Gyro_Get_All_Angles();
    Encoder_Process();
}

/**@brief   ��Ŀ���openart����
-- @param   ��
-- @auther  ������
-- @date    2023/12/23
**/
void Uart_Findborder_Receive(void)
{
	
    uart_query_byte(UART_1, &_UART_FINDBORDER.get_data);
    fifo_write_buffer(&uart_data_fifo, &_UART_FINDBORDER.get_data, 1);
	  border.dx = UART_ReadBuffer(&_UART_FINDBORDER);
}

/**@brief   ΢��openart����
-- @param   ��
-- @auther  ������
-- @date    2023/12/23
**/
void Uart_Fine_Tuning_Receive(void)
{
    uart_query_byte(UART_2, &_UART_FINE_TUNING.get_data);
    fifo_write_buffer(&uart_data_fifo, &_UART_FINE_TUNING.get_data, 1);
}
