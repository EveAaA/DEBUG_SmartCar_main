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
#include "zf_model_process.h"

#include "model.h"
#include "yolo_post_processing.h"

#define MODEL_IN_W (160)
#define MODEL_IN_H (128)
#define MODEL_IN_C (3)
#define MAX_OD_BOX_CNT (12)
#define SWAPBYTE(h) ((((uint16_t)h << 8) & 0xFF00) | ((uint16_t)h >> 8))

typedef struct tagodresult_t
{
    union
    {
        int16_t xyxy[4];
        struct
        {
            int16_t x1;
            int16_t y1;
            int16_t x2;
            int16_t y2;
        };
    };
    float score;
    int label;
} odresult_t;

// 按键调参
gpio_struct gpio_key_1 = {GPIO4, 2u};
gpio_struct gpio_key_2 = {GPIO4, 3u};
// 亮灯
gpio_struct gpio_led_red = {GPIO2, 8u};
gpio_struct gpio_led_green = {GPIO2, 9u};
gpio_struct gpio_led_blue = {GPIO2, 10u};
gpio_struct gpio_led_white = {GPIO2, 11u};

Press_t Key_1 = RELEASE; // 按键初始状态都是释放状态
Press_t Key_2 = RELEASE;
int16_t Brightness = 2000;
uint8_t LEFTLINE = 150;
uint8_t RIGHTLINE = 170;
SetMode_t setMode = NONE_SET;
volatile bool BIG_PLACE_FIND = false;
// 目标板预设定中心位置
Center_t BorderCenter = {
    .x = 160,
    .y = 120,
};
// 字母板预设定中心位置
Center_t SmallPlaceCenter = {
    .x = 160,
    .y = 120,
};
// 数字板预设定中心位置
Center_t BigPlaceCenter = {
    .x = 160,
    .y = 120,
};
// 待发送的坐标误差值
Center_t UartSendDiff = {
    .x = 0,
    .y = 0,
};

Center_t UnloadCenter = {
    .x = 160,
    .y = 120,
};

tensor_dims_t inputDims;
tensor_type_t inputType;
uint8_t *inputData;
tensor_dims_t outputDims;
tensor_type_t outputType;
uint8_t *outputData;
size_t arenaSize;
uint32_t out_size;
yolo::object_detection::PostProcessParams postProcessParams;
TfLiteTensor *outputTensor[3];
float *anchors;
std::vector<yolo::object_detection::DetectionResult> results;

int s_odretcnt = 0;
__attribute__((section(".model_input_buffer"))) volatile uint8_t __attribute((aligned(8))) model_input_buf[MODEL_IN_W * MODEL_IN_H * MODEL_IN_C] = {0};
odresult_t s_odrets[MAX_OD_BOX_CNT];

void LED_init(void)
{
    gpio_init(gpio_led_red, GPO, 1, PULL_UP);
    gpio_init(gpio_led_green, GPO, 1, PULL_UP);
    gpio_init(gpio_led_blue, GPO, 1, PULL_UP);
    gpio_init(gpio_led_white, GPO, 1, PULL_UP);
}

void KEY_init(void)
{
    gpio_init(gpio_key_1, GPI, 0, PULL_UP);
    gpio_init(gpio_key_2, GPI, 0, PULL_UP);
}

void draw_rect_on_slice_buffer(uint16_t *pcam, int srcw, int cury, int stride, odresult_t *podret, int retcnt, int slice_height)
{
    int i = 0;
    for (i = 0; i < retcnt; i++, podret++)
    {
        uint32_t color = 0xffffffff;
        uint16_t *phorline = 0;
        int stripey1 = podret->y1 - cury;
        if (stripey1 >= 0 && stripey1 < slice_height)
        {
            for (int j = 0; j < 2 && stripey1 + j < slice_height; j++)
            {
                phorline = pcam + srcw * (j + stripey1) + podret->x1;
                memset(phorline, color, (podret->x2 - podret->x1) * 2);
            }
        }

        int stripey2 = podret->y2 - cury;
        if (stripey2 >= 0 && stripey2 < slice_height)
        {
            for (int j = 0; j < 2 && stripey2 + j < slice_height; j++)
            {
                phorline = pcam + srcw * (j + stripey2) + podret->x1;
                memset(phorline, color, (podret->x2 - podret->x1) * 2);
            }
        }

        uint16_t *pvtclinel = pcam + podret->x1;
        uint16_t *pvtcliner = pcam + podret->x2 - 2;

        for (int y = cury; y < cury + slice_height; y++)
        {
            if (y > podret->y1 && y < podret->y2)
            {
                memset(pvtclinel, color, 4);
                memset(pvtcliner, color, 4);
            }
            pvtclinel += srcw;
            pvtcliner += srcw;
        }
    }
}

