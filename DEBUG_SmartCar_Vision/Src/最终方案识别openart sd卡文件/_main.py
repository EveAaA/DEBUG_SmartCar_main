# Untitled - By: G15 - 周日 5月 5 2024

import pyb
import sensor, image, time, math
import os, tf
import gc
from machine import UART
import openmv_numpy as np
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
#sensor.set_brightness(2000)
sensor.set_auto_exposure(True, 100)
sensor.skip_frames(time = 2000)
clock = time.clock()
classifyModel = "Debug_Vision2.tflite"           # 图像分类模型
SmallPlaceModel = "Debug_VisionPlace.tflite"    # 小类放置区域模型
BigPlaceModel = "trained.tflite"             # 大类放置区域模型
findBorderModel = "fineTuningV2.tflite"         # 找目标板目标检测模型
sharpen_kernel = (0, -1, 0, -1, 5, -1, 0, -1, 0)
SmallPlaceLabels = [line.rstrip("\n") for line in open("/sd/label.txt")] + ["none"]
BigPlaceLabels = [line.rstrip("\n") for line in open("/sd/number.txt")] + ["none"]
BigPlaceModel = tf.load(BigPlaceModel, load_to_fb=True)
SmallPlaceModel = tf.load(SmallPlaceModel, load_to_fb=True)
model = tf.load(classifyModel, load_to_fb=True)
net = tf.load(findBorderModel)
uart = UART(1, 115200)
X_FINETUNING_FLAG = False
Y_FINETUNING_FLAG = False
#########使用不同类型模型分类图像的串口标志位#######
UART_HEAD = b'\xee\xed'
UART_NOCARD_FLAG = b'\xbc'
UART_TAIl = b"\xfc\xbf"
martrixH = [[2.068966, 1.421085e-14, -4.547473e-13], [9.120551e-09, 2.142857, 2.142857], [4.795776e-10, 1.135327e-09, 1]]
UART_STARTFINETUNING = b'\x05'
UART_STARTFINETUNING_Y = b'\x07'
UART_STARTFINETUNING_PLACE = b'\x10'
UART_CLASSIFY_PIC = b'\x06'
UART_CLASSIFY_SMALLPLACE = b'\x07'
UART_CLASSIFY_BIGPLACE = b'\x08'

UART_SEND_SMALLPLACE = b'\xfe\xef'  # 发送小类帧尾
UART_SEND_BIGPLACE = b'\xfd\xdf'    # 发送大类帧尾
UART_SEND_CLASSIFY = b'\xfb\xcf'    # 发送目标板种类帧尾

