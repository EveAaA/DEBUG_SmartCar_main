# Untitled - By: G15 - 周日 5月 5 2024

import pyb
import sensor, image, time, math
import os, tf
import gc
from machine import UART
import openmv_numpy as np
from pyb import LED
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
# sensor.set_brightness(1500)
sensor.set_auto_exposure(True, 100)
sensor.skip_frames(time = 2000)
clock = time.clock()
classifyModel = "128model_44_0.9945.tflite"                  # 测试98准确率以上量化
# classifyModel = "128model_34_0.9973 (1).tflite"            # MobileNetv2 量化
#classifyModel = "128model_34_0.9973.tflite"                # MobileNetv2 未量化
# classifyModel = "Debug_VisionClassifyV3.tflite"           # 图像分类模型 v3
#SmallPlaceModel = "Debug_VisionPlaceV3.tflite"    # 小类放置区域模型 Debug_VisionPlace
SmallPlaceModel = "place128model_42_0.9989.tflite"    # 小类放置区域模型 MobileNet
BigPlaceModel = "Debug_VisionNum2.tflite"             # 大类放置区域模型
findBorderModel = "fineTuningV8.tflite"         # 找目标板目标检测模型
sharpen_kernel = (0, -1, 0, -1, 5, -1, 0, -1, 0)
red = LED(1)    # 定义一个LED1   红灯
green = LED(2)  # 定义一个LED2   绿灯
blue = LED(3)   # 定义一个LED3   蓝灯
white = LED(4)  # 定义一个LED4   照明灯
red.on()
SmallPlaceLabels = [line.rstrip("\n") for line in open("/sd/label.txt")] + ["none"]
BigPlaceLabels = [line.rstrip("\n") for line in open("/sd/number.txt")] + ["none"]
BigPlaceModel = tf.load(BigPlaceModel, load_to_fb=True)
SmallPlaceModel = tf.load(SmallPlaceModel, load_to_fb = True)
model = tf.load(classifyModel, load_to_fb=True)
net = tf.load(findBorderModel)
uart = UART(2, 115200)
IS_CARD_FLAG = False
X_FINETUNING_FLAG = False
Y_FINETUNING_FLAG = False
CY_ISBORDER_THRESHOLD = 40
CLASSIFY_NUM = 0
worldDx = 0
worldDy = 0
#########使用不同类型模型分类图像的串口标志位#######
UART_HEAD = b'\xee\xed'
UART_NOCARD_FLAG = b'\xbc'
UART_TAIl = b"\xfc\xbf"
martrixH = [[2.068966, -0.0, -2.068966], [1.233199e-07, 2.142857, 2.142857], [1.027666e-09, -0.0, 1]]
UART_STARTFINETUNING_BESIDEROAD = b'\x04'
UART_STARTFINETUNING = b'\x05'
UART_STARTFINETUNING_Y = b'\x07'
UART_STARTFINETUNING_PLACE = b'\x10'
UART_CLASSIFY_PIC = b'\x06'
UART_MOVEVOLUMEUP_FLAG = b'\x11'
UART_CLASSIFY_SMALLPLACE = b'\x07'
UART_CLASSIFY_BIGPLACE = b'\x08'
UART_UNLOAD_FLAG = b'\x12'
UART_FINETUNING_BIGPLACE = b'\x13'


UART_SEND_SMALLPLACE = b'\xfe\xef'  # 发送小类帧尾
UART_SEND_BIGPLACE   = b'\xfd\xdf'    # 发送大类帧尾
UART_SEND_CLASSIFY   = b'\xfb\xcf'    # 发送目标板种类帧尾
UART_SEND_ERR        = b'\xfb\xfa'
SEND_FLAG = False
red.off()
green.on()


def Limit(num, low, high):
    if num < low:
        return low
    elif num > high:
        return high
    return num


