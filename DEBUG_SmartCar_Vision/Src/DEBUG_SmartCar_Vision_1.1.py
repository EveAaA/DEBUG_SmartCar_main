# DEBUG_SmartCar_Vision - By: G15 - 周三 10月 25 2023
import pyb
import sensor, image, time, math
import os, tf
from machine import UART
from pyb import LED

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # 颜色跟踪必须自动增益
sensor.set_brightness(800)  # 光线现场调试,暂时未找到合适方法规避光线影响
sensor.set_auto_whitebal(True, (0,0,0)) # 颜色跟踪必须关闭白平衡
clock = time.clock()

uart = UART(2, 115200)   # 端口为2号 波特率115200
threshold = [(56, 100, -128, 127, -128, 127)]
dx = 0         # 与中心点x偏差
dy = 0         # 与中心点y偏差
Rect_cx = 0    # 图片的中心x坐标
Rect_cy = 0    # 图片的中心y坐标
Image_cx = 160 # QVGA清晰度像素x中心点
Image_cy = 120 # QVGA清晰度像素y中心点

################################################################################
#brief:识别环岛内带边框目标板
#return:None
################################################################################
# 寻找最大目标色块
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

def findBorder():
    while(True):
        clock.tick()
        img = sensor.snapshot()
        Blob = img.find_blobs(threshold,pixels_threshold = 400, area_threshold = 400, margin = 1,merge = True, invert = 0)
        for b in Blob:
            img.draw_edges(b.corners(), color=(255, 0, 0), thickness=2)
        Max_blob = find_max(Blob)
        # 最大色块存在则画出矩形
        if Max_blob:
            img.draw_rectangle(Max_blob[0:4], color = (0, 255, 0), thickness = 2)
            Rect_cx = int(Max_blob[0] + Max_blob[2] / 2)
            Rect_cy = int(Max_blob[1] + Max_blob[3] / 2)
            #这里添加后续模型训练后的代码（为了防止对原图像画图操作后，对图像内容识别进行干扰）
            #
            #
            #
            #
            #################################################################################
            img.draw_cross(Image_cx, Image_cy, color = (255, 0, 0))
            img.draw_cross(Rect_cx, Rect_cy, color = (0, 255, 0), size = 5)
            img.draw_line(Rect_cx, Rect_cy, Image_cx, Image_cy)
            img.draw_string(Max_blob[0], Max_blob[1]-20, '('+str(Rect_cx)+','+str(Rect_cy)+')', color = (255, 0, 0), scale = 2)
            img.draw_string(0, 0, "distance:"+str(math.sqrt((Image_cx - Rect_cx) ** 2 + (Image_cy - Rect_cy) ** 2)), scale = 2, color = (255, 0, 0))
        print(clock.fps())


################################################################################
#brief:寻找摆放地点
#return:None
################################################################################
NET_SIZE = 128
BORDER_WIDTH = (NET_SIZE + 9) // 10
CROP_SIZE = NET_SIZE + BORDER_WIDTH * 2
def findTargetPlace():
    corners = []
    net = "TargetPlace_v1.0.tflite"
    labels = [line.rstrip("\n") for line in open("/sd/labels.txt")] + ["none"]
    model = tf.load(net, load_to_fb=True)
    while True:
        clock.tick()
        img = sensor.snapshot()
        Blob = img.find_blobs(threshold, pixels_threshold = 400, area_threshold = 400, margin = 1,merge = True, invert = 0)
        print(Blob)
        Max_blob = find_max(Blob)
        # 最大色块存在则画出矩形
        if Max_blob:
            img.draw_rectangle(Max_blob[0:4], color = (0, 255, 0), thickness = 2)
            corners.append([Max_blob[0], Max_blob[1]])
            corners.append([Max_blob[0] + Max_blob[2], Max_blob[1]])
            corners.append([Max_blob[0] + Max_blob[2], Max_blob[1] + Max_blob[3]])
            corners.append([Max_blob[0], Max_blob[1] + Max_blob[3]])
            for index in corners:
                 img.draw_cross(index[0], index[1])
            img.rotation_corr(corners = corners)
            img1 = img.copy(1, 1, Max_blob.rect())
            corners.clear()
            for obj in tf.classify(net , img1, min_scale=1.0, scale_mul=0.5, x_overlap=0.0, y_overlap=0.0):
                print("**********\nTop 1 Detections at [x=%d,y=%d,w=%d,h=%d]" % obj.rect())
                sorted_list = sorted(zip(labels, obj.output()), key = lambda x: x[1], reverse = True)
                print("%s = %f" % (sorted_list[0][0], sorted_list[0][1]))


            #img.draw_image(img, 0, 0, x_scale = 1, y_scale = 1)

        print(clock.fps())


################################################################################
#brief:寻找散落在道路边的目标板
#return:None
################################################################################


def findBesideRoadTarget():
    object_detect = '/sd/yolo_v1.1.tflite'
    object_net = tf.load(object_detect)
    # 以图像中线为对称轴, 将两侧的图像分别存放在下述数组当中
    object_coordinate_left = []    #存放目标板坐标, 并通过y坐标判断最近的目标板
    object_coordinate_right = []
    while True:
        img = sensor.snapshot()
        # 使用模型进行识别
        for obj in tf.detect(object_net,img):
            x1,y1,x2,y2,label,scores = obj
            if(scores>0.70):
                print(obj)
                w = x2- x1
                h = y2 - y1
                x1 = int((x1-0.1)*img.width())
                y1 = int(y1*img.height())
                w = int(w*img.width())
                h = int(h*img.height())
                targetCx = int(x1 + w/2)
                targetCy = int(y1 + h/2)
                img.draw_rectangle((x1,y1,w,h),thickness=3,color=(0, 255, 0))
                img.draw_cross(targetCx, targetCy)
                # 根据图像中心x=160中线区别不同目标板坐标
                if targetCx < Image_cx:
                    object_coordinate_left.append([targetCx, targetCy])
                else:
                    object_coordinate_right.append([targetCx, targetCy])
        # 获取列表当中最近的点
        leastY_left = max(object_coordinate_left, key=lambda x: x[1])
        leastY_right = max(object_coordinate_right, key=lambda x: x[1])
        # 简单决策
        if leastY_left[1] < leastY_right[1]:
            dx = leastY_left[0] - Image_cx
            print("左侧位置dx:",dx)
            uart.write(dx.to_bytes(1, "little"))
        else:
            dx = leastY_right[0] - Image_cx
            print("右侧位置dx:",dx)
            uart.write(dx.to_bytes(1, "little"))
        #对列表进行清空
        object_coordinate_left.clear()
        object_coordinate_right.clear()










findBorder()

