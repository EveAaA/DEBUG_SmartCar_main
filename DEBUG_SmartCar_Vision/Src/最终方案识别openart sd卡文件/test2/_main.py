import pyb
import sensor, image, time, tf, gc
import math
from machine import UART
import openmv_numpy as np
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_auto_exposure(True, 120)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
# sensor.set_auto_exposure(True, 120)

clock = time.clock()                # Create a clock object to track the FPS.

#设置模型路径
face_detect = '/sd/fineTuningV2.tflite'
#载入模型
InErr = 0
net = tf.load(face_detect, load_to_fb = True)
UART_TAIl = b"\xfc\xbf"
martrixH = [[2.0, 4.440892e-16, -2.0], [0.0, 2.033898, 0.0], [-1.059638e-09, 1.683747e-11, 1]]

FISRT_SEND_FLAG = True
LastWorldDx = 0
LastWorldDy = 0
FineFlag = False
Total = 0
UART_STARTFINETUNING = b'\x05'
UART_FINISH = b'\x20'
uart = UART(2, 9600)

def getTrueDiffV1():
    global object
    # time.sleep_ms(1500)
    while True:
        img = img = sensor.snapshot()
        img1 = img.copy(x_scale = 0.75)
        object = tf.detect(net, img1)
        if object:
            x1,y1,x2,y2,label,scores = object[0]
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height())
            w = int(w*img.width())
            h = int(h*img.height() + 6)
            img.draw_line(0, 153, 320, 153, color = (255, 255, 0))
            img.draw_line(162, 240, 162, 0, color = (255, 0, 0), thickness = 1)
            img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
        else:
            continue
        # print(scores)
        if scores > 0.80 and (w * h) > 1000 and (w * h) < 10000:
            break
    if object:
        x1,y1,x2,y2,label,scores = object[0]
        w = x2- x1
        h = y2 - y1
        x1 = int((x1)*img.width())
        y1 = int(y1*img.height())
        w = int(w*img.width())
        h = int(h*img.height()+6)
        cx = x1 + w / 2
        cy = y1 + h / 2
        center = [(cx - 162), (153 - cy), 1]
        np_center = np.array([[p] for p in center])#升维
        np_H = np.array(martrixH)
        Point = np_H * np_center
        worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
        worldDz = 0
        print(worldDx, worldDy)
        img.draw_cross(int(center[0] + 162), int(153 - center[1]), color = (0, 255, 255))
        img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
        if abs(worldDx) < 10 and abs(worldDy) < 10:
            worldDx, worldDy = 0, 0
            FINE_FINISHTUNING_FLAG = b"\x01"
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)
        else:
            FINE_FINISHTUNING_FLAG = b"\x00"
            if abs(worldDx) < 10:
                worldDx = 0
            elif abs(worldDy) < 10:
                worldDy = 0
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)


def getTrueDiff():
    global object
    global Total
    searchNum = 0
    while True:
        img = img = sensor.snapshot()
        img1 = img.copy(x_scale = 0.75)
        object = tf.detect(net, img1)
        if object:
            x1,y1,x2,y2,label,scores = object[0]
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height())
            w = int(w*img.width())
            h = int(h*img.height() + 6)
            cx = x1 + w / 2
            cy = y1 + h / 2
            center = [(cx - 162), (153 - cy), 1]
            np_center = np.array([[p] for p in center])#升维
            np_H = np.array(martrixH)
            Point = np_H * np_center
            worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
            img.draw_line(0, 153, 320, 153, color = (255, 255, 0))
            img.draw_line(162, 240, 162, 0, color = (255, 0, 0), thickness = 1)
            img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
            img.draw_cross(int(cx), int(cy), color = (0, 255, 0))
            if scores > 0.90 and (cx > 30 or cx < 210) and (w * h) > 1000:
                #print(worldDx, worldDy)
                if abs(worldDx) < 10 and abs(worldDy) < 10:
                    Total += 1
                else:
                    Total = 0
                searchNum += 1
                if searchNum > 5:
                    searchNum = 0
                    break
        else:
            continue

    if object:
        x1,y1,x2,y2,label,scores = object[0]
        w = x2- x1
        h = y2 - y1
        x1 = int((x1)*img.width())
        y1 = int(y1*img.height() + 5)
        w = int(w*img.width())
        h = int(h*img.height())
        cx = x1 + w / 2
        cy = y1 + h / 2
        center = [(cx - 150), (133 - cy), 1]
        np_center = np.array([[p] for p in center])#升维
        np_H = np.array(martrixH)
        Point = np_H * np_center
        worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
        worldDz = 0
        #print(worldDx, worldDy)
        #img.draw_cross(int(center[0] + 147), int(129 - center[1]), color = (0, 255, 255))
        #img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
        if Total > 4:
            Total = 0
            worldDx, worldDy = 0, 0
            print("finish")
            FINE_FINISHTUNING_FLAG = b"\x01"
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)
        else:
            Total = 0
            print("keep")
            FINE_FINISHTUNING_FLAG = b"\x00"
            if abs(worldDx) < 10:
                worldDx = 0
            elif abs(worldDy) < 10:
                worldDy = 0
            print(worldDx, worldDy)
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)

