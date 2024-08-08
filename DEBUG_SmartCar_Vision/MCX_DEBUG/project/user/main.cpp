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
* 文件名称          main
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
#include "zf_model_process.h"
#if defined(__cplusplus)
extern "C" // mian文件是C++文件，如果需要包含C语言的头文件，就需要使用extern "C"
{
#endif /* __cplusplus */ 
#include "zf_common_headfile.h"

Mode_t currMode = NONE;
volatile bool IS_ALIVE_FLAG = false;
    
int main(void)
{
    // 时钟和调试串口-串口4初始化
    zf_board_init();
    // 用户串口-串口5初始化
    user_uart_init();
    // 延时300ms
    system_delay_ms(300);
    // 使用C++编译无法使用printf，可以使用zf_debug_printf和zf_user_printf替代
    zf_debug_printf("debug_uart_init_finish\r\n");  // 使用调试串口-串口4发送数据
    zf_user_printf("user_uart_init_finish\r\n");    // 使用用户串口-串口5发送数据
    // LED初始化
    LED_init();
    // sd卡初始化
    sd_card_init();
    // 按键初始化
    KEY_init();
    // 屏幕初始化
    ips200_init();
    // 摄像头初始化
    scc8660_init();
    // 参数初始化
    PARAM_init();
    // 设置sd卡存储摄像头亮度
    scc8660_set_brightness(Brightness);
    // 模型初始化，模型的输出阈值修改函数中的postProcessParams.threshold参数，范围是0 - 1，默认为0.4，参数越小，越容易识别到目标
    zf_model_init();
    zf_debug_printf("init finish\r\n");
    while (1)
    {
        if(scc8660_finish)
        {
            scc8660_finish = 0;
            // 模型运行并显示图像
            zf_model_run();
        }
    }
}


/**
 * @brief 串口接收中断，配合状态机转换
 * @param data 
 * @return none
 */
void UART5_IRQHandler(uint8 data) {
    switch (data)
    {
    case UART_STARTFINETUNING_BESIDEROAD: 
        currMode = TUNING_BESIDEROAD;
        break;
    case UART_STARTFINETUNING_INELEMENTS: // 在接收到开始微调命令后， 把目标板是否存在标志位设置为false
        IS_ALIVE_FLAG = false;
        currMode = TUNING_INELEMETS;
        break;
    case UART_STARTFINETUNING_PLACE:
        currMode = TUNING_PLACE;
        break;
    case UART_UNLOAD_FLAG:
        IS_ALIVE_FLAG = false;
        currMode = TUNING_UNLOAD;
        break;
    case UART_FINETUNING_BIGPLACE:
        IS_ALIVE_FLAG = false;
        currMode = TUNING_BIGPLACE;
        break;
    case UART_WAITING:
        currMode = NONE;
        break;
    case UART_START_RUN:
        setMode = RUN;
        break;
    case UART_GET_SMALLPLACE:
        IS_ALIVE_FLAG = false;
        currMode = FIND_SMALL_PLACE;
        break;
    case UART_FIND_BIGPLACE:
        currMode = FIND_BIG_PLACE;
        break;
    default:
        break;
    }
}

// **************************** 代码区域 ****************************
#if defined(__cplusplus)
}
#endif /* __cplusplus */