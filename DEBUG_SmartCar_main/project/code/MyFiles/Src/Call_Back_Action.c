/**
  ******************************************************************************
  * @file    Callback_Action.c
  * @author  庄文标
  * @brief   回调函数
  * @date    11/4/2023
  *
    @verbatim
    所有需要在中断执行的函数都放在这一文件
    @endverbatim
  * @{
**/

/* Includes ------------------------------------------------------------------*/
#include "Call_Back_Action.h"
#include "zf_common_headfile.h"
#include "UserMain.h"

/* Define\Declare ------------------------------------------------------------*/
#define Sensor_CH (PIT_CH0)        // 使用的周期中断编号 如果修改 需要同步对应修改周期中断编号与 isr.c 中的调用
#define Sensor_PRIORITY (PIT_IRQn) // 对应周期中断的中断编号

/**
 ******************************************************************************
 *  @defgroup 外部调用
 *  @brief
 *
 **/

/**@brief   定时器初始化
-- @param   无
-- @author  庄文标
-- @date    2023/11/4
**/
void TIM_Init()
{
  pit_ms_init(Sensor_CH, 5); // 初始化 PIT_CH0 为周期中断 5ms 周期
  interrupt_set_priority(Sensor_PRIORITY, 0);
  pit_ms_init(PIT_CH2, 15); // 初始化 PIT_CH2 为周期中断 15ms 周期
  pit_ms_init(PIT_CH3, 1);  // 初始化 PIT_CH3 为周期中断 1ms 周期
}

/**@brief   传感器中断函数
-- @param   无
-- @author  庄文标
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
      // 两字节转浮点数
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

/**@brief   找目标板openart串口
-- @param   无
-- @author  戴骐阳
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
      // 两字节转浮点数
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

/**@brief   微调openart串口
-- @param   无
-- @auther  戴骐阳
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
  // 微调状态
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
      // 两字节转浮点数
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
  // 识别目标板接收识别种类状态
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
      uint8_t offset = 1; // 偏移量 表头是未有量
      // 接收到这个就代表没有卡片了
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
         *  武器WEAPONS: 枪支A, 爆炸物B, 匕首C, 警棍D, 消防斧E
         *  物资MATERIAL:急救包F, 手电筒G, 对讲机H, 防弹背心I, 望远镜J, 头盔K
         *  交通工具TRANSPORTATION: 消防车L, 救护车M, 装甲车N, 摩托车O
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
  // 获取放置大类位置的识别
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
      uint8_t offset = 1; // 偏移量 表头是未有量
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
  // 获取放置小类位置的识别
  else if (_UART_FINE_TUNING.get_data == 0xef && lastData == 0xfe)
  {
    if (_UART_FINE_TUNING.index == 3)
    {
      uint8_t offset = 1; // 偏移量 表头是未有量
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