import pyb
import sensor, image, time, tf, gc
from machine import UART
from Decoder import *
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
sensor.set_auto_exposure(True, 120)
clock = time.clock()                # Create a clock object to track the FPS.

#设置模型路径
face_detect = '/sd/(48, 64)model_58_0.0341.tflite'
#载入模型
net = tf.load(face_detect)

while(True):
    clock.tick()
    img = sensor.snapshot()
    Decoder = target_decoder(input_size = (240, 320))
    #使用模型进行识别
    obj = tf.classify(net, img)
    for o in obj:
        coords = Decoder.decode(o.output())
        print(coords)
        for cord in coords:
            img.draw_cross(int(cord[0]), int(cord[1]))
    #print(clock.fps())
