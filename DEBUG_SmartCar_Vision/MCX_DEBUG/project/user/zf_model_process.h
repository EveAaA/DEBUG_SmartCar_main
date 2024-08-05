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
* �ļ�����          zf_model_process
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
#ifndef _zf_model_process_h_
#define _zf_model_process_h_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */ 
#include "zf_common_headfile.h"

    
#define IS_SHOW_SCC8660                 (true)   // �Ƿ�������Ļ��ʾ
#define IS_UART_OUTPUT_ODRESULT         (false)  // �Ƿ��������ش������Ŀ��������Ϣ
#define UART_STARTFINETUNING_BESIDEROAD (0x04)   // ��·�߿�Ƭ΢����־λ
#define UART_STARTFINETUNING_INELEMENTS (0x05)   // ��Ԫ�ؿ�Ƭ΢����־λ
#define UART_STARTFINETUNING_PLACE      (0x10)   // ΢����ĸ���־λ
#define UART_UNLOAD_FLAG                (0x12)   // һ����ж��΢����־λ
#define UART_FINETUNING_BIGPLACE        (0x13)   // �������΢������ĸ���־λ
#define UART_WAITING                    (0xee)   // ����״̬���� 
#define UART_START_RUN                  (0xce)   // ��ʼ����״̬����
#define UART_GET_SMALLPLACE             (0x20)   // ��ʼѰ����ĸ��


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
    SET_BORDER_CENTER,
    SET_SMALLPLACE_CENTER,
    SET_BIGPLACE_CENTER,
    SET_EXPOSURE,
    SET_TWOLINES,
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


extern Mode_t currMode; // ��ǰ״̬��
extern SetMode_t setMode;
extern Press_t Key_1;
extern Press_t Key_2;
extern Center_t BorderCenter;
extern Center_t SmallPlaceCenter;
extern Center_t BigPlaceCenter;
extern Center_t UartSendDiff;
extern int16_t Brightness;
extern uint8_t LEFTLINE;
extern uint8_t RIGHTLINE;
extern gpio_struct gpio_led_red;
extern gpio_struct gpio_led_green;
extern gpio_struct gpio_led_blue;
extern gpio_struct gpio_led_white;
extern volatile bool IS_ALIVE_FLAG;


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