def getDistanceFromCardToRoadSide(cx, cy, h, img):
    # 将原图像缩放成160 * 120
    resizeCx = int(0.5 * cx)
    resizeCy = int(0.5 * cy)
    inputImage = sensor.alloc_extra_fb(160, 120, sensor.GRAYSCALE)
    inputImage.draw_image(img, 0, 0,x_scale=0.5, y_scale=0.5)
    inputImage.invert()
    # inputImage.morph(1, sharpen_kernel, 1)
    his = inputImage.get_histogram()
    inputImage.binary([[his.get_threshold().value(), 255]], invert = False) # 突出图像特征
    inputImage.erode(1)
    # 已知卡片的长宽约为120mm, 中心距离卡片底端60mm, 由于中心距离会有一定的偏差存在
    # 由逆透视可知在320*240像素当中60mm对应差不多65个像素, 缩放至0.5倍, 60mm对应65/2 = 32.5个像素
    # 将卡片的中心往下32.5 + (5, -5) 个像素置为黑色像素
    for i in range(resizeCy, resizeCy + int(h / 4) + 5):
        inputImage.set_pixel(resizeCx, i, 255)
   #  img.draw_image(inputImage, 1 ,1)
    # 从目标板中心点往下寻找黑白跳变点确定赛道边沿位置
    currY = resizeCy
    lastPixel = 0
    pixel_dy = 0
    while True:
        pixel = inputImage.get_pixel(resizeCx, currY)
        nextPixel = inputImage.get_pixel(resizeCx, currY)
        # 找到黑白跳变点
        if lastPixel == 255 and pixel == 0:
            pixel_dy = abs(currY - resizeCy - int(h / 4))
            break
        else:
            lastPixel = pixel
            currY += 1
            if currY >= 118:
                pixel_dy = abs(currY - resizeCy - int(h / 4))
                break
    sensor.dealloc_extra_fb()
    pixelDiff = [0, pixel_dy * 2, 1]
    np_dy = np.array([[p] for p in pixelDiff])#升维
    np_H = np.array(martrixH)
    TrueDy = np_H * np_dy
    worldDy = abs(int(TrueDy[1][0]))
    img.draw_line(cx, cy, cx, int(currY * 2), color = (255, 0, 0), thickness = 2)
    img.draw_circle(cx, int(currY * 2), 2, color = (0, 255, 0))
#    print("距离赛道边沿的真实距离:", worldDy)
#    print("赛道边缘与车底", abs(currY - 105))
    if abs(currY - 105) > 99:
        worldDy = 999
    return worldDy

def getTrueDiff(offsetX = 0, offsetY = 0, isBesideRoad = False, isBigPlace = False):
    global object
    global Total
    global X_FINETUNING_FLAG, IS_CARD_FLAG, worldDx, worldDy
    availableErr = [0, 0]
    maxRect = None
    searchNum = 0
#    worldDx = 0
#    worldDy = 0
    if offsetX == 0 and offsetY == 0:
        availableErr[0] = 10
        availableErr[1] = 10
    else:
        availableErr[0] = 10
        availableErr[1] = 10
    searchNum = 0
    scores = 0
    while True:
        img = sensor.snapshot()