void rgb565stridedtorgb888(const uint16_t *pin, int srcw, int wndw, int wndh, int wndx0, int wndy0,
                           uint8_t *p888, int stride, uint8_t issub128)
{
    const uint16_t *psrc = pin;
    uint32_t datin, datout, datouts[3];
    uint8_t *p888out = p888;

    for (int y = wndy0, y1 = (wndh - wndy0) / stride - wndy0; y < wndh; y += stride, y1--)
    {
        psrc = pin + srcw * y + wndx0;
        for (int x = 0; x < wndw; x += stride * 4)
        {
            datin = SWAPBYTE(psrc[0]);
            psrc += stride;
            datouts[0] = (datin & 31) << 19 | (datin & 63 << 5) << 5 | ((datin >> 8) & 0xf8);

            datin = SWAPBYTE(psrc[0]);
            psrc += stride;
            datout = (datin & 31) << 19 | (datin & 63 << 5) << 5 | ((datin >> 8) & 0xf8);
            datouts[0] |= datout << 24;
            datouts[1] = datout >> 8;

            datin = SWAPBYTE(psrc[0]);
            psrc += stride;
            datout = (datin & 31) << 19 | (datin & 63 << 5) << 5 | ((datin >> 8) & 0xf8);
            datouts[1] |= (datout << 16) & 0xffff0000;
            datouts[2] = datout >> 16;

            datin = SWAPBYTE(psrc[0]);
            psrc += stride;
            datout = (datin & 31) << 19 | (datin & 63 << 5) << 5 | ((datin >> 8) & 0xf8);

            datouts[2] |= datout << 8;

            if (issub128)
            {
                datouts[0] ^= 0x80808080;
                datouts[1] ^= 0x80808080;
                datouts[2] ^= 0x80808080;
            }
            memcpy(p888out, datouts, 3 * 4);
            p888out += 3 * 4;
        }
    }
}

