# 绘制图片
# 此示例显示如何在帧缓冲区中绘制图像。
import pyb
import sensor, image, time, math
import os, tf
import gc

sensor.reset()
sensor.set_pixformat(sensor.RGB565) # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA) # or QQVGA...
#sensor.set_auto_whitebal(True)
sensor.set_auto_exposure(True, 150)
sensor.skip_frames(time = 2000)
clock = time.clock()

H = sensor.height()
W = sensor.width()

net = "place1.3.tflite"
labels = [line.rstrip("\n") for line in open("/sd/label.txt")] + ["none"]

LAB_BackGround = (0, 100, -128, 127, -128, 9)
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

def getMinRect(major_axis, minor_axis):
    '''
        主轴坐标按照x坐标升序
        次轴坐标按照y坐标降序
    '''
    if major_axis[0] > major_axis[2]:
        major_axis[0], major_axis[2] = major_axis[2], major_axis[0]
        major_axis[1], major_axis[3] = major_axis[3], major_axis[1]

    if minor_axis[1] < minor_axis[3]:
        minor_axis[0], minor_axis[2] = minor_axis[2], minor_axis[0]
        minor_axis[1], minor_axis[3] = minor_axis[3], minor_axis[1]
    # 获取中心点坐标
    O = [int((major_axis[0] + major_axis[2]) / 2), int((major_axis[1] + major_axis[3]) / 2)]
    # 得到主轴向量
    Arrow = [O[0] - major_axis[0],O[1] - major_axis[1]]
    # 根据向量获取最小外接矩形角点坐标
    Pa = [minor_axis[0] - Arrow[0], minor_axis[1] - Arrow[1]]
    Pb = [minor_axis[0] + Arrow[0], minor_axis[1] + Arrow[1]]
    Pc = [minor_axis[2] - Arrow[0], minor_axis[3] - Arrow[1]]
    Pd = [minor_axis[2] + Arrow[0], minor_axis[3] + Arrow[1]]
    # 最小外接矩形边长
    length = math.sqrt((Pa[0] - Pb[0]) ** 2 + (Pa[1] - Pb[1]) ** 2)
    # 返回角点坐标(按照顺时针顺序)
    return [Pa, Pb, Pd, Pc, length]


def classify(img, minRect):
    img1 = img.copy()
    img1.rotation_corr(corners=minRect[0:4])
    inputImage = img1.copy(x_scale=0.3, y_scale=0.4)
    model = tf.load(net, load_to_fb=True)
    obj = tf.classify(model, inputImage)[0]
    tf.free_from_fb()
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    print("%s: %f" % (labels[result_index], m))
    # print(res)
    if m >= 0.85:
        result_index = res.index(m)
        print("%s: %f" % (labels[result_index], m))
    else:
        print("Unknow!")
    img.draw_image(inputImage, 0, 0)



while(True):
    clock.tick()
    img = sensor.snapshot()
    statistics=img.get_statistics()
    ListBackGround[5] = statistics.b_median()
    ListBackGround[4] = int(statistics.b_min())
    #print(ListBackGround)
    backGroundBlob = img.find_blobs([tuple(ListBackGround)],x_stride=3,y_stride=3, invert = False)
    MaxBackGroundBlob = find_max(backGroundBlob)
    if MaxBackGroundBlob:
        blobs = img.find_blobs([tuple(ListBackGround)],roi = MaxBackGroundBlob[0:4], x_stride=3, y_stride=3, invert = True, pixels_threshold=1000)
        for Border in blobs:
            minRect = getMinRect(list(Border.major_axis_line()),list(Border.minor_axis_line()))
            rate = Border.w()/Border.h()
            #print(rate, minRect)
            if rate >= 0.90 and rate <= 1.50:

                #print(rate, minRect)
                classify(img, minRect)

                img.draw_rectangle(Border.rect())
                # img.draw_edges(minRect[0:4], color = (0, 255, 0), thickness = 2)
    #img.binary([tuple(ListBackGround)], invert =0)

    #print(clock.fps())
