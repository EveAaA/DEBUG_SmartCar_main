# 对模型输出解算  - By: G15 - 周六 3月 2 2024
from ulab import numpy as np

class target_decoder(object):
    def __init__(self,
                 input_size: tuple = (96, 128),
                 FeatureMap_shape: tuple = (3, 4),
                 box_size=40,
                 pred_size=False,
                 match_threshold=10):
        self.input_size = input_size
        self.FeatureMap_shape = FeatureMap_shape
        assert self.input_size[0]/self.FeatureMap_shape[0] == self.input_size[1]/self.FeatureMap_shape[1]

        #相邻格子中心的距离
        self.box_d = self.input_size[0]/self.FeatureMap_shape[0]
        #box_size应比box_d略大，这样能解决边界问题
        self.box_size = box_size
        #是否预测图片尺寸
        self.pred_size = pred_size

        self.box_centers = self.get_box_centers()
        self.match_threshold = match_threshold
    def get_box_centers(self):
        box_centers = []
        for y in range(self.FeatureMap_shape[0]):
            centers = []
            for x in range(self.FeatureMap_shape[1]):
                cx = self.box_d/2+x*self.box_d-0.5
                cy = self.box_d/2+y*self.box_d-0.5
                centers.append((cx,cy))
            box_centers.append(centers)
        return box_centers
    def NMS(self,coords):
        score = coords[:,2]
        coords = coords[:,:2]
        # print(score, coords)
        order_idx = np.argsort(-score, axis = 0)#按conf值从大到小排序的下标
        # print(list(order_idx))
        remain_point = []
        while len(order_idx)>0:
            idx = order_idx[0]
            remain_point.append(list(coords[idx]))

            # 计算当前目标与剩余预测目标的x,y距离
            delta = []
            for i in order_idx:
                delta.append(max(abs(coords[i][0] - coords[idx][0]), abs(coords[i][1] - coords[idx][1])))
            # print(delta)
            idxs = []
            for i in range(len(delta)):
                if delta[i] > self.match_threshold:
                    idxs.append(order_idx[i])
            order_idx = idxs
        return remain_point

    def decode(self,label):
        label = np.array(label)
        confs = label[24:]
        coords = []
        for idx, conf in enumerate(confs):
            if conf >= 0.5:
                x_offset, y_offset = label[2 * idx]*self.box_size, label[2 * idx + 1]*self.box_size
                box_x_idx, box_y_idx = int(idx % self.FeatureMap_shape[1]), \
                    int(idx//self.FeatureMap_shape[1])
                x,y = self.box_centers[box_y_idx][box_x_idx][0]+x_offset,\
                    self.box_centers[box_y_idx][box_x_idx][1]+y_offset
                coords.append([x, y,conf])
        coords = np.array(coords)
        if len(coords):
            coords = self.NMS(coords)
        coords = list(coords)
        return coords

if __name__ == '__main__':
    label = \
        [0.03170936, 0.01161576, 0.002394535, -0.01728458, 0.001729883, -0.01523443, -0.07758833, -0.01384224,
         0.04466587, -0.03894591, 0.02919579, -0.03473084, 0.02252343, -0.03920266, -0.06710625, -0.01888932, 0.0895816,
         -0.04053132, 0.06885928, -0.02103919, 0.08269023, -0.02184934, -0.06870663, 0.01613627, 0.9999865, 0.9999944,
         0.9999947, 1.0, 0.9990911, 0.9996686, 0.9994372, 1.0, 0.9998446, 0.9998952, 0.9998863, 1.0]    #print(label)
    decoder = target_decoder()
    coords = decoder.decode(label)
    print(coords)