void ezh_copy_slice_to_model_input(uint32_t idx, uint32_t cam_slice_buffer, uint32_t cam_slice_width, uint32_t cam_slice_height, uint32_t max_idx)
{
    static uint8_t *pCurDat;
    uint32_t curY;
    uint32_t s_imgStride = cam_slice_width / MODEL_IN_W;

    if (idx > max_idx)
    {
        return;
    }
    uint32_t ndx = idx;
    curY = ndx * cam_slice_height;
    int wndY = (s_imgStride - (curY - 0) % s_imgStride) % s_imgStride;

    pCurDat = (uint8 *)model_input_buf + 3 * ((cam_slice_height * ndx + wndY) * cam_slice_width / s_imgStride / s_imgStride);

    if (curY + cam_slice_height >= 0)
    {
        rgb565stridedtorgb888((uint16_t *)cam_slice_buffer, cam_slice_width, cam_slice_width, cam_slice_height, 0, wndY, pCurDat, s_imgStride, 1);
    }
}
typedef struct
{
    uint16 res_x1;
    uint16 res_y1;
    uint16 res_x2;
    uint16 res_y2;
} od_result_t;
volatile od_result_t od_result;
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     模型的目标信息输出
// 参数说明     odres           目标结构体
// 参数说明     retcnt          目标数量
// 返回参数     void
// 使用示例     zf_model_odresult_out(s_odrets, s_odretcnt);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void zf_model_odresult_out(const odresult_t *odres, int retcnt)
{
    for (int i = 0; i < retcnt; i++, odres++)
    {
        zf_debug_printf("sc:%d,x1:%d,y1:%d,x2:%d,y2:%d\r\n",
                        (int)(odres->score * 100), odres->x1, odres->y1, odres->x2, odres->y2);

        // 通过用户串口-串口5发送数据，通过其他单片机接收
        od_result.res_x1 = odres->x1;
        od_result.res_y1 = odres->y1;
        od_result.res_x2 = odres->x2;
        od_result.res_y2 = odres->y2;
        user_uart_putchar(0xAA);
        user_uart_putchar((uint8)i);
        user_uart_write_buffer((const uint8 *)(&od_result), sizeof(od_result));
        user_uart_putchar(0xFF);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     模型初始化
// 返回参数     void
// 使用示例     zf_model_init();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void zf_model_init(void)
{
    if (MODEL_Init() != kStatus_Success)
    {
        zf_debug_printf("Failed initializing model");
        while (1)
            ;
    }
    size_t usedSize = MODEL_GetArenaUsedBytes(&arenaSize);
    zf_debug_printf("\r\n%d/%d kB (%0.2f%%) tensor arena used\r\n", usedSize / 1024, arenaSize / 1024, 100.0 * usedSize / arenaSize);

    inputData = MODEL_GetInputTensorData(&inputDims, &inputType);
    outputData = MODEL_GetOutputTensorData(&outputDims, &outputType);
    out_size = MODEL_GetOutputSize();

    postProcessParams.inputImgRows = (int)inputDims.data[1];
    postProcessParams.inputImgCols = (int)inputDims.data[2];
    postProcessParams.output_size = (int)out_size;
    postProcessParams.originalImageWidth = SCC8660_W;
    postProcessParams.originalImageHeight = SCC8660_H;
    postProcessParams.threshold = 0.5;
    postProcessParams.nms = 0.2;
    postProcessParams.numClasses = 1;
    postProcessParams.topN = 0;

    anchors = MODEL_GetAnchors();
    for (int i = 0; i < out_size; i++)
    {
        outputTensor[i] = MODEL_GetOutputTensor(i);
        postProcessParams.anchors[i][0] = *(anchors + 6 * (out_size - 1 - i));
        postProcessParams.anchors[i][1] = *(anchors + 6 * (out_size - 1 - i) + 1);
        postProcessParams.anchors[i][2] = *(anchors + 6 * (out_size - 1 - i) + 2);
        postProcessParams.anchors[i][3] = *(anchors + 6 * (out_size - 1 - i) + 3);
        postProcessParams.anchors[i][4] = *(anchors + 6 * (out_size - 1 - i) + 4);
        postProcessParams.anchors[i][5] = *(anchors + 6 * (out_size - 1 - i) + 5);
    }
}

void KEY_SCAN(Press_t *Key1, Press_t *Key2)
{
    static gpio_level_enum LastKey1State = GPIO_HIGH;
    static gpio_level_enum LastKey2State = GPIO_HIGH;
    static uint16_t pressTimeKey1 = 0;
    static uint16_t pressTimeKey2 = 0;
    gpio_level_enum Key1State = (gpio_level_enum)gpio_get_level(gpio_key_1);
    gpio_level_enum Key2State = (gpio_level_enum)gpio_get_level(gpio_key_2);
    if (!Key1State)
    {
        system_delay_ms(10);
        if (!Key1State && LastKey1State == GPIO_LOW)
        {
            pressTimeKey1 += 1;
        }
    }
    else if (Key1State)
    {
        system_delay_ms(10);
        if (Key1State)
        {
            if (pressTimeKey1 < 10 && pressTimeKey1 > 0)
            {
                *Key1 = PRESS;
            }
            else if (pressTimeKey1 >= 10)
            {
                *Key1 = LONG_PRESS;
            }
            else
            {
                *Key1 = RELEASE;
            }
            pressTimeKey1 = 0;
        }
    }
    if (!Key2State)
    {
        system_delay_ms(10);
        if (!Key2State && LastKey2State == GPIO_LOW)
        {
            pressTimeKey2 += 1;
        }
    }
    else if (Key2State)
    {
        system_delay_ms(10);
        if (Key2State)
        {
            if (pressTimeKey2 < 10 && pressTimeKey2 > 0)
            {
                *Key2 = PRESS;
            }
            else if (pressTimeKey2 >= 10)
            {
                *Key2 = LONG_PRESS;
            }
            else
            {
                *Key2 = RELEASE;
            }
            pressTimeKey2 = 0;
        }
    }
    LastKey1State = Key1State;
    LastKey2State = Key2State;
}

void MENU_SHOW(SetMode_t *Mode, uint16_t ArrayPlace, bool CxCyFlag)
{
    switch (*Mode)
    {
    case NONE_SET:
        ips200_show_string(0, ArrayPlace, ">");
        ips200_show_string(10, 10, "SET_BORDER_CENTER");
        ips200_show_string(10, 25, "SET_SMALLPLACE_CENTER");
        ips200_show_string(10, 40, "SET_BIGPLACE_CENTER");
        ips200_show_string(10, 55, "SET_EXPOSURE");
        ips200_show_string(10, 70, "RUN");
        ips200_show_string(10, 85, "SET_TWOLINES");
        ips200_show_string(10, 100, "SET_UNLOAD_CENTER");
        break;
    case SET_EXPOSURE:
        ips200_show_string(10, 10, "BRIGHTNESS:");
        ips200_show_int(10, 25, Brightness);
        break;
    case SET_BORDER_CENTER:
        ips200_show_string(10, 10, "cx:");
        ips200_show_int(30, 10, BorderCenter.x);
        ips200_show_string(10, 25, "cy:");
        ips200_show_int(30, 25, BorderCenter.y);
        if (!CxCyFlag)
        {
            ips200_show_string(10, 50, "change cx");
        }
        else
        {
            ips200_show_string(10, 50, "change cy");
        }
        ips200_show_string(10, 65, "Border");
        ips200_draw_line(0, BorderCenter.y, 320, BorderCenter.y, RGB565_RED);
        ips200_draw_line(BorderCenter.x, 0, BorderCenter.x, 240, RGB565_GREEN);
        break;
    case SET_SMALLPLACE_CENTER:
        ips200_show_string(10, 10, "cx:");
        ips200_show_int(30, 10, SmallPlaceCenter.x);
        ips200_show_string(10, 25, "cy:");
        ips200_show_int(30, 25, SmallPlaceCenter.y);
        if (!CxCyFlag)
        {
            ips200_show_string(10, 50, "change cx");
        }
        else
        {
            ips200_show_string(10, 50, "change cy");
        }
        ips200_show_string(10, 65, "SmallPlace");
        ips200_draw_line(0, SmallPlaceCenter.y, 320, SmallPlaceCenter.y, RGB565_RED);
        ips200_draw_line(SmallPlaceCenter.x, 0, SmallPlaceCenter.x, 240, RGB565_GREEN);
        break;
    case SET_BIGPLACE_CENTER:
        ips200_show_string(10, 10, "cx:");
        ips200_show_int(30, 10, BigPlaceCenter.x);
        ips200_show_string(10, 25, "cy:");
        ips200_show_int(30, 25, BigPlaceCenter.y);
        if (!CxCyFlag)
        {
            ips200_show_string(10, 50, "change cx");
        }
        else
        {
            ips200_show_string(10, 50, "change cy");
        }
        ips200_show_string(10, 65, "BigPlace");
        ips200_draw_line(0, BigPlaceCenter.y, 320, BigPlaceCenter.y, RGB565_RED);
        ips200_draw_line(BigPlaceCenter.x, 0, BigPlaceCenter.x, 240, RGB565_GREEN);
        break;
    case SET_TWOLINES:
        ips200_show_string(10, 10, "LEFTLINE:");
        ips200_show_int(120, 10, LEFTLINE);
        ips200_show_string(10, 25, "RIGHTLINE:");
        ips200_show_int(120, 25, RIGHTLINE);
        if (!CxCyFlag)
        {
            ips200_show_string(10, 50, "change LEFT RED");
        }
        else
        {
            ips200_show_string(10, 50, "change RIGHT GREEN");
        }
        ips200_show_string(10, 65, "LINES");
        ips200_draw_line(LEFTLINE, 0, LEFTLINE, 240, RGB565_RED);
        ips200_draw_line(RIGHTLINE, 0, RIGHTLINE, 240, RGB565_GREEN);
        break;
    case SET_UNLOAD_CENTER:
        ips200_show_string(10, 10, "cx:");
        ips200_show_int(30, 10, UnloadCenter.x);
        ips200_show_string(10, 25, "cy:");
        ips200_show_int(30, 25, UnloadCenter.y);
        if (!CxCyFlag)
        {
            ips200_show_string(10, 50, "change cx");
        }
        else
        {
            ips200_show_string(10, 50, "change cy");
        }
        ips200_show_string(10, 65, "Unload");
        ips200_draw_line(0, UnloadCenter.y, 320, UnloadCenter.y, RGB565_RED);
        ips200_draw_line(UnloadCenter.x, 0, UnloadCenter.x, 240, RGB565_GREEN);
        break;
    default:
        break;
    }
}

SetMode_t GetSelectMode(int8_t ArrayPlace)
{
    switch (ArrayPlace)
    {
    case 10:
        return SET_BORDER_CENTER;
    case 25:
        return SET_SMALLPLACE_CENTER;
    case 40:
        return SET_BIGPLACE_CENTER;
    case 55:
        return SET_EXPOSURE;
    case 70:
        return RUN;
    case 85:
        return SET_TWOLINES; 
    case 100:
        return SET_UNLOAD_CENTER;
    default:
        return NONE_SET;
    }
}

void UPDATE_SETMODE(SetMode_t *Mode)
{
    KEY_SCAN(&Key_1, &Key_2); // 按键扫描状态
    static int8_t ArrayPlace = 10;
    static bool changeCxCyFlag = false;
    switch (*Mode)
    {
    case NONE_SET:
        if (Key_1 == PRESS)
        {
            ArrayPlace -= 15;
            if (ArrayPlace < 10)
            {
                ArrayPlace = 100;
            }
        }
        else if (Key_2 == PRESS)
        {
            ArrayPlace += 15;
            if (ArrayPlace > 100)
            {
                ArrayPlace = 10;
            }
        }
        if (Key_2 == LONG_PRESS)
        {
            *Mode = GetSelectMode(ArrayPlace);
        }
        break;
    case SET_EXPOSURE:
        if (Key_1 == PRESS)
        {
            Brightness -= 50;
            if (Brightness < 0)
            {
                Brightness = 5000;
            }
            scc8660_set_brightness(Brightness);
        }
        else if (Key_2 == PRESS)
        {
            Brightness += 50;
            if (Brightness > 5000)
            {
                Brightness = 50;
            }
            scc8660_set_brightness(Brightness);
        }
        if (Key_2 == LONG_PRESS)
        {
            *Mode = NONE_SET;
            ArrayPlace = 10;
            sd_write_data(Brightness, ADDRESS(0));
            // 将亮度写入SD卡未实现
        }
        break;
    case SET_BORDER_CENTER:
        if (Key_1 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                BorderCenter.x -= 1;
            }
            else
            {
                BorderCenter.y -= 1;
            }
        }
        else if (Key_2 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                BorderCenter.x += 1;
            }
            else
            {
                BorderCenter.y += 1;
            }
        }
        if (Key_2 == LONG_PRESS)
        {
            *Mode = NONE_SET;
            ArrayPlace = 10;
            // 将cx,cy写入SD卡未实现
            sd_write_data(BorderCenter.x, ADDRESS(1));
            sd_write_data(BorderCenter.y, ADDRESS(2));
        }
        else if (Key_1 == LONG_PRESS)
        {
            changeCxCyFlag = !changeCxCyFlag;
        }
        break;
    case SET_SMALLPLACE_CENTER:
        if (Key_1 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                SmallPlaceCenter.x -= 5;
            }
            else
            {
                SmallPlaceCenter.y -= 5;
            }
        }
        else if (Key_2 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                SmallPlaceCenter.x += 5;
            }
            else
            {
                SmallPlaceCenter.y += 5;
            }
        }
        if (Key_2 == LONG_PRESS)
        {
            *Mode = NONE_SET;
            ArrayPlace = 10;
            // 将cx,cy写入SD卡未实现
            sd_write_data(SmallPlaceCenter.x, ADDRESS(3));
            sd_write_data(SmallPlaceCenter.y, ADDRESS(4));
        }
        else if (Key_1 == LONG_PRESS)
        {
            changeCxCyFlag = !changeCxCyFlag;
        }
        break;
    case SET_BIGPLACE_CENTER:
        if (Key_1 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                BigPlaceCenter.x -= 5;
            }
            else
            {
                BigPlaceCenter.y -= 5;
            }
        }
        else if (Key_2 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                BigPlaceCenter.x += 5;
            }
            else
            {
                BigPlaceCenter.y += 5;
            }
        }
        if (Key_2 == LONG_PRESS)
        {
            *Mode = NONE_SET;
            ArrayPlace = 10;
            // 将cx,cy写入SD卡未实现
            sd_write_data(BigPlaceCenter.x, ADDRESS(5));
            sd_write_data(BigPlaceCenter.y, ADDRESS(6));
        }
        else if (Key_1 == LONG_PRESS)
        {
            changeCxCyFlag = !changeCxCyFlag;
        }
        break;
    case SET_TWOLINES:
        if (Key_1 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                LEFTLINE -= 5;
            }
            else
            {
                RIGHTLINE -= 5;
            }
        }
        else if (Key_2 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                LEFTLINE += 5;
            }
            else
            {
                RIGHTLINE += 5;
            }
        }
        if (Key_2 == LONG_PRESS)
        {
            *Mode = NONE_SET;
            ArrayPlace = 10;
            sd_write_data(LEFTLINE, ADDRESS(7));
            sd_write_data(RIGHTLINE, ADDRESS(8));
        }
        else if (Key_1 == LONG_PRESS)
        {
            changeCxCyFlag = !changeCxCyFlag;
        }
        break;
    case SET_UNLOAD_CENTER:
        if (Key_1 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                UnloadCenter.x -= 5;
            }
            else
            {
                UnloadCenter.y -= 5;
            }
        }
        else if (Key_2 == PRESS)
        {
            if (!changeCxCyFlag)
            {
                UnloadCenter.x += 5;
            }
            else
            {
                UnloadCenter.y += 5;
            }
        }
        if (Key_2 == LONG_PRESS)
        {
            *Mode = NONE_SET;
            ArrayPlace = 10;
            // 将cx,cy写入SD卡未实现
            sd_write_data(UnloadCenter.x, ADDRESS(9));
            sd_write_data(UnloadCenter.y, ADDRESS(10));
        }
        else if (Key_1 == LONG_PRESS)
        {
            changeCxCyFlag = !changeCxCyFlag;
        }
        break;
    }
    MENU_SHOW(Mode, ArrayPlace, changeCxCyFlag);
}

