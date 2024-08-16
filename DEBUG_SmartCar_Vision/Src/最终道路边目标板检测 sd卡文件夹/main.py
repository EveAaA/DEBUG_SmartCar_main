
import sensor, image, time, tf, gc
from machine import UART
from pyb import LED
import openmv_numpy as np
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
sensor.set_auto_exposure(True, 100)
#sensor.set_vflip(True)
# sensor.set_auto_exposure(True, 70)
clock = time.clock()                # Create a clock object to track the FPS.
red = LED(1)    # 定义一个LED1   红灯
green = LED(2)  # 定义一个LED2   绿灯
#blue = LED(3)   # 定义一个LED3   蓝灯
#white = LED(4)  # 定义一个LED4   照明灯

red.on()
#设置模型路径
border_detect = '/sd/findBesideRoadCardV3.tflite'
net2 = '/sd/Debug_VisionPlace.tflite'
labels = [line.rstrip("\n") for line in open("/sd/label.txt")] + ["none"]
#载入模型
net = tf.load(border_detect)
UART_BIGPLACE_TAIl = b"\xfe\x7e"
UART_TAIl = b"\x7f\x8f"
UART_DIR_STRAIGHT = b"\x12"
UART_DIR_LEFT = b"\x10"
UART_DIR_RIGHT = b"\x11"
UART_BORDER_FIND = b"\x01"
UART_NOT_BORDER_FIND = b"\x00"
UART_GET_BIGPLACE = b'\x19'
UART_GET_SMALLPLACE = b'\x20'
UART_FIND_BESIDE_ROAD = b'\x21'
UART_RING_LEFT_TURN = b'\x22'
UART_RING_RIGHT_TURN = b'\x23'
NO_OBJ = 0
IS_OBJ = 0
LEFT = b'\x01'
RIGHT = b'\x02'
currState = 0
TARGET_LINE = 125
ENDLINE = 55
SLOW_LINE = 250
LOCK_IMAGE_LINE = 50
LEFTLINE = int(sensor.width() / 2) - 36
RIGHTLINE = int(sensor.width() / 2) + 60
uart = UART(2, 115200)
sharpen_kernel = (0, -1, 0, -1, 5, -1, 0, -1, 0)
red.off()
green.on()

def getDistanceFromCardToRoadSide(cx, cy, w, img):
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
    inputImage.draw_line(int(61 / 2), 0, int(82 / 2), int(76 / 2), color = (0, 0, 0), thickness = 5)
    inputImage.draw_line(int(242 / 2), 0, int(219 / 2), int(77 / 2), color = (0, 0, 0), thickness = 5)
    # img.draw_image(inputImage,1, 1)
    # 已知卡片的长宽约为120mm, 中心距离卡片底端60mm, 由于中心距离会有一定的偏差存在
    # 由逆透视可知在320*240像素当中60mm对应差不多65个像素, 缩放至0.5倍, 60mm对应65/2 = 32.5个像素
    # 将卡片的中心往下32.5 + (5, -5) 个像素置为黑色像素
    if cx < 160:
        for i in range(resizeCx, resizeCx + int(w / 4) + 5):
            inputImage.set_pixel(i, resizeCy, 255)
    else:
        for i in range(resizeCx - int(w / 4) - 5, resizeCx):
            inputImage.set_pixel(i, resizeCy - 1, 255)
            inputImage.set_pixel(i, resizeCy, 255)
            inputImage.set_pixel(i, resizeCy + 1, 255)
    #img.draw_image(inputImage, 1 ,1)
    # 从目标板中心点往下寻找黑白跳变点确定赛道边沿位置
    currX = resizeCx
    lastPixel = 0
    pixel_dx = 0
    if cx < 160:
        while True:
            pixel = inputImage.get_pixel(currX, resizeCy)
            nextPixel = inputImage.get_pixel(currX, resizeCy)
            # 找到黑白跳变点
            if lastPixel == 255 and pixel == 0:
                pixel_dx = abs(currX - resizeCx)
                break
            else:
                lastPixel = pixel
                currX += 1
                if currX >= 60:
                    pixel_dx = abs(currX - resizeCx)
                    break
    else:
        while True:
            pixel = inputImage.get_pixel(currX, resizeCy)
            nextPixel = inputImage.get_pixel(currX, resizeCy)
            # 找到黑白跳变点
            if lastPixel == 255 and pixel == 0:
                pixel_dx = abs(currX - resizeCx)
                break
            else:
                lastPixel = pixel
                currX -= 1
                if currX <= 60:
                    pixel_dx = abs(currX - resizeCx)
                    break
    sensor.dealloc_extra_fb()
    img.draw_line(cx, cy, int(currX * 2), cy, color = (255, 0, 0), thickness = 2)
    img.draw_circle(int(currX * 2), cy, 2, color = (0, 255, 0))
    print(abs(pixel_dx - w / 4))
    return abs(pixel_dx - w / 4)