#        if offsetX == 0 and offsetY == 0:
        img1 = img.copy(1, 1, x_scale = 0.75)
        object = tf.detect(net, img1)
        if searchNum >= 2:
            break
        if object:
            searchNum = 0
            x1,y1,x2,y2,label,scores = object[0]
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height())
            w = int(w*img.width())
            h = int(h*img.height())
            cx = int(x1 + w / 2)
            cy = int(y1 + h / 2)
            print(scores)
            if offsetX == 0 and offsetY == 0:
                ErrBesideRoad = getDistanceFromCardToRoadSide(cx, cy, h, img)
                print(ErrBesideRoad)
                if not IS_CARD_FLAG and isBesideRoad:
                    if (w * h) > 9000 or cy <= 55 or (ErrBesideRoad > CY_ISBORDER_THRESHOLD) and isBesideRoad:
                        print("None")
                        return False
                    elif scores > 0.70 and ErrBesideRoad < CY_ISBORDER_THRESHOLD and isBesideRoad:
                        print("FInd")
                        IS_CARD_FLAG = True
                        break
                elif IS_CARD_FLAG and isBesideRoad:
                    print("FIND")
                    if scores > 0.70:
                        break
                elif scores > 0.70 and not isBesideRoad:
                    break
            else:
                if scores > 0.75:
                    break
                elif scores > 0.5 and isBigPlace:
                    break
        else:
            searchNum += 1
    if object:
        global X_FINETUNING_FLAG
        global Y_FINETUNING_FLAG
        serachNum = 0
        x1,y1,x2,y2,label,scores = object[0]
        #print("send score:", scores)
        w = x2 - x1
        h = y2 - y1
        x1 = int((x1)*img.width())
        y1 = int(y1*img.height())
        w = int(w*img.width())
        h = int(h*img.height())
        cx = x1 + w / 2
        cy = y1 + h / 2
        center = [(cx - 152 + offsetX), (177 - cy + offsetY), 1]
        np_center = np.array([[p] for p in center])#升维
        np_H = np.array(martrixH)
        Point = np_H * np_center
        worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
#        if offsetX != 0 or offsetY != 0:
#            worldDy = int(Limit(Point[1][0], -150, 150))
        worldDz = 0
        img.draw_line(0, 177, 320, 177, color = (255, 255, 0))
        img.draw_line(152, 240, 152, 0, color = (255, 0, 0), thickness = 1)
        img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
        img.draw_cross(int(cx), int(cy), color = (0, 255, 0))

#    print(X_FINETUNING_FLAG, Y_FINETUNING_FLAG)
    _NOP_ = b'\x00'
    if offsetX == 0 and offsetY == 0 and not isBesideRoad:
        if searchNum == 0:
            print(worldDx, worldDy)
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + _NOP_)
            time.sleep_ms(5)
            uart.write(UART_TAIl)
        else:
            X_FINETUNING_FLAG = False
            Y_FINETUNING_FLAG = False
            worldDx = -999
            worldDy = -999
            # print("NO TARGET")
            FINE_FINISHTUNING_FLAG = b"\x00"
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
            time.sleep_ms(5)
            uart.write(UART_TAIl)
    elif offsetX == 0 and offsetY == 0 and isBesideRoad:
        if searchNum == 0 and IS_CARD_FLAG:
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + _NOP_)
            time.sleep_ms(5)
            uart.write(UART_TAIl)
        elif searchNum != 0 and not IS_CARD_FLAG:
            X_FINETUNING_FLAG = False
            Y_FINETUNING_FLAG = False
            worldDx = -999
            worldDy = -999
            # print("NO TARGET")
            FINE_FINISHTUNING_FLAG = b"\x00"
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
            time.sleep_ms(5)
            uart.write(UART_TAIl)
    elif isBigPlace:
        if searchNum == 0 and cx < (17 + 269):
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + _NOP_)
            time.sleep_ms(5)
            uart.write(UART_TAIl)
        else:
            X_FINETUNING_FLAG = False
            Y_FINETUNING_FLAG = False
            worldDx = -999
            worldDy = -999
            print("NO TARGET")
            FINE_FINISHTUNING_FLAG = b"\x00"
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
            time.sleep_ms(5)
            uart.write(UART_TAIl)
    else:
        if searchNum == 0:
            print(worldDx, worldDy)
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + _NOP_)
            time.sleep_ms(5)
            uart.write(UART_TAIl)
    return True
#    if abs(worldDx) > availableErr[0] and not X_FINETUNING_FLAG:
#        FINE_FINISHTUNING_FLAG = b"\x00"
#        _NOP_  = b'\xff'
#        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
#        time.sleep_ms(5)
#        uart.write(UART_TAIl)
#    else:
#        X_FINETUNING_FLAG = True
#    time.sleep_ms(5)
#    if abs(worldDy) > availableErr[1] and X_FINETUNING_FLAG:
#        FINE_X_FLAG = b'\x02'
#        print("x_finish")
#        # 当X轴调整完成发送十六进制0x02
#        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_X_FLAG)
#        time.sleep_ms(5)
#        uart.write(UART_TAIl)
#    elif X_FINETUNING_FLAG and abs(worldDy) <= availableErr[1]:
#        Y_FINETUNING_FLAG = True

