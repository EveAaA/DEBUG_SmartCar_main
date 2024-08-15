import pyb
import sensor, image, time, tf, gc
import math
from machine import UART
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
sensor.set_auto_exposure(True, 120)
clock = time.clock()                # Create a clock object to track the FPS.

#设置模型路径
face_detect = '/sd/fineTuningV2.tflite'
#载入模型
LastCx = 0
LastCy = 0
LastDx = 0
LastDy = 0
InErr = 0
net = tf.load(face_detect)
UART_TAIl = b"\x7E"
UART_HEAD = b"\x7f"
LAST_THEATA = 0
uart = UART(2, 115200)

while(True):
    clock.tick()
    img = sensor.snapshot()
    img1 = img.copy(x_scale = 0.75)
    #使用模型进行识别
    obj = tf.detect(net, img1)
    if obj:
        x1,y1,x2,y2,label,scores = obj[0]
        if(scores>0.70):
            # print(obj[0])
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height() + 5)
            w = int(w*img.width())
            h = int(h*img.height())
            cx = int(x1+w/2)
            cy = int(y1+h/2)
            dx = int((cx - img.width() / 2))
            dy = int((cy - img.height() / 2))
            Theata = int(math.atan(dx / (dy + 1e-10)) * 180 / math.pi)
            print(dx, dy, int(Theata))
            if abs(dx) < 10 and abs(dy) < 10:
                InErr += 1
                if InErr > 5:
                    print("finish")
                    dx = 0
                    dy = 0
                    Theata = 0
                    UART_TUNING_FLAG = b'\x00'
                    uart.write(dx.to_bytes(1, "little") + dy.to_bytes(1, "little") + Theata.to_bytes(1, "little") + UART_TUNING_FLAG + UART_TAIl)
                #######这一部分进入识别类别的部分##########
            else:
                InErr = 0
                print("not finish")
                UART_TUNING_FLAG = b'\x01'
                data = UART_HEAD + dx.to_bytes(1, "little") + dy.to_bytes(1, "little") + Theata.to_bytes(1, "little") + UART_TUNING_FLAG + UART_TAIl
                uart.write(data)
                print(data)
                # print(dx.to_bytes(1, "little") + dy.to_bytes(1, "little") + UART_TUNING_FLAG + UART_TAIl)
            img.draw_line(int(cx), int(cy), int(img.width() / 2), int(img.height() / 2), color = (255, 0, 0), thickness = 3)
            img.draw_cross(int(cx), int(cy), color = (0, 255, 255), thickness = 3)
            img.draw_rectangle((x1,y1,w,h),thickness=2, color = (0, 255, 0))
    # 没有找得到就发送结束微调标志位为非法值 同时将偏差量设置为0
    else:
        dx = 0
        dy = 0
        Theata = 0
        UART_TUNING_FLAG = b'\x02'
        uart.write(dx.to_bytes(1, "little") + dy.to_bytes(1, "little") + Theata.to_bytes(1, "little") + UART_TUNING_FLAG + UART_TAIl)

    # print(clock.ticks_ms())
