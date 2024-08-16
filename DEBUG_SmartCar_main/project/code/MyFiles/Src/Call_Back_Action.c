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
#define Sensor_CH (PIT_CH0)        // ʹ�õ������жϱ�� ����޸� ��Ҫͬ����Ӧ�޸������жϱ���� isr.c �еĵ���
#define Sensor_PRIORITY (PIT_IRQn) // ��Ӧ�����жϵ��жϱ��

/**
 ******************************************************************************
 *  @defgroup �ⲿ����
 *  @brief
 *
 **/

/**@brief   ��ʱ����ʼ��
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/4
**/
void TIM_Init()
{
  pit_ms_init(Sensor_CH, 5); // ��ʼ�� PIT_CH0 Ϊ�����ж� 5ms ����
  interrupt_set_priority(Sensor_PRIORITY, 0);
  pit_ms_init(PIT_CH2, 15); // ��ʼ�� PIT_CH2 Ϊ�����ж� 15ms ����
  pit_ms_init(PIT_CH3, 1);  // ��ʼ�� PIT_CH3 Ϊ�����ж� 1ms ����
}

/**@brief   �������жϺ���
-- @param   ��
-- @author  ׯ�ı�
-- @date    2023/11/4
**/
void Sensor_Handler()
{
  Gyro_Get_All_Angles();
  Encoder_Process();
}

void Uart_Findborder_Hard_Receive(void)
{
  static uint8_t tailLastData;
  uart_query_byte(UART_4, &_UART_RECOGNIZE_PLACE.get_data);
  _UART_RECOGNIZE_PLACE.fifo_get_data[_UART_RECOGNIZE_PLACE.index] = _UART_RECOGNIZE_PLACE.get_data;
  _UART_RECOGNIZE_PLACE.index++;
  // fifo_write_buffer(&_UART_FINDBORDER.uart_data_fifo, &_UART_FINDBORDER.get_data, 1);
  if (_UART_RECOGNIZE_PLACE.get_data == 0x8f && tailLastData == 0x7f)
  {
    // UnpackFlag.FINDBORDER_DATA_FLAG = true;
    if (_UART_RECOGNIZE_PLACE.index == 5)
    {
      // printf("+++++++++++++++++\n");
      // // printf("index: %d\n", _UART_RECOGNIZE_PLACE.index);
      // for (uint32_t i = 0; i < _UART_RECOGNIZE_PLACE.index; i++)
      // {
      //   printf("%x ", _UART_RECOGNIZE_PLACE.fifo_get_data[i]);
      // }
      // printf("\n");
      // // printf("%x  \n", _UART_RECOGNIZE_PLACE.get_data);
      // printf("+++++++++++++++++\n");
      // ���ֽ�ת������
      if (_UART_RECOGNIZE_PLACE.fifo_get_data[0] == 0x01)
      {
        HARDBORDER_DATA.FINDBORDER_FLAG = true;
      }
      else if (_UART_RECOGNIZE_PLACE.fifo_get_data[0] == 0x00)
      {
        HARDBORDER_DATA.FINDBORDER_FLAG = false;
      }
      FINDBORDER_DATA.dx = (float)_UART_FINDBORDER.fifo_get_data[1];
      if (_UART_RECOGNIZE_PLACE.fifo_get_data[2] == 0x10)
      {
        HARDBORDER_DATA.dir = LEFT;
      }
      else if (_UART_RECOGNIZE_PLACE.fifo_get_data[2] == 0x11)
      {
        HARDBORDER_DATA.dir = RIGHT;
      }
      else if (_UART_RECOGNIZE_PLACE.fifo_get_data[2] == 0x12)
      {
        HARDBORDER_DATA.dir = STRAIGHT;
      }
      memset(_UART_RECOGNIZE_PLACE.fifo_get_data, 0, sizeof(_UART_RECOGNIZE_PLACE.fifo_get_data));
      _UART_RECOGNIZE_PLACE.index = 0;
    }
    else
    {
      memset(_UART_RECOGNIZE_PLACE.fifo_get_data, 0, sizeof(_UART_RECOGNIZE_PLACE.fifo_get_data));
      _UART_RECOGNIZE_PLACE.index = 0;
    }
  }
  tailLastData = _UART_RECOGNIZE_PLACE.get_data;
}