#    if X_FINETUNING_FLAG and Y_FINETUNING_FLAG:
#        Total = 0
#        worldDx, worldDy = 0, 0
#        print("finish")
#        return True

def find_max(blobs):
    if not blobs:  # 如果不存在与色域相符色块处理
        return None
    max_blob = None
    max_size = 0
    for blob in blobs:
        if blob[2] * blob[3] > max_size and abs(blob[2] - blob[3]) < 15 and blob[2] * blob[3] < 3000:
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

def resetSensor(light):
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.set_brightness(light)

def getImage(isPlace = False):
    minRect = []
    minRect2 = []
    noObject = 0
    scores = 0
    # sensor.flush()
    # resetSensor(500)
    while True:
        img = sensor.snapshot()
        # if isPlace == False:
        img1 = img.copy(1, 1, x_scale =0.75)
        object = tf.detect(net, img1)
        if object:
            x1,y1,x2,y2,label,scores = object[0]
            # print(scores)
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height())
            w = int(w*img.width())
            h = int(h*img.height())
            cx = int(x1 + w / 2)
            cy = int(y1 + h / 2)
            minRect2 = (x1, y1, w, h)
            #img.draw_rectangle(minRect2)
            if not isPlace:
                #minRect = minRect2 55 110
                minRect = [cx - 45, cy - 45, 90, 90]
                # minRect = [[cx - 40, cy - 40], [cx + 40, cy - 40], [cx + 40, cy + 40], [cx - 40, cy + 40]]
                # img.draw_edges(minRect)
            else:
                minRect = [cx - 50, cy - 50, 100, 100]

#                minRect = [[cx - 40, cy - 40], [cx + 40, cy - 40], [cx + 40, cy + 40], [cx - 40, cy + 40]]

                # minRect = [[x1 - 5 , y1 - 5], [x1 + w + 5, y1 - 5], [x1 + w + 5, y1 + h + 5], [x1 - 5, y1 + h + 5]]
                 #img.draw_rectangle(rect = minRect)
        # print(scores)
        if scores > 0.75 and isPlace:
            # print("Send")
            return [img, minRect]
        elif scores > 0.75 and not isPlace:
            return [img, minRect]
        else:
            print("No object")
            noObject += 1
        if noObject >= 2:
            return [None, None]

# 分类图像
def classify(img, minRect = [105, 120, 85, 85]):
    # print(minRect)
    img1 = img.copy(1, 1, minRect)
    img.draw_rectangle(minRect)
    obj = tf.classify(model, img1, min_scale=1.0, scale_mul=0.5, x_overlap=0.0, y_overlap=0.0)[0]
    res = obj.output()
    #print(res)
    m = max(res)
    result_index = res.index(m)
    if m >= 0.80:
        result_index = res.index(m)
        img.draw_string(10, 10, SmallPlaceLabels[result_index], (255, 0, 0), 5)
        img.draw_string(10, 60, str(m), (255, 0, 0), 3)
        uart.write(result_index.to_bytes(1, "little") + UART_SEND_CLASSIFY)

# 分类小类目标板
def classifySmallPlace(img, minRect = [105, 120, 85, 85]):
    # inputImage = img.copy(1, 1, minRect)
    #img.rotation_corr(corners=minRect[0:4])
    img1 = img.copy(1, 1, minRect)
    img.draw_rectangle(minRect)
    # inputImage = sensor.alloc_extra_fb(128, 128, sensor.GRAYSCALE)
