import sensor, image, time
from machine import UART
import time
import pyb
from pyb import LED
from machine import Pin
import openmv_numpy as np
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)#160,120 320,240
sensor.set_auto_exposure(True, 100)
sensor.skip_frames(time = 2000)

LAB_BackGround =(0, 100, -128, 127, -128, -23)
ListBackGround = list(LAB_BackGround)

martrixH = [[2.068966, -0.0, -2.068966], [1.233199e-07, 2.142857, 2.142857], [1.027666e-09, -0.0, 1]]

#A4纸参数
#可以换成其它尺寸
a4_w = 120
a4_h = 120

world_coordinates = [[0, a4_h], [a4_w, a4_h], [a4_w, 0], [0, 0]]
# world_coordinates = [[(-a4_w/2),0],[a4_w/2,0],[(a4_w/2),a4_h],[(-a4_w/2),a4_h]]

#返回透视矩阵
#XY为世界坐标，UV为相机坐标
def cal_mtx(UV:np.array,XY:np.array)->np.array:
    A = []
    B =[]
    for i in range(4):
        a = [[UV[i][0],UV[i][1],1,0,0,0,-XY[i][0]*UV[i][0],-XY[i][0]*UV[i][1]],
             [0,0,0,UV[i][0],UV[i][1],1,-XY[i][1]*UV[i][0],-XY[i][1]*UV[i][1]]]
        B+= [[XY[i][0]],
             [XY[i][1]]]
        A+=a

    A = np.array(A)
    B = np.array(B)

    x= np.solve(A,B)

    H = [[x[0][0], x[1][0], x[2][0]],
         [x[3][0], x[4][0], x[5][0]],
         [x[6][0], x[7][0], 1]]

    return np.array(H)


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
def newOSTU(hist, minIndex, maxIndex):
    if maxIndex >= 0:
        Pb = hist[0]
        Ub = 0
        Pf = 1.0
        Uf = 0
        Var = 0
        Max = 0
        The = 0
        sb = 0
        sf = 0
        Sum = sum((hist[i]*i) for i in range(len(hist)))
        epsilon = 1e-10
        for t in range(1, len(hist) - 1):
            Pb += hist[t]
            Pf -= hist[t]
            sb +=  t * hist[t]
            sf =  Sum - sb
            Ub = sb / (Pb + epsilon)
            Uf = sf / (Pf + epsilon)
            Var = (Pb * (1 - Pb) * (Ub - Uf) ** 2) ** 2
            #print(Var)
            if Var > Max:
                Max = Var
                The = t
        return The + minIndex
    else:
        return 0

show = False

while(True):
    img = sensor.snapshot()
    #statistics=img.get_statistics()
    #his = img.get_histogram()
    ##ListBackGround[5] = newOSTU((his.b_bins()[(statistics.b_min()+128): (statistics.b_max()+128-1)]), statistics.b_min(), statistics.b_max())
    ##backGroundBlob = img.find_blobs([tuple(ListBackGround)],x_stride=5, y_stride=5, invert = False)
    ##backGroundBlob = img.find_blobs([tuple(ListBackGround)],x_stride=5, y_stride=5, invert = False)
    ##MaxBackGroundBlob = find_max(backGroundBlob)
    blobs = img.find_blobs([tuple(ListBackGround)], x_stride=1, y_stride=1, invert = True, pixels_threshold=800)
    img.draw_line(0, 153, 320, 153, color = (255, 255, 0))
    img.draw_line(162, 240, 162, 0, color = (255, 0, 0), thickness = 1)
    # img.draw_rectangle(MaxBackGroundBlob.rect(), color = (255, 255, 0))
    for r in blobs:
        # img.draw_rectangle(r.rect(), color = (255, 0, 0))
        if r.area() < 6000 and (r.w() / r.h()) <= 1.05:
            img.draw_rectangle(r.rect(), color = (255, 0, 0))
            img_coordinate=[]
            if show:
                print("********")
                point = [[r.x(), r.y()], [r.x() + r.w(), r.y()], [r.x() + r.w(), r.y() + r.h()], [r.x(), r.y() + r.h()]]
                for p in point:
                    img.draw_circle(p[0], p[1], 2, color = (0, 255, 0))
                    img_coordinate.append([p[0] - 162, 153 - p[1]])
                    print(p[0]-150, 137 - p[1])
                dn_cx = (img_coordinate[0][0]+img_coordinate[1][0])/2
                dn_cy = (img_coordinate[1][0]+img_coordinate[1][1])/2
                up_cx = (img_coordinate[2][0]+img_coordinate[3][0])/2
                up_cy = (img_coordinate[2][1]+img_coordinate[3][1])/2
                if abs(img_coordinate[3][0]) < 2 and abs(img_coordinate[3][1]) < 2:
                    img_coordinate =np.array(img_coordinate)
                    world_coordinates =np.array(world_coordinates)
                    H = cal_mtx(img_coordinate,world_coordinates)
                    print(H)
            else:
                center = [(r.x() + r.w() / 2) - 162,153 - (r.y() + r.h() / 2), 1]
                img.draw_cross(int(center[0] + 162), int(153 - center[1]), color = (0, 255, 255))
                img.draw_line(int(center[0] + 162), 0, int(center[0] + 162), 240, color = (0, 255, 0))
                img.draw_line(0, int(153 - center[1]), 320, int(153 - center[1]), color = (0, 255, 0))
                np_center = np.array([[p] for p in center])#升维
                np_H = np.array(martrixH)
                Point = np_H * np_center
                print(int(Point[0][0]), int(Point[1][0]))
                print(int(Point[0][0]).to_bytes(2, "little"), int(Point[1][0]).to_bytes(2, "little"))