/**@brief   ��Ŀ���openart����
-- @param   ��
-- @author  ������
-- @date    2023/12/23
**/
void Uart_Findborder_Receive(void)
{
  static uint8_t tailLastData;
  uart_query_byte(UART_1, &_UART_FINDBORDER.get_data);
  _UART_FINDBORDER.fifo_get_data[_UART_FINDBORDER.index] = _UART_FINDBORDER.get_data;
  _UART_FINDBORDER.index++;
  // fifo_write_buffer(&_UART_FINDBORDER.uart_data_fifo, &_UART_FINDBORDER.get_data, 1);
  if (_UART_FINDBORDER.get_data == 0x8f && tailLastData == 0x7f)
  {
    // UnpackFlag.FINDBORDER_DATA_FLAG = true;
    if (_UART_FINDBORDER.index == 5)
    {
      // printf("+++++++++++++++++\n");
      // printf("index: %d\n", _UART_FINDBORDER.index);
      // for (uint32_t i = 0; i < _UART_FINDBORDER.index; i++)
      // {
      //   printf("%x ", _UART_FINDBORDER.fifo_get_data[i]);
      // }
      // printf("\n");
      // printf("%x  \n", _UART_FINDBORDER.get_data);
      // printf("+++++++++++++++++\n");
      // ���ֽ�ת������
      if (_UART_FINDBORDER.fifo_get_data[0] == 0x01)
      {
        FINDBORDER_DATA.FINDBORDER_FLAG = true;
      }
      else if (_UART_FINDBORDER.fifo_get_data[0] == 0x00)
      {
        FINDBORDER_DATA.FINDBORDER_FLAG = false;
      }
      FINDBORDER_DATA.dx = (float)_UART_FINDBORDER.fifo_get_data[1];
      if (_UART_FINDBORDER.fifo_get_data[2] == 0x10)
      {
        FINDBORDER_DATA.dir = LEFT;
      }
      else if (_UART_FINDBORDER.fifo_get_data[2] == 0x11)
      {
        FINDBORDER_DATA.dir = RIGHT;
      }
      else if (_UART_FINDBORDER.fifo_get_data[2] == 0x12)
      {
        FINDBORDER_DATA.dir = STRAIGHT;
      }
      memset(_UART_FINDBORDER.fifo_get_data, 0, sizeof(_UART_FINDBORDER.fifo_get_data));
      _UART_FINDBORDER.index = 0;
    }
    else
    {
      memset(_UART_FINDBORDER.fifo_get_data, 0, sizeof(_UART_FINDBORDER.fifo_get_data));
      _UART_FINDBORDER.index = 0;
    }
  }
  else if (_UART_FINDBORDER.get_data == 0x7e && tailLastData == 0xfe)
  {
    // printf("index: %d\n", _UART_FINDBORDER.index);
    if (_UART_FINDBORDER.index == 5)
    {
      //printf("+++++++++++++++++\n");
      // printf("index: %d\n", _UART_FINDBORDER.index);
      // for (uint32_t i = 0; i < _UART_FINDBORDER.index; i++)
      // {
      //   printf("%x ", _UART_FINDBORDER.fifo_get_data[i]);
      // }
      // printf("\n");
      // printf("%x  \n", _UART_FINDBORDER.get_data);
      // printf("+++++++++++++++++\n");
      UnpackFlag.FINETUNING_DATA_FLAG = true;
      if (_UART_FINDBORDER.fifo_get_data[0] == 0x01)
      {
        FINDBORDER_DATA.FINDBIGPLACE_FLAG = true;
      }
      else
      {
        FINDBORDER_DATA.FINDBIGPLACE_FLAG = false;
      }
      memset(_UART_FINDBORDER.fifo_get_data, 0, sizeof(_UART_FINDBORDER.fifo_get_data));
      _UART_FINDBORDER.index = 0;
    }
    else
    {
      memset(_UART_FINDBORDER.fifo_get_data, 0, sizeof(_UART_FINDBORDER.fifo_get_data));
      _UART_FINDBORDER.index = 0;
    }
  }
  else if (_UART_FINDBORDER.get_data == 0xbf && tailLastData == 0xfc)
  {
    if (_UART_FINDBORDER.index == 7)
    {
      UnpackFlag.FINETUNING_DATA_FLAG = true;
      FINETUNING_DATA.dx = (float)((_UART_FINDBORDER.fifo_get_data[1] >> 7) == 0) ? ((_UART_FINDBORDER.fifo_get_data[0] + (_UART_FINDBORDER.fifo_get_data[1] << 8))) : (-(65536 - (_UART_FINDBORDER.fifo_get_data[0] + (_UART_FINDBORDER.fifo_get_data[1] << 8))));
      FINETUNING_DATA.dy = (float)((_UART_FINDBORDER.fifo_get_data[3] >> 7) == 0) ? ((_UART_FINDBORDER.fifo_get_data[2] + (_UART_FINDBORDER.fifo_get_data[3] << 8))) : (-(65536 - (_UART_FINDBORDER.fifo_get_data[2] + (_UART_FINDBORDER.fifo_get_data[3] << 8))));
      FINETUNING_DATA.FINETUNING_FINISH_FLAG = _UART_FINDBORDER.fifo_get_data[4];
      // printf("dx: %f dy: %f \r\n", FINETUNING_DATA.dx, FINETUNING_DATA.dy);
      if (FINETUNING_DATA.dx != -999 && FINETUNING_DATA.dy != -999)
      {
        FINETUNING_DATA.IS_BORDER_ALIVE = true;
      }
      else
      {
        FINETUNING_DATA.IS_BORDER_ALIVE = false;
      }
      memset(_UART_FINDBORDER.fifo_get_data, 0, sizeof(_UART_FINDBORDER.fifo_get_data));
      _UART_FINDBORDER.index = 0;
    }
    else
    {
      memset(_UART_FINDBORDER.fifo_get_data, 0, sizeof(_UART_FINDBORDER.fifo_get_data));
      _UART_FINDBORDER.index = 0;
    }
  }
  tailLastData = _UART_FINDBORDER.get_data;
}