test = False

if __name__ == "__main__":
    if test:
        while True:
            getTrueDiffV1()
    else:
        while True:
            uart_num = uart.any()
            if uart_num:
                uart_str = uart.read()
                print(uart_str)
                if uart_str == UART_STARTFINETUNING:
                    getTrueDiffV1()
                else:
                    continue
            else:
                img = sensor.snapshot()























##  目标板实际宽高为120mm
#while(True):
    #clock.tick()
    ##如果是第一次看到
    #if FISRT_SEND_FLAG:
        #img = sensor.snapshot()
        #img1 = img.copy(x_scale = 0.75)
        #obj = tf.detect(net, img1)
        #if obj:
            #x1,y1,x2,y2,label,scores = obj[0]
            #if(scores>0.80):
                ## print(obj[0])
                #w = x2- x1
                #h = y2 - y1
                #x1 = int((x1)*img.width())
                #y1 = int(y1*img.height())
                #w = int(w*img.width())
                #h = int(h*img.height())
                #cx = x1+w/2
                #cy = y1+h/2
                #center = [(cx - 160), (120 - cy), 1]
                #img.draw_cross(int(center[0] + 160), int(120 - center[1]), color = (0, 255, 255))
                #FINE_FINISHTUNING_FLAG = b'\x00'
                #np_center = np.array([[p] for p in center])#升维
                #np_H = np.array(martrixH)
                #Point = np_H * np_center
                #worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
                #worldDz = 0
                #print(worldDx, worldDy)
                #uart.write(worldDx.to_bytes(1, "little") + worldDy.to_bytes(1, "little") + worldDz.to_bytes(1, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)
                #FISRT_SEND_FLAG = False
    #else:
        ## 等待主控微调之后发送数据
        #uart_num = uart.any()
        #if uart_num:
            #uart_str = uart.read(uart_num)
            #print(uart_str)
            #if uart_str.to_bytes(1, "little") == UART_FINISH:
                #global object
                ## 不断拍照直到找到卡片的准确率为80以上为止
                #while True:
                    #img = sensor.snapshot()
                    #img1 = img.copy(x_scale = 0.75)
                    #object = tf.detect(net, img1)
                    #x1,y1,x2,y2,label,scores = obj[0]
                    #if obj[0] > 80:
                        #break
                #if object:
                    #x1,y1,x2,y2,label,scores = object[0]
                    #if(scores>0.80):
                        ## print(obj[0])
                        #w = x2- x1
                        #h = y2 - y1
                        #x1 = int((x1)*img.width())
                        #y1 = int(y1*img.height())
                        #w = int(w*img.width())
                        #h = int(h*img.height())
                        #cx = x1+w/2
                        #cy = y1+h/2
                        #center = [(cx - 160), (120 - cy), 1]
                        #img.draw_cross(int(center[0] + 160), int(120 - center[1]), color = (0, 255, 255))
                        #FINE_FINISHTUNING_FLAG = b'\x00'
                        #np_center = np.array([[p] for p in center])#升维
                        #np_H = np.array(martrixH)
                        #Point = np_H * np_center
                        #worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
                        #worldDz = 0
                        #print(worldDx, worldDy)
                        #if abs(worldDx) < 15 and abs(worldDy) < 15:
                            #worldDx = 0
                            #worldDy = 0
                            #FINE_FINISHTUNING_FLAG = b"\x01"
                            #uart.write(worldDx.to_bytes(1, "little") + worldDy.to_bytes(1, "little") + worldDz.to_bytes(1, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)
                        #else:
                            #FINE_FINISHTUNING_FLAG = b"\x00"
                            #uart.write(worldDx.to_bytes(1, "little") + worldDy.to_bytes(1, "little") + worldDz.to_bytes(1, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)

    # print(clock.ticks_ms())
