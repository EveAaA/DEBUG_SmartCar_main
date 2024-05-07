# Untitled - By: G15 - 周日 5月 5 2024

import pyb
import sensor, image, time, math
import os, tf
import gc
from machine import UART
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

clock = time.clock()
classifyModel = "Debug_Vision.tflite"           # 图像分类模型
SmallPlaceModel = "Debug_VisionPlace.tflite"    # 小类放置区域模型
BigPlaceModel = "QVGAnumber.tflite"             # 大类放置区域模型
findBorderModel = "fineTuningV2.tflite"         # 找目标板目标检测模型
sharpen_kernel = (0, -1, 0, -1, 5, -1, 0, -1, 0)
SmallPlaceLabels = [line.rstrip("\n") for line in open("/sd/label.txt")] + ["none"]
BigPlaceLabels = [line.rstrip("\n") for line in open("/sd/number.txt")] + ["none"]
net = tf.load(findBorderModel)
uart = UART(2, 9600)
#########使用不同类型模型分类图像的串口标志位#######
UART_NOCARD_FLAG = b'\xbc'

UART_CLASSIFY_PIC = b'\x06'
UART_CLASSIFY_SMALLPLACE = b'\x07'
UART_CLASSIFY_BIGPLACE = b'\x08'

UART_SEND_SMALLPLACE = b'\xfe\xef'  # 发送小类帧尾
UART_SEND_BIGPLACE = b'\xfd\xdf'    # 发送大类帧尾
UART_SEND_CLASSIFY = b'\xfb\xcf'    # 发送目标板种类帧尾

def getImage():
    minRect = []
    noObject = 0
    while True:
        img = sensor.snapshot()
        img1 = img.copy(x_scale = 0.75)
        object = tf.detect(net, img1)
        if object:
            x1,y1,x2,y2,label,scores = object[0]
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height() + 5)
            w = int(w*img.width())
            h = int(h*img.height())
            minRect = [[x1, y1], [x1 + w, y1], [x1 + w, y1 + h], [x1, y1 + h]]
            img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
        else:
            continue
        # print(scores)
        if scores > 0.90 and abs(w-h) < 10:
            return [img, minRect]
        else:
             noObject += 1
        if noObject > 15:
            return [None, None]

# 分类图像
def classify(img, minRect):
    img.rotation_corr(corners=minRect[0:4])
    model = tf.load(classifyModel, load_to_fb=True)
    obj = tf.classify(model, img)[0]  # 输入图像为img1则Debug_Vision.tflite分类模型, 输入图像为inputImage 则为Debug_VisionClassify的模型
    tf.free_from_fb()
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    if m >= 0.85:
        result_index = res.index(m)
        uart.write(result_index.to_bytes(1, "little") + UART_SEND_CLASSIFY)
        print("%s: %f" % (SmallPlaceLabels[result_index], m))
    else:
        print("Unknow!")
    # img.draw_image(inputImage, 0, 0)

# 分类小类目标板
def classifySmallPlace(img, minRect):
    img.rotation_corr(corners=minRect[0:4])
    inputImage = sensor.alloc_extra_fb(96, 96, sensor.GRAYSCALE)
    inputImage.draw_image(img, 0, 0,x_scale=0.3, y_scale=0.4)
    inputImage.invert()
    inputImage.morph(1, sharpen_kernel, 1)
    his = inputImage.get_histogram()
    inputImage.binary([[his.get_threshold().value(), 255]], invert = True) # 突出图像特征
    inputImage.erode(1)
    model = tf.load(SmallPlaceModel, load_to_fb=True)
    obj = tf.classify(model, inputImage)[0]
    tf.free_from_fb()
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    print("%s: %f" % (SmallPlaceLabels[result_index], m))
    # print(res)
    if m >= 0.95:
        result_index = res.index(m)
        uart.write(result_index.to_bytes(1, "little") + UART_SEND_SMALLPLACE)
        print("%s: %f" % (SmallPlaceLabels[result_index], m))
    else:
        print("Unknow!")
    # img.draw_image(inputImage, 0, 0)
    sensor.dealloc_extra_fb()

# 分类大类目标板
def classifyBigPlace(img, minRect):
    img.rotation_corr(corners=minRect[0:4])
    model = tf.load(BigPlaceModel, load_to_fb=True)
    obj = tf.classify(model, img)[0]
    tf.free_from_fb()
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    print("%s: %f" % (BigPlaceLabels[result_index], m))
    # print(res)
    if m >= 0.95:
        result_index = res.index(m)
        uart.write(result_index.to_bytes(1, "little") + UART_SEND_BIGPLACE)
        print("%s: %f" % (BigPlaceLabels[result_index], m))
    else:
        print("Unknow!")
    # img.draw_image(inputImage, 0, 0)
    sensor.dealloc_extra_fb()

test = False

while(True):
    if test == False:
        while True:
            clock.tick()
            img, minRect = getImage()
            if img and minRect:
                classify(img, minRect)
            else:
                # 没有卡片的时候
                uart.write(UART_NOCARD_FLAG + UART_SEND_CLASSIFY)
            print(clock.avg())
    else:
        while True:
            uart_num = uart.any()
            if uart_num:
                uart_str = uart.read()
                print(uart_str)
                if uart_str == UART_CLASSIFY_PIC:
                    img, minRect = getImage()
                    if img and minRect:
                        classify(img, minRect)
                    else:
                        # 没有卡片的时候
                        uart.write(UART_NOCARD_FLAG + UART_SEND_CLASSIFY)
                elif uart_str == UART_CLASSIFY_SMALLPLACE:
                    img, minRect = getImage()
                    if img and minRect:
                        classifySmallPlace(img, minRect)
                elif uart_str == UART_CLASSIFY_BIGPLACE:
                    img, minRect = getImage()
                    if img and minRect:
                        classifyBigPlace(img, minRect)
                else:
                    continue