/**@brief   ΢��openart����
-- @param   ��
-- @auther  ������
-- @date    2023/12/23
**/
void Uart_Fine_Tuning_Receive(void)
{
  static uint8_t lastData;
  uart_query_byte(UART_2, &_UART_FINE_TUNING.get_data);
  _UART_FINE_TUNING.fifo_get_data[_UART_FINE_TUNING.index] = _UART_FINE_TUNING.get_data;
  _UART_FINE_TUNING.index++;

  // fifo_write_buffer(&_UART_FINE_TUNING.uart_data_fifo, &_UART_FINE_TUNING.get_data, 1);
  // printf("index: %d\n", _UART_FINE_TUNING.index);
  // ΢��״̬
  if (_UART_FINE_TUNING.get_data == 0xbf && lastData == 0xfc)
  {

    if (_UART_FINE_TUNING.index == 7)
    {
      // printf("+++++++++++++++++\n");
      // printf("index: %d\n", _UART_FINE_TUNING.index);
      // for (uint32_t i = 0; i < _UART_FINE_TUNING.index; i++)
      // {
      //   printf("%x ", _UART_FINE_TUNING.fifo_get_data[i]);
      // }
      // printf("\n");
      // printf("%x  \n", _UART_FINE_TUNING.get_data);
      // ���ֽ�ת������
      UnpackFlag.FINETUNING_DATA_FLAG = true;

      FINETUNING_DATA.dx = (float)((_UART_FINE_TUNING.fifo_get_data[1] >> 7) == 0) ? ((_UART_FINE_TUNING.fifo_get_data[0] + (_UART_FINE_TUNING.fifo_get_data[1] << 8))) : (-(65536 - (_UART_FINE_TUNING.fifo_get_data[0] + (_UART_FINE_TUNING.fifo_get_data[1] << 8))));
      FINETUNING_DATA.dy = (float)((_UART_FINE_TUNING.fifo_get_data[3] >> 7) == 0) ? ((_UART_FINE_TUNING.fifo_get_data[2] + (_UART_FINE_TUNING.fifo_get_data[3] << 8))) : (-(65536 - (_UART_FINE_TUNING.fifo_get_data[2] + (_UART_FINE_TUNING.fifo_get_data[3] << 8))));
      FINETUNING_DATA.FINETUNING_FINISH_FLAG = _UART_FINE_TUNING.fifo_get_data[4];
      // printf("dx: %f dy: %f \r\n", FINETUNING_DATA.dx, FINETUNING_DATA.dy);
      if (FINETUNING_DATA.dx != -999 && FINETUNING_DATA.dy != -999)
      {
        FINETUNING_DATA.IS_BORDER_ALIVE = true;
      }
      else
      {
        FINETUNING_DATA.IS_BORDER_ALIVE = false;
      }
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
    else
    {
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
  }
  // ʶ��Ŀ������ʶ������״̬
  else if (_UART_FINE_TUNING.get_data == 0xcf && lastData == 0xfb)
  {
    if (_UART_FINE_TUNING.index == 3)
    {
      // printf("+++++++++++++++++\n");
      // printf("index: %d\n", _UART_FINE_TUNING.index);
      // for (uint32_t i = 0; i < _UART_FINE_TUNING.index; i++)
      // {
      //   printf("%x ", _UART_FINE_TUNING.fifo_get_data[i]);
      // }
      // printf("\n");
      // printf("+++++++++++++++++\n");
      UnpackFlag.FINETUNING_DATA_FLAG = true;
      uint8_t offset = 1; // ƫ���� ��ͷ��δ����
      // ���յ�����ʹ���û�п�Ƭ��
      if (_UART_FINE_TUNING.fifo_get_data[0] == 0xbc)
      {
        CLASSIFY_DATA.IS_CLASSIFY = false;
        CLASSIFY_DATA.place = nil;
        CLASSIFY_DATA.type = None;
      }
      else
      {
        CLASSIFY_DATA.IS_CLASSIFY = true;
        CLASSIFY_DATA.place = PLACE_TABLE[_UART_FINE_TUNING.fifo_get_data[0] + offset];
        /*
         *  ����WEAPONS: ǹ֧A, ��ը��B, ذ��C, ����D, ������E
         *  ����MATERIAL:���Ȱ�F, �ֵ�ͲG, �Խ���H, ��������I, ��Զ��J, ͷ��K
         *  ��ͨ����TRANSPORTATION: ������L, �Ȼ���M, װ�׳�N, Ħ�г�O
         */
        if (CLASSIFY_DATA.place == A || CLASSIFY_DATA.place == B || CLASSIFY_DATA.place == C || CLASSIFY_DATA.place == D || CLASSIFY_DATA.place == E)
        {
          CLASSIFY_DATA.type = WEAPONS;
        }
        else if (CLASSIFY_DATA.place == F || CLASSIFY_DATA.place == G || CLASSIFY_DATA.place == H || CLASSIFY_DATA.place == I || CLASSIFY_DATA.place == J || CLASSIFY_DATA.place == K)
        {
          CLASSIFY_DATA.type = MATERIAL;
        }
        else if (CLASSIFY_DATA.place == L || CLASSIFY_DATA.place == M || CLASSIFY_DATA.place == N || CLASSIFY_DATA.place == O)
        {
          CLASSIFY_DATA.type = TRANSPORTATION;
        }
        else
        {
          CLASSIFY_DATA.type = None;
        }
      }
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
    else
    {
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
  }
  // ��ȡ���ô���λ�õ�ʶ��
  else if (_UART_FINE_TUNING.get_data == 0xdf && lastData == 0xfd)
  {
    if (_UART_FINE_TUNING.index == 3)
    {
      // printf("+++++++++++++++++\n");
      // printf("index: %d\n", _UART_FINE_TUNING.index);
      // for (uint32_t i = 0; i < _UART_FINE_TUNING.index; i++)
      // {
      //   printf("%x ", _UART_FINE_TUNING.fifo_get_data[i]);
      // }
      // printf("\n");
      // printf("+++++++++++++++++\n");
      uint8_t offset = 1; // ƫ���� ��ͷ��δ����
      BIG_PLACE_DATA.IS_PLACE = true;
      BIG_PLACE_DATA.Big_Place = MAIN_TABLE[_UART_FINE_TUNING.fifo_get_data[0] + offset];
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
    else
    {
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
  }
  // ��ȡ����С��λ�õ�ʶ��
  else if (_UART_FINE_TUNING.get_data == 0xef && lastData == 0xfe)
  {
    if (_UART_FINE_TUNING.index == 3)
    {
      uint8_t offset = 1; // ƫ���� ��ͷ��δ����
      SMALL_PLACE_DATA.IS_PLACE = true;
      SMALL_PLACE_DATA.place = PLACE_TABLE[_UART_FINE_TUNING.fifo_get_data[0] + offset];
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
    else
    {
      SMALL_PLACE_DATA.IS_PLACE = false;
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
  }
  else if (_UART_FINE_TUNING.get_data == 0xfa && lastData == 0xfb)
  {
    if (_UART_FINE_TUNING.index == 4)
    {
      VOLUMEUP_DATA.AngleErr = (float)(_UART_FINE_TUNING.fifo_get_data[0] >> 7 == 0) ? (_UART_FINE_TUNING.fifo_get_data[0]) : (-(256 - _UART_FINE_TUNING.fifo_get_data[0]));
      VOLUMEUP_DATA.HeightErr = (float)(_UART_FINE_TUNING.fifo_get_data[1] >> 7 == 0) ? (_UART_FINE_TUNING.fifo_get_data[1]) : (-(256 - _UART_FINE_TUNING.fifo_get_data[1]));
      // printf("AngleErr: %f HeightErr: %f\n", VOLUMEUP_DATA.AngleErr, VOLUMEUP_DATA.HeightErr);
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
    else
    {
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
  }
  // else if (_UART_FINE_TUNING.index > 7)
  // {
  //   memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
  //   _UART_FINE_TUNING.index = 0;
  // }
  lastData = _UART_FINE_TUNING.get_data;
}