def getTrueDiff(offsetX = 0, offsetY = 0):
    global object
    global Total
    global X_FINETUNING_FLAG
    availableErr = [0, 0]
    maxRect = None
    if offsetX == 0 and offsetY == 0:
        availableErr[0] = 10
        availableErr[1] = 10
    else:
        availableErr[0] = 30
        availableErr[1] = 30
    searchNum = 0
    while True:
        img = sensor.snapshot()
        if offsetX == 0 and offsetY == 0:
            img1 = img.copy(1, 1, x_scale = 0.75)
            object = tf.detect(net, img1)
            if object:
                x1,y1,x2,y2,label,scores = object[0]
                w = x2- x1
                h = y2 - y1
                x1 = int((x1)*img.width())
                y1 = int(y1*img.height() + 5)
                w = int(w*img.width())
                h = int(h*img.height())
                cx = int(x1 + w / 2)
                cy = int(y1 + h / 2)
                if offsetX == 0 and offsetY == 0:
                    if scores > 0.50 and (w * h) > 1000:
                        break
                else:
                    if scores > 0.50 and (w * h) > 1000:
                        break
            else:
                continue
        else:
            inputImage = sensor.alloc_extra_fb(160, 120, sensor.GRAYSCALE)
            inputImage.draw_image(img, 0, 0,x_scale=0.5, y_scale=0.5)
            his = inputImage.get_histogram()
            inputImage.binary([[his.get_threshold().value(), 255]], invert = False) # 突出图像特征
           #  img.draw_image(inputImage, 1, 1)

            # rect = inputImage.find_rects(threshold = 2000, roi = (0, 0, 160, 120))
            Blob = inputImage.find_blobs([(230, 255)], roi = (0, 0, 160, 70), area_threshold = 200, merge = True)
            sensor.dealloc_extra_fb()
            # print(rect)
            maxRect = find_max(Blob)
            if maxRect:
                break


    if offsetX == 0 and offsetY == 0:
        if object:
            global X_FINETUNING_FLAG
            global Y_FINETUNING_FLAG
            x1,y1,x2,y2,label,scores = object[0]
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height() + 5)
            w = int(w*img.width())
            h = int(h*img.height())
            cx = x1 + w / 2
            cy = y1 + h / 2
            center = [(cx - 159 + offsetX), (119 - cy + offsetY), 1]
            np_center = np.array([[p] for p in center])#升维
            np_H = np.array(martrixH)
            Point = np_H * np_center
            worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
            worldDz = 0
            img.draw_line(0, 119, 320, 119, color = (255, 255, 0))
            img.draw_line(159, 240, 159, 0, color = (255, 0, 0), thickness = 1)
            img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
            img.draw_cross(int(cx), int(cy), color = (0, 255, 0))
    else:
        if maxRect:
            centerPoint = [int(maxRect.rect()[0] * 2 + maxRect.rect()[2]), int(maxRect.rect()[1] * 2 + maxRect.rect()[2])]
            center = [(centerPoint[0] - 159), (119 - centerPoint[1]), 1]
            np_center = np.array([[p] for p in center])#升维
            np_H = np.array(martrixH)
            Point = np_H * np_center
            worldDx, worldDy = (int(Point[0][0]) + offsetX), (int(Point[1][0]) + offsetY)
            worldDz = 0
            img.draw_line(0, 119, 320, 119, color = (255, 255, 0))
            img.draw_line(159, 240, 159, 0, color = (255, 0, 0), thickness = 1)
            img.draw_cross(int(centerPoint[0]), int(centerPoint[1]), color = (0, 255, 0))
            img.draw_rectangle([int(maxRect.rect()[0] * 2), int(maxRect.rect()[1] * 2), int(maxRect.rect()[2] * 2), int(maxRect.rect()[3] * 2)], color = (0, 255, 0))

    print(worldDx, worldDy)
    print(X_FINETUNING_FLAG, Y_FINETUNING_FLAG)
    if abs(worldDx) > availableErr[0] and not X_FINETUNING_FLAG:
        FINE_FINISHTUNING_FLAG = b"\x00"
        _NOP_  = b'\xff'
        # print(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)
        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
        time.sleep_ms(5)
        uart.write(UART_TAIl)
    else:
        X_FINETUNING_FLAG = True
    time.sleep_ms(5)
    if abs(worldDy) > availableErr[1] and X_FINETUNING_FLAG:
        FINE_X_FLAG = b'\x02'
        print("x_finish")
        # 当X轴调整完成发送十六进制0x02
        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_X_FLAG)
        time.sleep_ms(5)
        uart.write(UART_TAIl)
    elif X_FINETUNING_FLAG and abs(worldDy) <= availableErr[1]:
        Y_FINETUNING_FLAG = True

    if X_FINETUNING_FLAG and Y_FINETUNING_FLAG:
        Total = 0
        worldDx, worldDy = 0, 0
        print("finish")
        return True

def find_max(blobs):
    if not blobs:  # 如果不存在与色域相符色块处理
        return None
    max_blob = None
    max_size = 0
    for blob in blobs:
        if blob[2] * blob[3] > max_size and abs(blob[2] - blob[3]) < 10 and blob[2] * blob[3] < 2000 and blob[2] * blob[3] > 00:
            max_blob = blob
            max_size = blob[2] * blob[3]
    return max_blob