#    pixelDiff = [0, pixel_dy * 2, 1]
#    np_dy = np.array([[p] for p in pixelDiff])#升维
#    np_H = np.array(martrixH)
#    TrueDy = np_H * np_dy
#    worldDy = abs(int(TrueDy[1][0]))
#    img.draw_line(cx, cy, cx, int(currY * 2), color = (255, 0, 0), thickness = 2)
#    img.draw_circle(cx, int(currY * 2), 2, color = (0, 255, 0))
##    print("距离赛道边沿的真实距离:", worldDy)
##    print("赛道边缘与车底", abs(currY - 105))
#    if abs(currY - 105) > 40:
#        worldDy = 999
#    return worldDy

def getBesideRoadCard_RING(isLeftRing = False, isRightRing = False):
    global currState
    RingTargetLine = 0
    if isLeftRing or isRightRing:
        RingTargetLine = 138
    while True:
        img = sensor.snapshot()
        uart_num = uart.any()
        # print(uart_num)
        if uart_num:
            uart_str = uart.read()[0]
            uart_str = uart_str.to_bytes(1, "little")
            if uart_str == UART_GET_SMALLPLACE:
                currState = 1
                return
            elif uart_str == UART_GET_BIGPLACE:
                currState = 4
                return
            elif uart_str == UART_FIND_BESIDE_ROAD:
                currState = 0
                return
        img1 = img.copy(x_scale = 0.75)
        objs = tf.detect(net, img1)
        img.draw_line(0, RingTargetLine, sensor.width(), RingTargetLine, color = (255, 0, 0), thickness = 3)
        for obj in objs:
            x1,y1,x2,y2,label,scores = obj # 获取置信度最高的一项
            if scores < 0.70:
                print("CONTINUE")
                dx = 0
                uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                continue
            else:
                w = x2 - x1
                h = y2 - y1
                x1 = int((x1)*img.width())
                y1 = int(y1*img.height())
                w = int(w*img.width())
                h = int(h*img.height())
                cx = int(x1+w/2)
                cy = int(y1+h/2)
                img.draw_rectangle((x1,y1,w,h),thickness=2, color = (0, 255, 0))
                if isLeftRing and cx < 160:
                    dx = 0
                    if cy > RingTargetLine:
                        print("LEFT")
                        img.draw_line(0, RingTargetLine, sensor.width(), RingTargetLine, color = (0, 255, 0), thickness = 3)
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_LEFT + UART_TAIl)
                        time.sleep_ms(5)
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_LEFT + UART_TAIl)
                        time.sleep_ms(5)
                elif isRightRing and cx > 160:
                    if cy > RingTargetLine:
                        print("RIGHT")
                        img.draw_line(0, RingTargetLine, sensor.width(), RingTargetLine, color = (0, 255, 0), thickness = 3)
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_RIGHT + UART_TAIl)
                        time.sleep_ms(5)
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_RIGHT + UART_TAIl)
                        time.sleep_ms(5)
                else:
                    print("CONTINUE")
                    dx = 0
                    uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)





def getBesideRoadCard():
    global IS_OBJ, NO_OBJ, currState, SLOW_LINE, UART_NOT_BORDER_FIND
    while True:
        img = sensor.snapshot()
        uart_num = uart.any()
        # print(uart_num)
        if uart_num:
            uart_str = uart.read()[0]
            uart_str = uart_str.to_bytes(1, "little")
            if uart_str == UART_GET_SMALLPLACE:
                currState = 1
                return
            elif uart_str == UART_GET_BIGPLACE:
                currState = 4
                return
            elif uart_str == UART_RING_LEFT_TURN:
                currState = 5
                return
            elif uart_str == UART_RING_RIGHT_TURN:
                currState = 6
                return
        img1 = img.copy(x_scale = 0.75)
        #使用模型进行识别
        obj = tf.detect(net, img1)
        if obj:
            x1,y1,x2,y2,label,scores = obj[0] # 获取置信度最高的一项
            print(scores)
            if(scores > 0.85):
                # print(scores)
                NO_OBJ = 0
                IS_OBJ += 1
                w = x2 - x1
                h = y2 - y1
                x1 = int((x1)*img.width())
                y1 = int(y1*img.height())
                w = int(w*img.width())
                h = int(h*img.height())
                cx = int(x1+w/2)
                cy = int(y1+h/2)
                minRect = [(x1, y1), ((x1 + w), y1), ((x1 + w), (y1 + h)), (x1, (y1 + h))]
                pixelDx = getDistanceFromCardToRoadSide(cx, cy, w, img)
                #classifyAlphaBet(img, tuple(minRect))
                if cy <= TARGET_LINE and IS_OBJ >= 2 and (cx >= 176 or cx <= 63) and cy >= ENDLINE and pixelDx <= 6.5:
                    dx = 1
                    img.draw_rectangle((x1,y1,w,h),thickness=2, color = (0, 255, 0))
                    if cx > int(img.width() / 2):
                        print("LEFT")
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_LEFT + UART_TAIl)
                        time.sleep_ms(5)
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_LEFT + UART_TAIl)
                        time.sleep_ms(5)
                        # print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                    else:
                        print("RIGHT")
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_RIGHT + UART_TAIl)
                        time.sleep_ms(5)
                        uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_RIGHT + UART_TAIl)
                        print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                    img.draw_line(0, TARGET_LINE, sensor.width(), TARGET_LINE, color = (255, 0, 0), thickness = 3)
                elif cy <= SLOW_LINE and cy > TARGET_LINE:
                    dx = 1
                    # print("GO AHEAD")
                    uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                    img.draw_line(0, SLOW_LINE,  sensor.width(), SLOW_LINE, color = (255, 255, 0), thickness = 3)
                elif cy >= LOCK_IMAGE_LINE and cy > SLOW_LINE:
                    dx = 1
                    # print("LOCK")