#    inputImage.draw_image(img1, 0, 0, x_scale=(128 / 100), y_scale= (128 / 100))
#    inputImage.invert()
#    inputImage.morph(1, sharpen_kernel, 1)
#    his = inputImage.get_histogram()
#    inputImage.binary([[his.get_threshold().value(), 255]], invert = True) # 突出图像特征
#    inputImage.close(1)
#    inputImage.erode(1)
#    img.draw_image(inputImage, 1, 1)
    obj = tf.classify(SmallPlaceModel, img1, min_scale=1.0, scale_mul=0.5, x_overlap=0.0, y_overlap=0.0)[0]
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    #print("%s: %f" % (SmallPlaceLabels[result_index], m))
    # print(res)
    if m >= 0.85:
        result_index = res.index(m)
        img.draw_string(10, 10, SmallPlaceLabels[result_index], (255, 0, 0), 5)
        img.draw_string(10, 60, str(m), (255, 0, 0), 3)
        uart.write(result_index.to_bytes(1, "little") + UART_SEND_SMALLPLACE)
        # print("%s: %f" % (SmallPlaceLabels[result_index], m))
    else:
        print("Unknow!")
    # img.draw_image(inputImage, 0, 0)
    # sensor.dealloc_extra_fb()

# 分类大类目标板
def classifyBigPlace(img, minRect):
    # img.rotation_corr(corners=minRect[0:4])
    img1 = img.copy(1, 1, minRect)
    inputImage = sensor.alloc_extra_fb(128, 128, sensor.GRAYSCALE)
    inputImage.draw_image(img1, 1, 1, x_scale= (128 / 100), y_scale= (128 / 100))
    inputImage.invert()
    # inputImage.morph(1, sharpen_kernel, 1)
    his = inputImage.get_histogram()
    inputImage.binary([[his.get_threshold().value(), 255]], invert = True) # 突出图像特征
    inputImage.dilate(1)
    img.draw_image(inputImage, 1, 1)
    obj = tf.classify(BigPlaceModel, inputImage)[0]
    # tf.free_from_fb()
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    # print("%s: %f" % (BigPlaceLabels[result_index], m))
    # print(res)
    if m >= 0.85:
        _NOP_ = b"\x00"
        result_index = res.index(m)
        uart.write(result_index.to_bytes(1, "little") +  UART_SEND_BIGPLACE)
        img.draw_string(10, 10, BigPlaceLabels[result_index], (255, 0, 0), 5)
        img.draw_string(10, 60, str(m), (255, 0, 0), 3)
        # print("%s: %f" % (BigPlaceLabels[result_index], m))
    else:
        print("Unknow!")
    # img.draw_image(inputImage, 0, 0)
    sensor.dealloc_extra_fb()

def find_maxBlob(blobs):
    if not blobs:  # 如果不存在与色域相符色块处理
        return None
    max_blob = None
    max_size = 0
    for blob in blobs:
        if blob[2] * blob[3] > max_size:
            max_blob = blob
            max_size = blob[2] * blob[3]
    return max_blob

def getAngleErr(inputImage, roi, img):
    LeftHeight = 0
    RightHeight = 0
    LeftHeight = 0
    RightHeight = 0
    for i in range(roi[1], roi[1] + roi[3]):
        pixelLeft = inputImage.get_pixel(roi[0] + 5, i)
        pixelRight = inputImage.get_pixel(roi[0]+roi[2] - 5, i)
        if pixelLeft == 0 and LeftHeight == 0:
            LeftHeight = i
            img.draw_circle(int(roi[0] * 2), int(i * 2), 5, color = (255, 0, 0))
        if pixelRight == 0 and RightHeight == 0:
            RightHeight = i
            img.draw_circle(int((roi[0] + roi[2]) * 2), int(i * 2), 5, color = (0, 0, 255))
        if LeftHeight != 0 and RightHeight != 0:
            Err = LeftHeight - RightHeight
            return Err
    Err = LeftHeight - RightHeight
    return Err