def getMinRect(major_axis, minor_axis, rate_x = 1, rate_y = 1):
    '''
        主轴坐标按照x坐标升序
        次轴坐标按照y坐标降序
    '''
    if major_axis[0] > major_axis[2]:
        major_axis[0], major_axis[2] = major_axis[2], major_axis[0]
        major_axis[1], major_axis[3] = major_axis[3], major_axis[1]

    if minor_axis[1] < minor_axis[3]:
        minor_axis[0], minor_axis[2] = minor_axis[2], minor_axis[0]
        minor_axis[1], minor_axis[3] = minor_axis[3], minor_axis[1]
    # 获取中心点坐标
    O = [int((major_axis[0] + major_axis[2]) / 2), int((major_axis[1] + major_axis[3]) / 2)]
    # 得到主轴向量
    Arrow = [O[0] - major_axis[0],O[1] - major_axis[1]]
    # 根据向量获取最小外接矩形角点坐标
    Pa = [int((minor_axis[0] - Arrow[0]) * rate_x), int((minor_axis[1] - Arrow[1]) * rate_y)]
    Pb = [int((minor_axis[0] + Arrow[0]) * rate_x), int((minor_axis[1] + Arrow[1]) * rate_y)]
    Pc = [int((minor_axis[2] - Arrow[0]) * rate_x), int((minor_axis[3] - Arrow[1]) * rate_y)]
    Pd = [int((minor_axis[2] + Arrow[0]) * rate_x), int((minor_axis[3] + Arrow[1]) * rate_y)]
    # 按照顺时针顺序排列坐标
    corner = sorted([Pa, Pb, Pc, Pd], key=lambda x: x[1], reverse=False)
    if corner[0][0] > corner[1][0]:
        corner[0], corner[1] = corner[1], corner[0]
    if corner[2][0] < corner[3][0]:
        corner[2], corner[3] = corner[3], corner[2]
    # 最小外接矩形边长
    #length = math.sqrt((Pa[0] - Pb[0]) ** 2 + (Pa[1] - Pb[1]) ** 2)
    # 返回角点坐标(按照顺时针顺序)
    return corner

def resizeCorner(corner, rate_x = 1, rate_y = 1):
    corner = [(int(corner[0][0] * rate_x), int(corner[0][1] * rate_y)),
             (int(corner[1][0] * rate_x), int(corner[1][1] * rate_y)),
             (int(corner[2][0] * rate_x), int(corner[2][1] * rate_y)),
             (int(corner[3][0] * rate_x), int(corner[3][1] * rate_y)),]
    corner = sorted([corner[0], corner[1], corner[2], corner[3]], key=lambda x: x[1], reverse=False)
    if corner[0][0] > corner[1][0]:
        corner[0], corner[1] = corner[1], corner[0]
    if corner[2][0] < corner[3][0]:
        corner[2], corner[3] = corner[3], corner[2]
    return corner

def getImage(isPlace = False):
    minRect = []
    noObject = 0
    # sensor.flush()
    while True:
        img = sensor.snapshot()
        if isPlace == False:
            img1 = img.copy(x_scale =0.75)
            object = tf.detect(net, img1)
            if object:
                x1,y1,x2,y2,label,scores = object[0]
                w = x2- x1
                h = y2 - y1
                x1 = int((x1)*img.width())
                y1 = int(y1*img.height() + 5)
                w = int(w*img.width())
                h = int(h*img.height())
                minRect2 = (x1, y1, w, h)
                minRect = [[x1, y1], [x1 + w, y1], [x1 + w, y1 + h], [x1, y1 + h]]
            else:
                continue
            # print(scores)
            if scores > 0.50:
                print("Send")
                return [img, minRect]
            else:
                 noObject += 1
            if noObject > 2:
                return [None, None]
        else:
            corners = []
            minRect = None
            inputImage = sensor.alloc_extra_fb(160, 120, sensor.GRAYSCALE)
            inputImage.draw_image(img, 0, 0,x_scale=0.5, y_scale=0.5)
            his = inputImage.get_histogram()
            inputImage.binary([[his.get_threshold().value(), 255]], invert = False) # 突出图像特征
           #  img.draw_image(inputImage, 1, 1)

            # rect = inputImage.find_rects(threshold = 2000, roi = (0, 0, 160, 120))
            Blob = inputImage.find_blobs([(230, 255)], roi = (0, 0, 160, 70), area_threshold = 200, merge = True)
            # print(rect)
            maxRect = find_max(Blob)
