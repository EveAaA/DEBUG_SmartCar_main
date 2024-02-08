# 绘制图片
# 此示例显示如何在帧缓冲区中绘制图像。
import pyb
import sensor, image, time, math
import os, tf
import gc

sensor.reset()
sensor.set_pixformat(sensor.RGB565) # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA) # or QQVGA...
sensor.skip_frames(time = 2000)
#sensor.set_auto_whitebal(True)
sensor.set_brightness(150)
clock = time.clock()

H = sensor.height()
W = sensor.width()

net = "place.tflite"
labels = [line.rstrip("\n") for line in open("/sd/label.txt")] + ["none"]

NET_SIZE = 96
BORDER_WIDTH = (NET_SIZE + 9) // 10

NET_SIZE = 96
BORDER_WIDTH = (NET_SIZE + 9) // 10
CROP_SIZE = NET_SIZE + BORDER_WIDTH * 2
ROI = (112, 72, 96, 96)

BORDER_REGION = 80
BORDER_ROI = ((W - BORDER_REGION) >> 1, (H - BORDER_REGION) >> 1, BORDER_REGION, BORDER_REGION)
BORDER_PIXEL_MIN = 1000
BORDER_THRESHOLD = [(76, 100, -128, 127, -128, 127)]

LAB_BackGround = (0, 100, -128, 127, -128, -30)
ListBackGround = list(LAB_BackGround)

def find_max(blobs):
    if not blobs:  # 如果不存在与色域相符色块处理
        return None
    max_blob = None
    max_size = 0
    for blob in blobs:
        if blob[2] * blob[3] > max_size:
            max_blob = blob
            max_size = blob[2] * blob[3]
    return max_blob


while(True):
    clock.tick()
    model = tf.load(net, load_to_fb=True)
    img = sensor.snapshot()
    #BorderBlob = img.find_blobs(BORDER_THRESHOLD)
    #Border = find_max(BorderBlob)
    backGroundBlob = img.find_blobs([tuple(ListBackGround)],x_stride=3,y_stride=3, invert = False)
    MaxBackGroundBlob = find_max(backGroundBlob)
    if MaxBackGroundBlob:
        blobs = img.find_blobs([tuple(ListBackGround)],roi = MaxBackGroundBlob[0:4], x_stride=3,y_stride=3, invert = True)
        Border = find_max(blobs)
        corners = ((Border.x(), Border.y()), (Border.x() + Border.w(), Border.y()),  (Border.x() + Border.w(), Border.y() + Border.h()),  (Border.x(), Border.y() + Border.h()))
        for corner in Border.corners():
            img.draw_cross(corner)
        #print(Border.density())
        img1 = img.copy()

        if Border.density() > 0.86 and Border.density() < 1.0:
            img1.rotation_corr(corners=corners)
        else:
            img1.rotation_corr(corners=Border.corners())
        small_img = img1.mean_pooled(2, 2) # Makes a copy.
        img.draw_image(small_img, 0, 0, x_scale=1, y_scale=1)
        img.draw_rectangle(Border.rect())
        img.draw_rectangle(ROI)
        #obj = tf.classify(model, img1)[0]
        obj = tf.classify(model, img, roi = ROI)[0]
        res = obj.output()
        m = max(res)
        result_index = res.index(m)
        if m >= 0.85:
            result_index = res.index(m)
            print("%s: %f" % (labels[result_index], m))
        else:
            print("Unknow!")
    tf.free_from_fb()
    #print(clock.fps())
