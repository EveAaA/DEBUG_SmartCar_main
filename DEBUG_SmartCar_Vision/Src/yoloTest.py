import seekfree, pyb
import sensor, image, time, tf, gc

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

#设置模型路径
face_detect = '/sd/yolo_v1.1.tflite'
#载入模型
net = tf.load(face_detect)

while(True):
    clock.tick()
    img = sensor.snapshot()

    #使用模型进行识别
    for obj in tf.detect(net,img):
        x1,y1,x2,y2,label,scores = obj

        if(scores>0.70):
            print(obj)
            w = x2- x1
            h = y2 - y1
            x1 = int((x1-0.1)*img.width())
            y1 = int(y1*img.height())
            w = int(w*img.width())
            h = int(h*img.height())
            img.draw_rectangle((x1,y1,w,h),thickness=2)

    print(clock.fps())
