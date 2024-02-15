import sensor, image, time
sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # 设置摄像头像素格式
sensor.set_framesize(sensor.QVGA)   # 设置摄像头分辨率
sensor.set_auto_exposure(True, 150)
#sensor.set_brightness(100)
#sensor.set_auto_whitebal(True)
sensor.skip_frames(time = 3000)

#定义一个roi区域
#cards2_roi2=[30,20,260,200]

#用于测试用的lab阈值 元组
test_tuple=(0, 100, -128, 127, -128, 9)
test_list=list(test_tuple)
#test_list即为[25, 100, -128, 127, -6, 127]


while(True):
    img = sensor.snapshot()
    his = img.get_histogram()
    per = his.get_percentile(0.85)
    statistics=img.get_statistics()
    hper = statistics.b_min() * 0.8
    test_list[5] = statistics.b_median()
    test_list[4] = int(statistics.b_min())
    the = per.b_value() - statistics.b_uq()
    #print(per.b_value())
    #print(per.b_value(), statistics.b_uq())
    #print(the)
    #print(his.get_threshold())
    print(test_list)
    blobs = img.find_blobs([tuple(test_list)],x_stride=3,y_stride=3,invert=True)
    #筛选出合适的色块
    #for b in blobs:
        ##if(b.w()>50 and b.w()<160 and b.h()>50 and b.h()<160):
        #img.draw_rectangle(b.rect(), color=(0, 0, 255), thickness=5)
    #img1=img.copy()
    #img.binary([test_list], invert =0)