#                    uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
#                    time.sleep_ms(5)
#                    uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                     # img.draw_line(0, LOCK_IMAGE_LINE,  sensor.width(), LOCK_IMAGE_LINE, color = (0, 255, 0), thickness = 3)
                else:
                    dx = 0
                    uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)

            else:
                NO_OBJ += 1
                IS_OBJ = 0
                dx = 0
                uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                # print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
        else:
            NO_OBJ += 1
            IS_OBJ = 0
            dx = 0
            uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
        if NO_OBJ > 5:
            dx = 0
            # print("None")
            uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
            # print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
        img.draw_line(0, ENDLINE,  sensor.width(), ENDLINE, color = (0, 255, 0), thickness = 3)

def getPlace(isSmallPlace = False):
    global currState, LEFTLINE, RIGHTLINE # 转换状态
    while True:
        uart_num = uart.any()
        if uart_num:
            uart_str = uart.read()[0]
            uart_str = uart_str.to_bytes(1, "little")
            print(uart_str)
            if uart_str == UART_FIND_BESIDE_ROAD:
                currState = 0
                return
        img = sensor.snapshot()
        img1 = img.copy(x_scale = 0.75)
        #使用模型进行识别
        obj = tf.detect(net, img1)
        if obj:
            img.draw_line(LEFTLINE, 0, LEFTLINE, int(img.height()), color = (255, 255, 0))
            img.draw_line(LEFTLINE - 10, 0, LEFTLINE - 10, int(img.height()), color = (255, 255, 0))
            img.draw_line(LEFTLINE + 60, 0, LEFTLINE + 60, int(img.height()), color = (255, 255, 0))
            img.draw_line(0, 51, 320, 51, color = (255, 0, 255))
            for o in obj:
                x1,y1,x2,y2,label,scores = o
                print(scores)
                if(scores > 0.70):
                    #print(scores)
                    w = x2- x1
                    h = y2 - y1
                    x1 = int((x1)*img.width())
                    y1 = int(y1*img.height())
                    w = int(w*img.width())
                    h = int(h*img.height())
                    cx = int(x1+w/2)
                    cy = int(y1+h/2)
                    print(cx - LEFTLINE)
                    img.draw_cross(cx, cy, color = (255, 0, 0), thickness = 5)
                    if (cx - LEFTLINE) <= 60 and (cx - LEFTLINE) >= -10 and cy > 51 and isSmallPlace == True:
                        print("FindTarget")
                        currState = 2
                        return
                    elif (cx - LEFTLINE) <= -1 and isSmallPlace == False:
                        currState = 3
                        return
                    else:
                        NOT_FIND_TARGET = b'\x00'
                        _NOP_ = b'\x00'
                        uart.write(NOT_FIND_TARGET + _NOP_ + _NOP_ + UART_BIGPLACE_TAIl)

def UART_SendFindTarget(isSmallPlace = False):
    global currState
    while True:
        img = sensor.snapshot()
        FINDTARGET = b'\x01'
        _NOP_ = b'\x00'
        #print("找到放置区域")
        img.draw_line(LEFTLINE, 0, LEFTLINE, int(img.height()), color = (0, 255, 0))
        #print(FINDTARGET + _NOP_ + _NOP_ + UART_BIGPLACE_TAIl)
        uart.write(FINDTARGET + _NOP_ + _NOP_)
        time.sleep_ms(5)
        uart.write(UART_BIGPLACE_TAIl)
        time.sleep_ms(5)
        uart.write(FINDTARGET + _NOP_ + _NOP_)
        time.sleep_ms(5)
        uart.write(UART_BIGPLACE_TAIl)
        time.sleep_ms(5)
        if isSmallPlace == True:
            uart_num = uart.any()
            if uart_num:
                uart_str = uart.read()[0]
                uart_str = uart_str.to_bytes(1, "little")
                print(uart_str)
                if uart_str == UART_GET_SMALLPLACE:
                    currState = 1
                    return
                elif uart_str == UART_FIND_BESIDE_ROAD:
                    currState = 0
                    return

Dir = b'\x02'

while(True):
    clock.tick()
    sensor.snapshot()
    #getPlace(isSmallPlace = True)
#     getBesideRoadCard()
#    getBesideRoadCard_RING(isLeftRing = False, isRightRing = True)
    getBesideRoadCard()

