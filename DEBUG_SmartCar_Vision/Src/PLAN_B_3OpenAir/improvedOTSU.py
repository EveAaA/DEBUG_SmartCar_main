'''
@brief: 一维Otsu改进大津法追踪色块对B通道直方图做阈值分割
        传统Otsu由于冗余的计算均值, 方差做过多运算, 运用递推法
        做一次加速, 由于可以证明阈值的区间在0-直方图均值的范围,
        所以只需要遍历0-直方图均值即可进行二次加速(连电脑帧率为22帧)
        参考文献
@param: 手动传入区间范围直方图, 以及平均值的映射于0-255下标
@return: 返回阈值重新映射于-128-127
'''
def improvedOTSU(hist, mean):
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
            The = t
    return int(The / len(hist) * 256 + -128)
