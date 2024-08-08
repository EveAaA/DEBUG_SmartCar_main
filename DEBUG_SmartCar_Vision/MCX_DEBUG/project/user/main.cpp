/*********************************************************************************************************************
* MCX Vision Opensourec Library ����MCX Vision ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2024 SEEKFREE ��ɿƼ�
* 
* ���ļ��� MCX Vision ��Դ���һ����
* 
* MCX Vision ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
* 
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
* 
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
* 
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
* 
* �ļ�����          main
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          MDK 5.38a
* ����ƽ̨          MCX Vision
* ��������          https://seekfree.taobao.com/
* 
* �޸ļ�¼
* ����              ����                ��ע
* 2024-04-21        ZSY            first version
********************************************************************************************************************/
#include "zf_model_process.h"
#if defined(__cplusplus)
extern "C" // mian�ļ���C++�ļ��������Ҫ����C���Ե�ͷ�ļ�������Ҫʹ��extern "C"
{
#endif /* __cplusplus */ 
#include "zf_common_headfile.h"

Mode_t currMode = NONE;
volatile bool IS_ALIVE_FLAG = false;
    
int main(void)
{
    // ʱ�Ӻ͵��Դ���-����4��ʼ��
    zf_board_init();
    // �û�����-����5��ʼ��
    user_uart_init();
    // ��ʱ300ms
    system_delay_ms(300);
    // ʹ��C++�����޷�ʹ��printf������ʹ��zf_debug_printf��zf_user_printf���
    zf_debug_printf("debug_uart_init_finish\r\n");  // ʹ�õ��Դ���-����4��������
    zf_user_printf("user_uart_init_finish\r\n");    // ʹ���û�����-����5��������
    // LED��ʼ��
    LED_init();
    // sd����ʼ��
    sd_card_init();
    // ������ʼ��
    KEY_init();
    // ��Ļ��ʼ��
    ips200_init();
    // ����ͷ��ʼ��
    scc8660_init();
    // ������ʼ��
    PARAM_init();
    // ����sd���洢����ͷ����
    scc8660_set_brightness(Brightness);
    // ģ�ͳ�ʼ����ģ�͵������ֵ�޸ĺ����е�postProcessParams.threshold��������Χ��0 - 1��Ĭ��Ϊ0.4������ԽС��Խ����ʶ��Ŀ��
    zf_model_init();
    zf_debug_printf("init finish\r\n");
    while (1)
    {
        if(scc8660_finish)
        {
            scc8660_finish = 0;
            // ģ�����в���ʾͼ��
            zf_model_run();
        }
    }
}


/**
 * @brief ���ڽ����жϣ����״̬��ת��
 * @param data 
 * @return none
 */
void UART5_IRQHandler(uint8 data) {
    switch (data)
    {
    case UART_STARTFINETUNING_BESIDEROAD: 
        currMode = TUNING_BESIDEROAD;
        break;
    case UART_STARTFINETUNING_INELEMENTS: // �ڽ��յ���ʼ΢������� ��Ŀ����Ƿ���ڱ�־λ����Ϊfalse
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

// **************************** �������� ****************************
#if defined(__cplusplus)
}
#endif /* __cplusplus */