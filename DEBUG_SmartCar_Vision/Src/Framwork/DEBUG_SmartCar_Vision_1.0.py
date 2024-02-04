# Untitled - By: G15 - 周三 10月 25 2023
import sensor, image, time, math
from machine import UART
#from pyb import LED

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # 颜色跟踪必须自动增益
sensor.set_brightness(800)  # 光线现场调试,暂时未找到合适方法规避光线影响
sensor.set_auto_whitebal(True, (0,0,0)) # 颜色跟踪必须关闭白平衡
clock = time.clock()

#green = LED(2)  # 定义一个LED2   绿灯
#blue = LED(3)   # 定义一个LED3   蓝灯
uart = UART(2, 115200)   # 端口为2号 波特率115200
threshold = [(40, 100, -128, 127, -128, 127)]
dx = 0         # 与中心点x偏差
dy = 0         # 与中心点y偏差
Rect_cx = 0    # 图片的中心x坐标
Rect_cy = 0    # 图片的中心y坐标
Image_cx = 160 # QVGA清晰度像素x中心点
Image_cy = 120 # QVGA清晰度像素y中心点

#检测摄像头正常启动蓝绿快闪
#green.on()
#time.sleep_ms(100)
#green.off()
#time.sleep_ms(100)
#blue.on()
#time.sleep_ms(100)
#blue.off()
#time.sleep_ms(100)
#white.on()

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


while(True):
    clock.tick()
    img = sensor.snapshot()
    Blob = img.find_blobs(threshold,pixels_threshold = 400, area_threshold = 400, margin = 1,merge = True, invert = 0)
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
