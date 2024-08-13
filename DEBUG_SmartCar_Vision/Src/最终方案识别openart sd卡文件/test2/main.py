# Hello World Example
#
# Welcome to the OpenMV IDE! Click on the green run arrow button below to run the script!

import sensor, image, time
from pyb import LED
from machine import UART
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_brightness(2500)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.
white = LED(4)  # 定义一个LED4   照明灯
UART_TAIl = b"\xfc\xbf"
UART_FINISH = b'\x20'
FINE_FINISHTUNING_FLAG = b"\x01"
FINE_NOT_FINISHTUNING_FLAG = b"\x00"
threshold = [(93, 100, -128, 127, -128, 127)]
inErrTime = 0
availableErr = 3
uart = UART(2, 9600)
center = [158, 123]

while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot().gamma_corr(gamma = 0.2,contrast = 5,brightness = -4.3)         # Take a picture and return the image.
    blob = img.find_blobs(threshold, roi = (1 ,1, 319, 177), area_threshold = 1000, pixels_threshold = 500, margin = True)
    if blob:
        for b in blob:
           # img.draw_rectangle(b.rect(), color = (255, 0, 0))
            if abs(b.h() - b.w()) < 10 and b.area() < 8000:
                # print(abs(b.h() - b.w()))
                dx = center[0] - int(b.x() + b.w() / 2)
                dy = int(b.y() + b.h() / 2) - center[1]
                #img.draw_string(10, 10, "dx:"+str(dx), color = (0, 255, 0), scale = 2)
               #  img.draw_string(10, 20, "dy:"+str(dy), color = (0, 255, 0), scale = 2)
                if abs(dx) < availableErr and abs(dy) > availableErr:
                    inErrTime = 0
                    dx = 0
                elif abs(dy) < availableErr and abs(dx) > availableErr:
                    inErrTime = 0
                    dy = 0
                elif abs(dx) <= availableErr and abs(dy) <= availableErr:
                    inErrTime += 1
                else:
                    inErrTime = 0
                uart.write(dx.to_bytes(2, "little") + dy.to_bytes(2, "little") + FINE_NOT_FINISHTUNING_FLAG + UART_TAIl)
                img.draw_cross(int(b.x() + b.w() / 2), int(b.y() + b.h() / 2), color = (255, 0, 0))
                img.draw_rectangle(b.rect(), color = (0, 255, 0))
    else:
        dx = 0
        dy = 0
        uart.write(dx.to_bytes(2, "little") + dy.to_bytes(2, "little") + FINE_NOT_FINISHTUNING_FLAG + UART_TAIl)
    if inErrTime > 5:
        inErrTime = 0
        #white.on()
        img.draw_string(10, 30, "inErrTime:"+str(inErrTime), color = (0, 255, 0), scale = 3)
        uart.write(dx.to_bytes(2, "little") + dy.to_bytes(2, "little") + FINE_FINISHTUNING_FLAG + UART_TAIl)
        #time.sleep_ms(500)
       # white.off()
    img.draw_circle(center[0], center[1], availableErr, color = (255, 0, 0))
    # img.draw_rectangle([int(center[0] - availableErr), int(center[1] - availableErr), availableErr * 2, availableErr * 2], color = (255, 0, 0))
    #img.draw_string(10, 30, "inErrTime:"+str(inErrTime), color = (255, 0, 0), scale = 3)
    #img.draw_line(0, center[1], 320, center[1], color = (255, 255, 0))
    #img.draw_line(center[0], 240, center[0], 0, color = (255, 0, 0), thickness = 1)
    print(clock.fps())              # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.