void getSendDiff(Mode_t *Mode, int16_t cx, int16_t cy, Center_t *UartSendDiff)
{
    switch (*Mode)
    {
    case TUNING_BESIDEROAD:
        UartSendDiff->x = cx - BorderCenter.x;
        UartSendDiff->y = BorderCenter.y - cy;
        break;
    case TUNING_INELEMETS:
        UartSendDiff->x = cx - BorderCenter.x;
        UartSendDiff->y = BorderCenter.y - cy;
        break;
    case TUNING_PLACE:
        UartSendDiff->x = cx - SmallPlaceCenter.x;
        UartSendDiff->y = SmallPlaceCenter.y - cy;
        break;
    case TUNING_BIGPLACE:
        UartSendDiff->x = cx - BigPlaceCenter.x;
        UartSendDiff->y = BigPlaceCenter.y - cy;
        break;
    case TUNING_UNLOAD:
        UartSendDiff->x = cx - UnloadCenter.x;
        UartSendDiff->y = UnloadCenter.y - cy;
        break;
    default:
        break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     模型运行
// 返回参数     void
// 使用示例     zf_model_run();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void zf_model_run(void)
{
    static uint8_t isBorderNotAliveTime = 0; // 卡片不存在的帧数
    // 数据通讯格式 X坐标(低八位, 高八位) Y坐标(低八位，高八位) 0x00, 校验帧尾(0xFC 0xBF)
    uint8 buffer[7] = {0};
    buffer[4] = (uint8)0x00; // 暂未使用
    buffer[5] = (uint8)0xFC;
    buffer[6] = (uint8)0xBF;
    // ********************************************************************************
    uint8_t *buf = 0;
    memset(inputData, 0, inputDims.data[1] * inputDims.data[2] * inputDims.data[3]);
    buf = inputData + (inputDims.data[1] - MODEL_IN_H) / 2 * MODEL_IN_W * MODEL_IN_C;
    memcpy(buf, (uint8 *)model_input_buf, MODEL_IN_W * MODEL_IN_H * MODEL_IN_C);

    results.clear();
    if (setMode != RUN)
    {
        UPDATE_SETMODE(&setMode);
        ips200_show_scc8660(scc8660_image);
    }
    else
    {

        MODEL_RunInference();

        s_odretcnt = 0;
        if (!(yolo::DetectorPostProcess((const TfLiteTensor **)outputTensor, results, postProcessParams).DoPostProcess()))
        {
            s_odretcnt = 0;
        }
        for (const auto &result : results)
        {
            if (result.m_normalisedVal > postProcessParams.threshold)
            {
                s_odrets[s_odretcnt].x1 = result.m_x0;
                s_odrets[s_odretcnt].x2 = result.m_x0 + result.m_w;
                s_odrets[s_odretcnt].y1 = result.m_y0;
                s_odrets[s_odretcnt].y2 = result.m_y0 + result.m_h;
                s_odrets[s_odretcnt].score = result.m_normalisedVal;
                // zf_D_printf("scores = %f \r\n", s_odrets[s_odretcnt].score);
                //  获取图像中心
                int16_t cx = (s_odrets[s_odretcnt].x1 + s_odrets[s_odretcnt].x2) / 2;
                int16_t cy = (s_odrets[s_odretcnt].y1 + s_odrets[s_odretcnt].y2) / 2;
                // 获取待发送图像误差
                getSendDiff(&currMode, cx, cy, &UartSendDiff);
                ips200_show_string(10, 25, "dx");
                ips200_show_float(10, 40, cx);
                ips200_show_float(10, 55, s_odrets[s_odretcnt].score);
                uint8_t dx_high_8bit = UartSendDiff.x >> 8;
                uint8_t dx_low_8bit = UartSendDiff.x & 0xff;
                uint8_t dy_high_8bit = UartSendDiff.y >> 8;
                uint8_t dy_low_8bit = UartSendDiff.y & 0xff;
                buffer[0] = dx_low_8bit;
                buffer[1] = dx_high_8bit;
                buffer[2] = dy_low_8bit;
                buffer[3] = dy_high_8bit;
                s_odretcnt++;
                ///////////////////////元素内目标板//////////////////////////////////
                if (currMode == TUNING_INELEMETS)
                {
                    LED_BLUE(LED_ON);
                    IS_ALIVE_FLAG = true;
                    isBorderNotAliveTime = 0;
                    user_uart_write_buffer(buffer, sizeof(buffer));
                }
                //////////////////////粗定位字母板，数字板///////////////////////////
                else if (currMode == FIND_SMALL_PLACE || currMode == FIND_BIG_PLACE)
                {
                    if (cx <= RIGHTLINE && cx >= LEFTLINE && cy <= 120)
                    {
                        LED_RED(LED_OFF);
                        uint8_t UartBuffer[5] = {0x01, 0x00, 0x00, 0xfe, 0x7e};
                        LED_GREEN(LED_ON);
                        user_uart_write_buffer(UartBuffer, sizeof(UartBuffer));
                        system_delay_ms(10);
                        user_uart_write_buffer(UartBuffer, sizeof(UartBuffer));
                        system_delay_ms(10);
                        user_uart_write_buffer(UartBuffer, sizeof(UartBuffer));
                        LED_GREEN(LED_OFF);
                        if (currMode == FIND_BIG_PLACE)
                        {
                            BIG_PLACE_FIND = true;
                        }
                    }
                    else
                    {
                        LED_RED(LED_ON);
                        if (currMode == FIND_BIG_PLACE)
                        {
                            BIG_PLACE_FIND = false;
                        }
                        uint8_t UartBuffer[5] = {0x00, 0x00, 0x00, 0xfe, 0x7e};
                        user_uart_write_buffer(UartBuffer, sizeof(UartBuffer));
                    }
                }
                /////////////////////微调字母板////////////////////////////////////////////
                else if (currMode == TUNING_PLACE)
                {
                    user_uart_write_buffer(buffer, sizeof(buffer));
                }
                ////////////////////微调路边目标板/////////////////////////////////////////
                else if (currMode == TUNING_BESIDEROAD)
                {
                    user_uart_write_buffer(buffer, sizeof(buffer));
                }
                ////////////////////微调数字板//////////////////////////////////////////////
                else if(currMode == TUNING_BIGPLACE || currMode == TUNING_UNLOAD)
                {
                    if (BIG_PLACE_FIND == true)
                    {
                        LED_BLUE(LED_ON);
                        user_uart_write_buffer(buffer, sizeof(buffer));
                        LED_BLUE(LED_OFF);
                    }
                }
                /////////////////////微调一次性放置///////////////////////////////////////////
                else if(currMode == TUNING_UNLOAD)
                {
                    user_uart_write_buffer(buffer, sizeof(buffer));
                }
                break;
            }
        }
        ////////////////////////////////处在元素内卡片判断并且判断为没有卡片//////////////////////
        if ((currMode == TUNING_INELEMETS || currMode == TUNING_BIGPLACE || currMode == TUNING_UNLOAD)
             && !IS_ALIVE_FLAG && isBorderNotAliveTime > 15)
        {
            int16_t cx = -999;
            int16_t cy = -999;
            uint8_t dx_high_8bit = cx >> 8;
            uint8_t dx_low_8bit = cx & 0xff;
            uint8_t dy_high_8bit = cy >> 8;
            uint8_t dy_low_8bit = cy & 0xff;
            buffer[0] = dx_low_8bit;
            buffer[1] = dx_high_8bit;
            buffer[2] = dy_low_8bit;
            buffer[3] = dy_high_8bit;
            user_uart_write_buffer(buffer, sizeof(buffer));
            isBorderNotAliveTime = 0;
            LED_RED(LED_OFF);
        }
        ////////////////////////////////处在元素内卡片判断且正在判断卡片是否有无///////////////////
        else if (currMode == TUNING_INELEMETS && !IS_ALIVE_FLAG && isBorderNotAliveTime <= 15) 
        {
            LED_RED(LED_ON);
            isBorderNotAliveTime += 1;
        }
        /////////////////////////////////第一次寻找数字版////////////////////////////////////////////
        else if(currMode == FIND_BIG_PLACE && !IS_ALIVE_FLAG && isBorderNotAliveTime <= 15)
        {
            LED_RED(LED_ON);
            isBorderNotAliveTime += 1;
        }
        ////////////////////////////////微调判断是否有数字板存在/////////////////////////////////
        else if(currMode == TUNING_BIGPLACE && !IS_ALIVE_FLAG && isBorderNotAliveTime <= 15)
        {
            LED_RED(LED_ON);
            isBorderNotAliveTime += 1;
        }
        else if(currMode == TUNING_UNLOAD && !IS_ALIVE_FLAG && isBorderNotAliveTime <= 15)
        {
            LED_RED(LED_ON);
            isBorderNotAliveTime += 1;
        }
        // *****
#if IS_UART_OUTPUT_ODRESULT
        if (s_odretcnt > 0)
        {
            zf_model_odresult_out(s_odrets, s_odretcnt);
        }
#endif
#if IS_SHOW_SCC8660 
        if (s_odretcnt)
        {
            ips200_show_string(10, 10, "OBJ");
            draw_rect_on_slice_buffer((scc8660_image), SCC8660_W, 0, 1, s_odrets, s_odretcnt, SCC8660_H);
        }
        else
        {
            ips200_show_string(10, 10, "NO OBJ");
        }
        ips200_show_scc8660(scc8660_image);
#endif
    }
}