#            if maxRect:
#                img.draw_line(maxRect.major_axis_line(), color = (0, 0, 0))
#                img.draw_line(maxRect.minor_axis_line(), color = (0, 0, 0))
            if maxRect:
#                corners = [(int(maxRect.rect()[0] * 2), int(maxRect.rect()[1] * 2)),
#                           (int(maxRect.rect()[0] * 2) + int(maxRect.rect()[2] * 2), int(maxRect.rect()[1] * 2)),
#                           (int(maxRect.rect()[0] * 2) + int(maxRect.rect()[2] * 2), int(maxRect.rect()[1] * 2) + int(maxRect.rect()[3] * 2)),
#                           (int(maxRect.rect()[0] * 2), int(maxRect.rect()[1] * 2) + int(maxRect.rect()[3] * 2))]
#                img.draw_rectangle([int(maxRect.rect()[0] * 2), int(maxRect.rect()[1] * 2), int(maxRect.rect()[2] * 2), int(maxRect.rect()[3] * 2)], color = (0, 255, 0))
#                print(maxRect.density())
#                if maxRect.density() < 0.40:
#                corners = resizeCorner(list(maxRect.corners()), rate_x = 2, rate_y = 2)
#                else:
                corners = getMinRect(list(maxRect.major_axis_line()), list(maxRect.minor_axis_line()), rate_x = 2, rate_y = 2)
#                for c in corners:
#                    img.draw_circle(c[0], c[1], 5)
                # corners = resizeCorner(list(maxRect.corners()), rate_x = 2, rate_y = 2)
                # img.rotation_corr(corners = corners)
            sensor.dealloc_extra_fb()
            minRect = corners
            return [img, minRect]



# 分类图像
def classify(img, minRect = [(106, 72), (106 + 97, 72), (106 + 97, 72 + 74), (106, 72 + 74)]):
    # img1 = img.copy(1, 1, roi = minRect, x_scale = (sensor.width() / minRect[2]), y_scale = (sensor.height() / minRect[3]))
    #img.crop(roi = minRect, hint = image.BICUBIC)
    #cimg.draw_image(img1, 1, 1)
    # img.crop(roi = minRect, hint = image.BILINEAR)
    img.rotation_corr(corners=minRect[0:4])
    # print(model)
    # img1 = img.copy(1, 1, x_scale =0.4, y_scale = 0.53)
    # clock.tick()
    obj = tf.classify(model, img)[0]  # 输入图像为img1则Debug_Vision.tflite分类模型, 输入图像为inputImage 则为Debug_VisionClassify的模型
    # print("time:"+str(clock.avg())+"ms")
    # tf.free_from_fb()
    res = obj.output()
    # print(res)
    m = max(res)
    result_index = res.index(m)
    if m >= 0.01:
        result_index = res.index(m)
        uart.write(result_index.to_bytes(1, "little") + UART_SEND_CLASSIFY)
        print("%s: %f" % (SmallPlaceLabels[result_index], m))
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

    obj = tf.classify(SmallPlaceModel, inputImage)[0]
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
    inputImage = sensor.alloc_extra_fb(96, 96, sensor.GRAYSCALE)
    inputImage.draw_image(img, 0, 0,x_scale=0.3, y_scale=0.4)
    inputImage.invert()
    # inputImage.morph(1, sharpen_kernel, 1)
    his = inputImage.get_histogram()
    inputImage.binary([[his.get_threshold().value(), 255]], invert = False) # 突出图像特征
    inputImage.erode(1)
    # img.draw_image(inputImage, 1, 1)
    obj = tf.classify(BigPlaceModel, inputImage)[0]
    # tf.free_from_fb()
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    print("%s: %f" % (BigPlaceLabels[result_index], m))
    # print(res)
    if m >= 0.01:
        _NOP_ = b"\x00"
        result_index = res.index(m)
        uart.write(result_index.to_bytes(1, "little") +  UART_SEND_BIGPLACE)
        print("%s: %f" % (BigPlaceLabels[result_index], m))
    else:
        print("Unknow!")
    # img.draw_image(inputImage, 0, 0)
    sensor.dealloc_extra_fb()

