import pyb
import sensor, image, time, tf, gc
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
lastDx = 0
lastDy = 0
net = tf.load(face_detect)
UART_TAIl = b"\x80"
uart = UART(2, 115200)

while(True):
    clock.tick()
    img = sensor.snapshot()
    img1 = img.copy(x_scale = 0.75)
    #使用模型进行识别
    obj = tf.detect(net, img1)
    if obj:
        x1,y1,x2,y2,label,scores = obj[0]
        if(scores>0.65):
            # print(obj[0])
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height())
            w = int(w*img.width())
            h = int(h*img.height())
            cx = int(x1+w/2)
            cy = int(y1+h/2)
            dx = int(cx - img.width() / 2)
            dy = int(cy - img.height() / 2)
            print(dx, dy)
            if abs(dx) < 5 and abs(dy) < 5:
                dx = 0
                dy = 0
                UART_TUNING_FLAG = b'\x00'
                uart.write(dx.to_bytes(1, "little") + dy.to_bytes(1, "little") + UART_TUNING_FLAG + UART_TAIl)
                #######这一部分进入识别类别的部分##########
            else:
                UART_TUNING_FLAG = b'\x01'
                uart.write(dx.to_bytes(1, "little") + dy.to_bytes(1, "little") + UART_TUNING_FLAG + UART_TAIl)
                print(dx.to_bytes(1, "little") + dy.to_bytes(1, "little") + UART_TUNING_FLAG + UART_TAIl)
            img.draw_line(int(cx), int(cy), int(img.width() / 2), int(img.height() / 2), color = (255, 0, 0), thickness = 3)
            img.draw_cross(int(x1+w/2), int(y1+h/2))
            img.draw_rectangle((x1,y1,w,h),thickness=2, color = (0, 255, 0))

    #print(clock.fps())
