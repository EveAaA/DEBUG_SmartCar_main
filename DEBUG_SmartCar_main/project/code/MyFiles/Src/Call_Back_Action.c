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
#define Sensor_CH                  (PIT_CH0 )// 使用的周期中断编号 如果修改 需要同步对应修改周期中断编号与 isr.c 中的调用
#define Sensor_PRIORITY            (PIT_IRQn)// 对应周期中断的中断编号 


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
    pit_ms_init(Sensor_CH, 5);                                                  // 初始化 PIT_CH0 为周期中断 5ms 周期
    interrupt_set_priority(Sensor_PRIORITY, 0); 
    pit_ms_init(PIT_CH1, 10);                                                  // 初始化 PIT_CH1 为周期中断 10ms 周期
    pit_ms_init(PIT_CH2, 15);                                                  // 初始化 PIT_CH2 为周期中断 15ms 周期
    pit_ms_init(PIT_CH3, 1);                                                  // 初始化 PIT_CH2 为周期中断 15ms 周期
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

/**@brief   找目标板openart串口
-- @param   无
-- @author  戴骐阳
-- @date    2023/12/23
**/
void Uart_Findborder_Receive(void)
{
    uart_query_byte(UART_1, &_UART_FINDBORDER.get_data);
    fifo_write_buffer(&_UART_FINDBORDER.uart_data_fifo, &_UART_FINDBORDER.get_data, 1);
    if(_UART_FINDBORDER.get_data == 0x80)
    {
       UnpackFlag.FINDBORDER_DATA_FLAG = true;
    }
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
    // printf("%x  \n", _UART_FINE_TUNING.get_data);
    // 微调状态
    if (_UART_FINE_TUNING.get_data == 0xbf && lastData == 0xfc)
    {
        /*
        printf("+++++++++++++++++\n");

        for (uint32_t i = 0; i < _UART_FINE_TUNING.index; i++)
        {
        printf("%x \n", _UART_FINE_TUNING.fifo_get_data[i]);
        }
        */
        // 两字节转浮点数
        UnpackFlag.FINETUNING_DATA_FLAG = true;
        FINETUNING_DATA.dx = (float)((_UART_FINE_TUNING.fifo_get_data[1] >> 7) == 0) ? ((_UART_FINE_TUNING.fifo_get_data[0] + (_UART_FINE_TUNING.fifo_get_data[1] << 8))) : (-(65536 - (_UART_FINE_TUNING.fifo_get_data[0] + (_UART_FINE_TUNING.fifo_get_data[1] << 8))));
        FINETUNING_DATA.dy = (float)((_UART_FINE_TUNING.fifo_get_data[3] >> 7) == 0) ? ((_UART_FINE_TUNING.fifo_get_data[2] + (_UART_FINE_TUNING.fifo_get_data[3] << 8))) : (-(65536 - (_UART_FINE_TUNING.fifo_get_data[2] + (_UART_FINE_TUNING.fifo_get_data[3] << 8))));
        FINETUNING_DATA.FINETUNING_FINISH_FLAG = _UART_FINE_TUNING.fifo_get_data[4];
        // printf("dx:%f dy:%f tuning: %d \n", FINETUNING_DATA.dx, FINETUNING_DATA.dy,  FINETUNING_DATA.FINETUNING_FINISH_FLAG);
        // printf("+++++++++++++++++\n");
        memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
        _UART_FINE_TUNING.index = 0;
    }
    // 识别目标板接收识别种类状态
    else if (_UART_FINE_TUNING.get_data == 0xcf && lastData == 0xfb)
    {
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
    // 获取放置大类位置的识别
    else if (_UART_FINE_TUNING.get_data == 0xdf && lastData == 0xfd)
    {
      uint8_t offset = 1; // 偏移量 表头是未有量
      BIG_PLACE_DATA.IS_PLACE = true;
      BIG_PLACE_DATA.place = +PLACE_TABLE[_UART_FINE_TUNING.fifo_get_data[0] + offset];
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
    // 获取放置小类位置的识别
    else if (_UART_FINE_TUNING.get_data == 0xef && lastData == 0xfe)
    {
      uint8_t offset = 1; // 偏移量 表头是未有量
      SMALL_PLACE_DATA.IS_PLACE = true;
      SMALL_PLACE_DATA.place = PLACE_TABLE[_UART_FINE_TUNING.fifo_get_data[0] + offset];
      memset(_UART_FINE_TUNING.fifo_get_data, 0, sizeof(_UART_FINE_TUNING.fifo_get_data));
      _UART_FINE_TUNING.index = 0;
    }
    lastData = _UART_FINE_TUNING.get_data;
}