test = True

while(True):
    if test == False:
        while True:
            getTrueDiff()
#            img = sensor.snapshot()
#            classify(img)
#            uart_num = uart.any()
#            if uart_num:
#                uart_str = uart.read()[0]

#                uart_str = uart_str.to_bytes(1, "little")
#                print(uart_str)
#                if uart_str == UART_CLASSIFY_PIC:
#                    if getTrueDiff():
#                        worldDx = 0
#                        worldDy = 0
#                        FINE_FINISHTUNING_FLAG = b"\x01"
#                        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
#                        time.sleep_ms(5)
#                        uart.write(UART_TAIl)
#                        X_FINETUNING_FLAG = False
#                        Y_FINETUNING_FLAG = False
#                    else:
#                        img = sensor.snapshot()
#            else:
#                img = sensor.snapshot()
#            img, minRect = getImage(isPlace = True)
#            if minRect:
#                classifyBigPlace(img, minRect)
#             getTrueDiff_PLACE()
#            img, minRect = getImage()
#            if minRect:
#                classify(img, minRect)
#            break
    else:
        while True:
            uart_num = uart.any()
            if uart_num:
                uart_str = uart.read()[0]

                uart_str = uart_str.to_bytes(1, "little")
                print(uart_str)
                if uart_str == UART_CLASSIFY_PIC:
                    X_FINETUNING_FLAG = False
                    Y_FINETUNING_FLAG = False
                    surviveFrame = 0
                    while True:
                        img, minRect = getImage()
                        if minRect:
                            classify(img, minRect = minRect)
                            break
                        else:
                            surviveFrame += 1
                            # 没有卡片的时候
                        if surviveFrame > 5:
                            uart.write(UART_NOCARD_FLAG + UART_SEND_CLASSIFY)
                            break
                elif uart_str == UART_CLASSIFY_SMALLPLACE:
                    img, minRect = getImage()
                    if img and minRect:
                        classifySmallPlace(img, minRect)
                elif uart_str == UART_CLASSIFY_BIGPLACE:
                    X_FINETUNING_FLAG = False
                    Y_FINETUNING_FLAG = False
                    img, minRect = getImage()
                    if img and minRect:
                        classifyBigPlace(img, minRect)
                elif uart_str == UART_STARTFINETUNING:
                    if getTrueDiff():
                        worldDx = 0
                        worldDy = 0
                        FINE_FINISHTUNING_FLAG = b"\x01"
                        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
                        time.sleep_ms(5)
                        uart.write(UART_TAIl)
                    # continue
                elif uart_str == UART_STARTFINETUNING_PLACE:
                    if getTrueDiff(offsetX = 90, offsetY = -90):
                        worldDx = 0
                        worldDy = 0
                        FINE_FINISHTUNING_FLAG = b"\x01"
                        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
                        time.sleep_ms(5)
                        uart.write(UART_TAIl)
                else:
                    img = sensor.snapshot()
            else:
                img = sensor.snapshot()
                #print("wait")


#corners = [(int(maxRect.rect()[0] * 2), int(maxRect.rect()[1] * 2)),
#           (int(maxRect.rect()[0] * 2) + int(maxRect.rect()[2] * 2), int(maxRect.rect()[1] * 2)),
#           (int(maxRect.rect()[0] * 2) + int(maxRect.rect()[2] * 2), int(maxRect.rect()[1] * 2) + int(maxRect.rect()[3] * 2)),
#           (int(maxRect.rect()[0] * 2), int(maxRect.rect()[1] * 2) + int(maxRect.rect()[3] * 2))]
