# Untitled - By: G15 - 周四 2月 22 2024

import sensor, image, time
import math
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_auto_exposure(True, 120)
#sensor.set_brightness(100)
sensor.skip_frames(time = 2000)

clock = time.clock()
test_tuple=(0, 100, -128, 127, -128, -23)
test_list=list(test_tuple)
def otsu_threshold(hist):
    """
    用于归一化直方图的Otsu阈值算法。

    参数：
    hist (list)：表示为一维列表的归一化直方图。

    返回：
    int：最佳阈值。
    """
    # 初始化变量
    max_var = 0.0
    optimal_thresh = 0

    # 计算总平均值和方差
    total_mean = sum(i * hist[i] for i in range(len(hist)))
    total_var = sum((i - total_mean) ** 2 * hist[i] for i in range(len(hist)))

    # 动态规划数组
    dp = [[0] * len(hist) for _ in range(len(hist))]

    # 计算dp数组
    for t in range(len(hist)):
        for i in range(t, len(hist)):
            dp[t][i] = dp[t][i-1] + (i * hist[i])

    # 遍历所有可能的阈值
    for t in range(len(hist)):
        # 计算背景的平均值和方差
        back_mean = dp[0][t]
        back_var = sum((i - back_mean) ** 2 * hist[i] for i in range(t))

        # 计算前景的平均值和方差
        fore_mean = dp[t][len(hist)-1] - back_mean
        fore_var = sum((i - fore_mean) ** 2 * hist[i] for i in range(t, len(hist)))

        # 计算类内方差
        within_class_var = back_var + fore_var

        # 计算类间方差
        between_class_var = total_var - within_class_var

        # 计算类间方差比
        var_ratio = between_class_var / total_var

        # 如果方差比大于最大方差，则更新最佳阈值
        if var_ratio > max_var:
            max_var = var_ratio
            optimal_thresh = t

    return optimal_thresh

def newOSTU(hist, mean):
    '''
        @brief: 一维Otsu改进大津法追踪色块对B通道直方图做阈值分割
                传统Otsu由于冗余的计算均值, 方差做过多运算, 运用递推法
                做一次加速, 由于可以证明阈值的区间在0-直方图均值的范围,
                所以只需要遍历0-直方图均值即可进行二次加速(连电脑帧率为22帧)
        @param: 手动传入区间范围直方图, 以及平均值的映射于0-255下标
        @return: 返回阈值重新映射于-128-127
    '''
    Pb = hist[0]
    Ub = 0
    Pf = hist[0]
    Uf = 0
    Var = 0
    Max = 0
    The = 0
    epsilon = 1e-10 # 避免除数为0无穷小量
    for t in range(1, mean):
        Pb += hist[t]
        Pf += hist[t + 1]
        Ub = Ub + (hist[t]/(Pb+epsilon))*(t-Ub)
        Uf = Uf + (hist[t + 1] /(Pf+epsilon)) * (t-Uf)
        Var = (Pb * (1 - Pb) * (Ub - Uf) ** 2) ** 2
        if Var > Max:
            Max = Var
            The = t
    return int((The * 1.2 / len(hist) * 256 + -128))



def getAutoThreshold():
    his = img.get_histogram()
    #per = his.get_percentile(0.99)
    statistics=img.get_statistics()
    print(statistics.b_mean())
    print(his.b_bins()[(statistics.b_min()+128): (statistics.b_max()+128-1)])
    print( len(his.b_bins()[(statistics.b_min()+128): (statistics.b_max()+128-1)]))
    print(int(math.floor((statistics.b_min() + statistics.b_max()) / 2 + 128) / 256 * len(his.b_bins()[(statistics.b_min()+128): (statistics.b_max()+128-1)])))
    print(int(math.floor((statistics.b_mean()+128) / 255 * len(his.b_bins()[(statistics.b_min()+128): (statistics.b_max()+128-1)]))))
    The = newOSTU(his.b_bins()[(statistics.b_min()+128): (statistics.b_max()+128-1)], int(math.floor((statistics.b_mean()+128) / 255 * len(his.b_bins()[(statistics.b_min()+128): (statistics.b_max()+128-1)]))))
    #print(The)
    test_list[5] = The
    print(test_list)

while(True):
    clock.tick()
    img = sensor.snapshot()
    getAutoThreshold()
    #img.binary([test_list])
    print(clock.avg())
