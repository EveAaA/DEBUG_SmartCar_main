import pyb
import sensor, image, time, tf, gc
import math
from machine import UART
import openmv_numpy as np
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
sensor.set_brightness(100)
# sensor.set_auto_exposure(True, 120)
clock = time.clock()                # Create a clock object to track the FPS.

#设置模型路径
face_detect = '/sd/fineTuningV5.tflite'
#载入模型
InErr = 0
net = tf.load(face_detect)
UART_TAIl = b"\xfc\xbf"
martrixH = [[2.068966, 0.0, -2.068966], [1.413041e-08, 2.105263, 2.105263], [-6.851109e-11, -0.0, 1]]
FISRT_SEND_FLAG = True
UART_STARTFINETUNING = b'\x05'
UART_FINISH = b'\x20'
uart = UART(2, 9600)

def getTrueDiff():
    global object
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
            img.draw_line(0, 129, 320, 129, color = (255, 255, 0))
            img.draw_line(159, 240, 166, 0, color = (255, 0, 0), thickness = 1)
            img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
        else:
            continue
        print(scores)
        if scores > 0.90 and (w * h) > 1000 and (w * h) < 12000 :
            break
    if object:
        x1,y1,x2,y2,label,scores = object[0]
        w = x2- x1
        h = y2 - y1
        x1 = int((x1)*img.width())
        y1 = int(y1*img.height())
        w = int(w*img.width())
        h = int(h*img.height()+ 6)
        cx = x1 + w / 2
        cy = y1 + h / 2
        center = [(cx - 240), (129 - cy), 1]
        np_center = np.array([[p] for p in center])#升维
        np_H = np.array(martrixH)
        Point = np_H * np_center
        worldDx, worldDy = int(Point[0][0]), int(Point[1][0])
        worldDz = 0
        print(worldDx, worldDy)
        img.draw_cross(int(center[0] + 240), int(129 - center[1]), color = (0, 255, 255))
        img.draw_rectangle(x1, y1, w, h, color = (255, 0 ,0), thickness = 3)
        if abs(worldDx) < 25 and abs(worldDy) < 25:
            worldDx, worldDy = 0, 0
            FINE_FINISHTUNING_FLAG = b"\x01"
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)
        else:
            FINE_FINISHTUNING_FLAG = b"\x00"
            uart.write(worldDx.to_bytes(2, "little") + worldDy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)

if __name__ == "__main__":
    while True:
        clock.tick()
        getTrueDiff()
        print(clock.avg())
    #while True:
        #uart_num = uart.any()
        #if uart_num:
            #uart_str = uart.read()
            #print(uart_str)
            #if uart_str == UART_STARTFINETUNING:
                #getTrueDiff()
            #else:
                #continue























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
