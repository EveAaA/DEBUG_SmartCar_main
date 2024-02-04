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
Image_cx = 160 # QVGA清晰度像素x中心点
Image_cy = 120 # QVGA清晰度像素y中心点
leastY_left = []
leastY_right = []


object_detect = '/sd/yolo_v1.2.tflite'
object_net = tf.load(object_detect)
# 以图像中线为对称轴, 将两侧的图像分别存放在下述数组当中
object_coordinate_left = []    #存放目标板坐标, 并通过y坐标判断最近的目标板
object_coordinate_right = []

#限幅防止超过传输数据阈值, char类型范围-128-127之间
def limit(num):
    if num >= 127:
        return 127
    elif num <= -128:
        return -128
    else:
        return num

def findBesideRoadTarget():
    global leastY_left
    global leastY_right
    while True:
        img = sensor.snapshot()
        # 绘制图像中心线
        img.draw_line(160, 0, 160, 320, color=(255, 0, 0))
        # 使用模型进行识别
        for obj in tf.detect(object_net,img):
            x1,y1,x2,y2,label,scores = obj
            if(scores>0.70):
                #print(obj)
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
        # 将找到左侧所有目标当中距离最近的目标
        if object_coordinate_left:
            # 获取列表当中最近的点
            leastY_left = max(object_coordinate_left, key=lambda x: x[1])
        # 将找到右侧所有目标当中距离最近的目标
        if object_coordinate_right:
            leastY_right = max(object_coordinate_right, key=lambda x: x[1])
        # 简单决策
        # 若赛道旁两边都存在图片
        if leastY_left and leastY_right:
            #根据左右两边目标板获取最近项
            if leastY_left[1] >= leastY_right[1]:
                dx = limit(leastY_left[0] - Image_cx)
                img.draw_line(leastY_left[0], leastY_left[1], 160, leastY_left[1], color=(255, 0, 0))
                print("左侧位置dx:",dx)
                uart.write(dx.to_bytes(1, "little"))
            else:
                dx = limit(leastY_right[0] - Image_cx)
                img.draw_line(leastY_right[0], leastY_right[1], 160, leastY_right[1], color=(255, 0, 0))
                print("右侧位置dx:",dx)
                uart.write(dx.to_bytes(1, "little"))
        # 若仅有赛道右边存在图片
        elif leastY_right:
            dx = limit(leastY_right[0] - Image_cx)
            img.draw_line(leastY_right[0], leastY_right[1], 160, leastY_right[1], color=(255, 0, 0))
            print("dx_right:",dx)
            uart.write(dx.to_bytes(1, "little"))
        # 若仅有赛道左边存在图片
        elif leastY_left:
            dx = limit(leastY_left[0] - Image_cx)
            img.draw_line(leastY_left[0], leastY_left[1], 160, leastY_left[1], color=(255, 0, 0))
            print("dx_left:",dx)
            uart.write(dx.to_bytes(1, "little"))
        # 若都不存在
        else:
            print("None")
            continue
        # 对列表进行清空
        leastY_left.clear()
        leastY_right.clear()
        object_coordinate_left.clear()
        object_coordinate_right.clear()


findBesideRoadTarget()