def getVolumeUpErr():
    img = sensor.snapshot()# .lens_corr(1.8)
    inputImage = sensor.alloc_extra_fb(160, 120, sensor.GRAYSCALE)
    inputImage.draw_image(img, 0, 0,x_scale=0.5, y_scale=0.5)
    inputImage.invert()
    # inputImage.morph(1, sharpen_kernel, 1)
    his = inputImage.get_histogram()
    inputImage.binary([[his.get_threshold().value(), 255]], invert = False) # 突出图像特征
    inputImage.erode(1)
    Blob = inputImage.find_blobs([(0, 10)], invert = False, area_threshold = 1000)
    maxRect = find_maxBlob(Blob)
    if maxRect:
        angleErr = getAngleErr(inputImage, maxRect.rect(), img)
        heightErr = int(maxRect.rect()[1] - (inputImage.height() / 2) + 13)
        uart.write(angleErr.to_bytes(1, "little") + heightErr.to_bytes(1, "little") + UART_SEND_ERR)
        print(angleErr, heightErr)
    if maxRect:
        img.draw_rectangle([int(maxRect.rect()[0] * 2), int(maxRect.rect()[1] * 2), int(maxRect.rect()[2] * 2), int(maxRect.rect()[3] * 2)], color = (0, 255, 0))
   #  img.draw_image(inputImage, 1 ,1)
    sensor.dealloc_extra_fb()

test = False
while(True):
    if test == True:
        while True:
           #  getVolumeUpErr()
            img, minRect = getImage(isPlace = True)
            if minRect:
                classifySmallPlace(img, minRect = minRect)
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
                    IS_CARD_FLAG = False
                    surviveFrame = 0
                    while True:
                        img, minRect = getImage()
                        if minRect:
                            classify(img, minRect = minRect)
                            break
                        else:
                            CLASSIFY_NUM = 0
                            _NOP_ = b'\x00'
                            green.on()
                            uart.write(UART_NOCARD_FLAG + UART_SEND_CLASSIFY)
                            time.sleep_ms(10)
                            uart.write(UART_NOCARD_FLAG + UART_SEND_CLASSIFY)
                            time.sleep_ms(10)
                            uart.write(UART_NOCARD_FLAG + UART_SEND_CLASSIFY)
                            green.off()
                            break
                elif uart_str == UART_CLASSIFY_SMALLPLACE:
                    X_FINETUNING_FLAG = False
                    Y_FINETUNING_FLAG = False
                    IS_CARD_FLAG = False
                    img, minRect = getImage(isPlace = True)
                    if img and minRect:
                        classifySmallPlace(img, minRect)
                elif uart_str == UART_CLASSIFY_BIGPLACE:
                    X_FINETUNING_FLAG = False
                    Y_FINETUNING_FLAG = False
                    IS_CARD_FLAG = False
                    img, minRect = getImage(isPlace = True)
                    if img and minRect:
                        classifyBigPlace(img, minRect)
                elif uart_str == UART_STARTFINETUNING_BESIDEROAD:
                    if not getTrueDiff(isBesideRoad = True):
                        X_FINETUNING_FLAG = False
                        Y_FINETUNING_FLAG = False
                        worldDx = -999
                        worldDy = -999
                        FINE_FINISHTUNING_FLAG = b"\x00"
                        uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG)
                        time.sleep_ms(5)
                        uart.write(UART_TAIl)
                elif uart_str == UART_STARTFINETUNING:
                    getTrueDiff()
                elif uart_str == UART_STARTFINETUNING_PLACE:
                    getTrueDiff(offsetX = 56, offsetY = -41, isBigPlace = False)
                elif uart_str == UART_MOVEVOLUMEUP_FLAG:
                    getVolumeUpErr()
                elif uart_str == UART_UNLOAD_FLAG:
                    getTrueDiff(offsetX = 63, offsetY = -80, isBigPlace = True)
                elif uart_str == UART_FINETUNING_BIGPLACE:
                    getTrueDiff(offsetX = 56, offsetY = -41, isBigPlace = True)
                else:
                    img = sensor.snapshot()
            else:
                img = sensor.snapshot()
                print("wait")
