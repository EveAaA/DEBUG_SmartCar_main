/*********************************************************************************************************************
* MCX Vision Opensourec Library 即（MCX Vision 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2024 SEEKFREE 逐飞科技
* 
* 本文件是 MCX Vision 开源库的一部分
* 
* MCX Vision 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
* 
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
* 
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
* 
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
* 
* 文件名称          zf_model_process
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MDK 5.38a
* 适用平台          MCX Vision
* 店铺链接          https://seekfree.taobao.com/
* 
* 修改记录
* 日期              作者                备注
* 2024-04-21        ZSY            first version
********************************************************************************************************************/
#ifndef _zf_model_process_h_
#define _zf_model_process_h_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */ 
#include "zf_common_headfile.h"

    
#define IS_SHOW_SCC8660                 (true)   // 是否启用屏幕显示
#define IS_UART_OUTPUT_ODRESULT         (false)  // 是否启用下载串口输出目标坐标信息
#define UART_STARTFINETUNING_BESIDEROAD (0x04)   // 捡路边卡片微调标志位
#define UART_STARTFINETUNING_INELEMENTS (0x05)   // 捡元素卡片微调标志位
#define UART_STARTFINETUNING_PLACE      (0x10)   // 微调字母板标志位
#define UART_UNLOAD_FLAG                (0x12)   // 一次性卸载微调标志位
#define UART_FINETUNING_BIGPLACE        (0x13)   // 逐个放置微调大字母板标志位
#define UART_WAITING                    (0xee)   // 待机状态命令 
#define UART_START_RUN                  (0xce)   // 开始运行状态命令
#define UART_FIND_BIGPLACE              (0x19)   // 开始寻找大字母板
#define UART_GET_SMALLPLACE             (0x20)   // 开始寻找字母板


typedef enum{
    WAITING_START, 
    NONE,
    FIND_SMALL_PLACE,
    FIND_BIG_PLACE,
    TUNING_BESIDEROAD,
    TUNING_INELEMETS,
    TUNING_PLACE,
    TUNING_UNLOAD,
    TUNING_BIGPLACE,
}Mode_t;

typedef enum{
    NONE_SET,
    SET_SCREEN_SHOW,
    SET_BORDER_CENTER,
    SET_SMALLPLACE_CENTER,
    SET_BIGPLACE_CENTER,
    SET_EXPOSURE,
    SET_TWOLINES,
    SET_UNLOAD_CENTER,
    RUN,
}SetMode_t;

typedef enum{
    LONG_PRESS,
    PRESS,
    RELEASE,
}Press_t;

typedef struct 
{
    int16_t x;
    int16_t y;
}Center_t;


extern Mode_t currMode; // 当前状态机
extern SetMode_t setMode;
extern Press_t Key_1;
extern Press_t Key_2;
extern Center_t BorderCenter;
extern Center_t SmallPlaceCenter;
extern Center_t BigPlaceCenter;
extern Center_t UartSendDiff;
extern Center_t UnloadCenter;
extern int16_t Brightness;
extern uint8_t LEFTLINE;
extern uint8_t RIGHTLINE;
extern gpio_struct gpio_led_red;
extern gpio_struct gpio_led_green;
extern gpio_struct gpio_led_blue;
extern gpio_struct gpio_led_white;
extern volatile bool IS_ALIVE_FLAG;
extern volatile bool SHOW_SCREEN;

void ezh_copy_slice_to_model_input(uint32_t idx, uint32_t cam_slice_buffer, uint32_t cam_slice_width, uint32_t cam_slice_height, uint32_t max_idx);
void zf_model_init(void);
void zf_model_run(void);
void KEY_init(void);
void LED_init(void);
void PARAM_init(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif
