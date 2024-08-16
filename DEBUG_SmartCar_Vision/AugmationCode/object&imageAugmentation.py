# 导入 opencv �? os �?
import cv2
import os
import numpy as np
from tqdm import tqdm
from aug_utils import *
# from tf_augmentation import *
import xml.etree.ElementTree as ET
import random
import concurrent.futures
# bg_img = cv2.imread("background.jpg")
# bg_img = cv2.resize(bg_img, (640, 480))

# Constants
BRIGHTNESS_DELTA = (-60, 60)
CONTRAST_RANGE = (0.4, 1.7)
SATURATION_RANGE = (0.4, 1.6)
COLOR_SHIFT = 30.0

def generate_blob(x_value_rate, centers, dev):
    N, M = x_value_rate.shape
    for u, v in centers:
        for i in range(N):
            for j in range(M):
                di, dj = i - u, j - v
                d = np.sqrt(di * di + dj * dj)
                x_value_rate[i, j] += np.exp(-d ** 2 / (2 * dev ** 2)) / (dev * np.sqrt(2 * np.pi))
    return x_value_rate

def apply_blur(img, sigma):
    ksize = int(6*sigma + 1) | 1
    return cv2.GaussianBlur(img, (ksize, ksize), sigma)

def random_mask(image, mask, cnt=1):
    N, M, _ = image.shape
    blob = np.zeros((N, M))
    for _ in range(cnt):
        i = random.randint(0, N)
        j = random.randint(0, M)
        r2 = random.randint(30, min(M, N) // (2 * cnt))
        blob = generate_blob(blob, [[i, j]], r2)
    blob = np.repeat(blob[:, :, np.newaxis], 3, axis=2)
    return mask * blob + image * (1.0 - blob)

def random_blur(image):
    sigma = random.uniform(0.5, 2)
    return apply_blur(image, sigma)

def random_blur_mask(image):
    mask = random_blur(image)
    return random_mask(image, mask)

def random_brightness(image):
    delta = random.uniform(BRIGHTNESS_DELTA[0], BRIGHTNESS_DELTA[1])
    return np.clip(image + delta, 0, 255)

def random_brightness_mask(image):
    mask = random_brightness(image)
    return random_mask(image, mask)

def random_contrast(image):
    factor = random.uniform(CONTRAST_RANGE[0], CONTRAST_RANGE[1])
    return np.clip(128 + factor * (image - 128), 0, 255)

def random_contrast_mask(image):
    mask = random_contrast(image)
    return random_mask(image, mask)

def random_saturation(image):
    image = image.astype(np.float32)  # �?保图像是�?点类�?
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    hsv[:, :, 1] = np.clip(hsv[:, :, 1] * random.uniform(SATURATION_RANGE[0], SATURATION_RANGE[1]), 0, 255)
    return cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
def random_saturation_mask(image):
    mask = random_saturation(image)
    return random_mask(image, mask)

def random_color(image):
    shift = np.array([random.uniform(-COLOR_SHIFT, COLOR_SHIFT) for _ in range(3)])
    return np.clip(image + shift, 0, 255)

def random_color_mask(image):
    mask = random_color(image)
    return random_mask(image, mask)

def random_rotate(image):
    angle = random.randint(0, 360)
    h, w = image.shape[:2]
    M = cv2.getRotationMatrix2D((w//2, h//2), angle, 1)
    return cv2.warpAffine(image, M, (w, h), borderValue = (192, 78, 65))

def random_scaling_mask(image):
    h, w = image.shape[:2]
    small = cv2.resize(image, (w//2, h//2), interpolation=cv2.INTER_AREA)
    inter = cv2.resize(small, (w, h), interpolation=cv2.INTER_LINEAR)
    return random_mask(image, inter)

def random_crop(image):
    h, w = image.shape[:2]
    scale = random.uniform(0.85, 1)
    new_h, new_w = int(h * scale), int(w * scale)
    start_x = random.randint(0, h - new_h)
    start_y = random.randint(0, w - new_w)
    cropped = image[start_x:start_x + new_h, start_y:start_y + new_w]
    return cv2.resize(cropped, (w, h), interpolation=cv2.INTER_LINEAR)

def rotation_corr(image):
    width = image.shape[1]
    height = image.shape[0]
    pts1 = np.float32([[0, 0], [width, 0], [width, height], [0, height]])
    pts2 = np.float32([[0, 0], [320, 0], [320, 240], [0, 240]])
    M = cv2.getPerspectiveTransform(pts1, pts2)
    dst = cv2.warpPerspective(image, M, (320, 240))
    return dst

def augment(image):
    image = image.astype(np.float32)
    image = random_rotate(image)
    image = random_color(image)
    image = random_brightness(image)
    image = random_contrast(image)

    # image = random_color_mask(image)
    image = random_brightness_mask(image)
    image = random_contrast_mask(image)
    image = random_saturation_mask(image)
    image = random_blur_mask(image)

    image = random_crop(image)
    image = random_scaling_mask(image)

    image = np.clip(image, 0, 255).astype(np.uint8)
    #image  = cv2.resize(image, (320, 240))
    image = rotation_corr(image)
    return image




def get_mask(img):
    r, w, _ = img.shape
    img_mask = img.copy()
    img_mask = cv2.cvtColor(img_mask,cv2.COLOR_BGR2GRAY)
    _,mask = cv2.threshold(img_mask,80,255,cv2.THRESH_BINARY_INV)
    mask = cv2.resize(mask,(w,r))
    return mask

# 定义一�?函数 augment_image，用于�?�一张图片进行数�?增强
def augment_image(image):
    global bg_img
    image = cv2.resize(image, (320, 240))
    # 获取图片的�?�度和高�?
    width = image.shape[1]
    height = image.shape[0]


    # 随机生成一些数�?增强的参�?
    angle = np.random.randint(0, 360) # 旋转角度
    tx = np.random.randint(-50, 50) # 水平平移距�??
    ty = np.random.randint(-50, 50) # 垂直平移距�??
    channel = np.random.randint(1, 3)
    scale = np.random.uniform(1.0, 1.0) # 缩放比例
    crop_x = np.random.randint(0, int(width * 0.1)) # 裁剪左上角的 x 坐标
    crop_y = np.random.randint(0, int(height * 0.1)) # 裁剪左上角的 y 坐标
    crop_w = np.random.randint(int(width * 0.8), width - crop_x) # 裁剪的�?�度
    crop_h = np.random.randint(int(height * 0.8), height - crop_y) # 裁剪的高�?
    flip = np.random.choice([0, 1, -1]) # 翻转的方向，0 为水平，1 为垂直，-1 为水平和垂直
    blur = np.random.randint(2, 5) # 模糊的半�?
    noise = np.random.randint(2, 150) # �?声的强度
    hue = np.random.randint(-4, 4) # 色调的偏�?
    saturation = np.random.uniform(1, 5) # 饱和度的比例
    value = np.random.uniform(0.3, 4) # �?度的比例
    shear_matrix = np.array([[1, 0.2, 0], [0, 1, 0]], dtype=np.float32)
    # # 设置添加椒盐�?声的数目比例
    s_vs_p = np.random.uniform(0, 0.40)
    # # 设置添加�?声图像像素的数目
    amount = np.random.uniform(0, 0.05)
    image = np.copy(image)
    # # # 添加salt�?�?
    # num_salt = np.ceil(amount * image.size * s_vs_p)
    # # # 设置添加�?声的坐标位置
    # coords = [np.random.randint(0, i - 1, int(num_salt)) for i in image.shape]
    # image[coords[0], coords[1], :] = [255, 255, 255]
    # # # 添加pepper�?�?
    # num_pepper = np.ceil(amount * image.size * (1. - s_vs_p))
    # # # 设置添加�?声的坐标位置
    # coords = [np.random.randint(0, i - 1, int(num_pepper)) for i in image.shape]
    # image[coords[0], coords[1], :] = [0, 0, 0]
    #
    # 对图片进行模�?
    image = cv2.blur(image, (blur, blur))
    # 对图片添加噪
    noise_image = np.zeros((240, 320, 1), np.uint8)
    cv2.bitwise_not(noise_image, noise_image)
    cv2.randn(noise_image, 0, noise)
    # cv2.bitwise_not(noise_image, noise_image)
    image[:, :, 0] = cv2.bitwise_xor(image[:, :, 0], noise_image)
    image[:, :, 1] = cv2.bitwise_xor(image[:, :, 1], noise_image)
    image[:, :, 2] = cv2.bitwise_xor(image[:, :, 2], noise_image)
    for i in range(0, channel):
        image[:, :, i] = cv2.bitwise_xor(image[:, :, i], noise_image)
    # 对图片进行色彩变�?
    image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV) # �?换到 HSV 空间
    image[:, :, 0] = (image[:, :, 0] + hue) % 180 # 调整色调
    image[:, :, 1] = np.clip(image[:, :, 1] * saturation, 0, 255) # 调整饱和�?
    image[:, :, 2] = np.clip(image[:, :, 2] * value, 0, 255) # 调整�?�?
    image = cv2.cvtColor(image, cv2.COLOR_HSV2BGR) # �?换回 BGR 空间

    # 对图片进行错�?
    image = cv2.warpAffine(image, shear_matrix, (width, height))
    # 创建一�?旋转矩阵
    rotation_matrix = cv2.getRotationMatrix2D((width / 2, height / 2), angle, 1.0)
    # 对图片进行仿射变�?，即旋转，平移和缩放
    image = cv2.warpAffine(image, rotation_matrix, (width, height), borderValue = (192, 78, 65))

    # 对图片进行�?�剪
    image = image[crop_y:crop_y + crop_h, crop_x:crop_x + crop_w]

    # 对图片进行翻�?
    image = cv2.flip(image, flip)
    image = cv2.resize(image, (320, 240))
    # width = image.shape[1]
    # height = image.shape[0]
    # #print(width, height)
    # img = np.zeros((240, 320, 3), dtype=np.uint8)
    # img[0:height, 0:width] = image
    # image = cv2.resize(image, (96, 96))
    # image = cv2.add(bg_img, image)
    # 返回增强后的图片
    return image

def Object_augment_image(image):
    # 获取图片的�?�度和高�?
    width = image.shape[1]
    height = image.shape[0]

    # 随机生成一些数�?增强的参�?
    blur = np.random.randint(1, 5) # 模糊的半�?
    noise = np.random.randint(2, 10) # �?声的强度
    hue = np.random.randint(-15, 15) # 色调的偏�?
    saturation = np.random.uniform(0.8, 2.0) # 饱和度的比例
    value = np.random.uniform(0.5, 2.0) # �?度的比例
    red_increase = np.random.uniform(-50, 50)
    yellow_increase = np.random.uniform(-50, 50)

    # red_channel = image[:, :, 2]
    # red_channel = red_channel +  red_increase
    # red_channel[red_channel > 255] = 255
    #
    # yellow_channel = image[:, :, 1]
    # yellow_channel = yellow_channel + yellow_increase
    # yellow_channel[yellow_channel > 255] = 255
    image = np.copy(image)
    # 对图片进行色彩变�?
    image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV) # �?换到 HSV 空间
    image[:, :, 0] = (image[:, :, 0] + hue) % 180 # 调整色调
    image[:, :, 1] = np.clip(image[:, :, 1] * saturation, 0, 255) # 调整饱和�?
    image[:, :, 2] = np.clip(image[:, :, 2] * value, 0, 255) # 调整�?�?
    image = cv2.cvtColor(image, cv2.COLOR_HSV2BGR) # �?换回 BGR 空间

    # 对图片进行模�?
    image = cv2.blur(image, (blur, blur))

    # 对图片添加噪�?
    noise_image = np.zeros(image.shape, np.uint8)
    cv2.randn(noise_image, 0, noise)
    image = cv2.add(image, noise_image)
    # 返回增强后的图片
    return image

def augument_image2(image):
    image = random_aspect(image)
    image = random_size(image)
    image = random_crop(image)
    image = random_flip(image)
    image = random_hsv(image)
    image = random_pca(image)
    return image

# 定义一�?函数 save_image，用于将图片保存到指定的文件夹和文件�?
def save_image(image, folder, filename):
    # 创建文件夹，如果不存在的�?
    if not os.path.exists(folder):
        os.makedirs(folder)
    # 拼接文件�?�?
    filepath = os.path.join(folder, filename)
    # 保存图片
    cv2.imwrite(filepath, image)

def save_annoation(annotation, folder, filename):
    # 创建文件夹，如果不存在的�?
    if not os.path.exists(folder):
        os.makedirs(folder)
    filepath = os.path.join(folder, filename)
    with open(filepath, "w") as fp:
        for msg in annotation:
            fp.write(msg)
        fp.close()

def process_image(folder, filename, root_folder, save_folder, num_aug):
    """处理单张图片，包括读取、增强和保存"""
    # 读取图片
    image = cv2.imread(os.path.join(root_folder, folder, filename))
    image = augment(image)
    save_image(image, os.path.join(save_folder, folder), filename)

    # 对图片进行数据增强 num_aug 次
    for i in range(num_aug):
        augmented_image = augment(image)
        name, ext = os.path.splitext(filename)
        new_filename = name + "_aug_" + str(i + 1) + ext
        save_image(augmented_image, os.path.join(save_folder, folder), new_filename)


def main():
    root_folder = "C:\\Users\\G15\\Desktop\\test_openart"  # 原始图片所在的根文件夹
    save_folder = "D:\\aaaaWordSpace\\Embedded_Development\\DEBUG_Visual_Part\\Debug_Tensorflow_GPU_2.9\\class\\openartAug"
    num_aug = 10  # 每张图片要增强的次数

    # 遍历文件夹里的所有子文件夹
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for folder in tqdm(os.listdir(root_folder)):  # tqdm 包装一层循环
            folder_path = os.path.join(root_folder, folder)
            if os.path.isdir(folder_path):
                for filename in tqdm(os.listdir(folder_path)):  # tqdm 包装二层循环
                    futures.append(executor.submit(process_image, folder, filename, root_folder, save_folder, num_aug))

        # 等待所有线程完成
        for future in concurrent.futures.as_completed(futures):
            future.result()

# 遍历文件夹里的所有子文件�?
root_folder = "C:\\Users\\G15\\Desktop\\test_openart" # 原�?�图片所在的根文件夹
object_folder = "D:\\object_file\\yolo3_smartcar\\store\\2024_7_25"
save_folder = "D:\\aaaaWordSpace\\Embedded_Development\\DEBUG_Visual_Part\\Debug_Tensorflow_GPU_2.9\\class\\openartAug"
num_aug = 10 # 每张图片要�?�强的�?�数
imgNum = 0
mod = 0
# 对识�?图像的数�?增强
if mod == 0:
    main()
    # for folder in tqdm(os.listdir(root_folder)): # �? tqdm 包�?��??一层循�?
    #     # 遍历子文件夹里的所有图�?
    #     imgNum = 0
    #     for filename in tqdm(os.listdir(os.path.join(root_folder, folder))): # �? tqdm 包�?��??二层�?�?
    #         # 读取图片
    #         imgNum += 1
    #         image = cv2.imread(os.path.join(root_folder, folder, filename))
    #         image = augment(image)
    #         # image = rotation_corr(image)
    #         save_image(image, os.path.join(save_folder, folder), filename)
    #         # 对图片进行数�?增强 num_aug �?
    #         for i in range(num_aug):
    #             # 对图片进行数�?增强
    #             augmented_image = augment(image)
    #             # augmented_image = rotation_corr(augmented_image)
    #             # augmented_image = image
    #             # 生成新的文件名，格式�? 原文件名_aug_序号.原扩展名
    #             name, ext = os.path.splitext(filename)
    #             new_filename = name + "_aug_" + str(i + 1) + ext
    #             # 将�?�强后的图片保存到当前子文件夹和文件�?
    #             save_image(augmented_image, os.path.join(save_folder, folder), new_filename)
    #         #if imgNum == 3:
    #         # break
# 对目标�?�测图像的数据增强
else:
    folderAnnotations = os.listdir(object_folder)[0]
    folderImages = os.listdir(object_folder)[1]
    print(folderAnnotations, folderImages)
    # 遍历图像数据集�?�图像数�?增强
    for filename in tqdm(os.listdir(os.path.join(object_folder, folderImages))):
        global Annotation
        global AnnotationAdd
        name = filename.split(".")[0] # 获取图片名称
        # 获取对应的xml文件
        # ImageFlag = False
        for file in os.listdir(os.path.join(object_folder, folderAnnotations)):
            if file.split(".")[0] == name:
                # 获取xml名称
                AnnotationAdd = file
                # 获取xml对应的数�?存入变量
                Annotation = open(os.path.join(object_folder, folderAnnotations, file)).readlines()
                # ImageFlag = True
                # print(Annotation)
                break
        image = cv2.imread(os.path.join(object_folder, folderImages, filename))
        # if ImageFlag:
        #     image = cv2.imread(os.path.join(object_folder, folderImages, filename))
        #     save_image(image, os.path.join(save_folder, folderImages), filename)
        #     save_annoation(Annotation, os.path.join(save_folder, folderAnnotations), AnnotationAdd)

        for i in range(num_aug):
            augmented_image = Object_augment_image(image)
            # 生成新的文件名，格式�? 原文件名_aug_序号.原扩展名
            name, ext = os.path.splitext(filename)
            annName, ext2 = os.path.splitext(AnnotationAdd)
            # print(annName, ext2)
            new_filename = name + "_aug_" + str(i + 1) + ext
            new_AnnotationName = annName + "_aug_" + str(i+1) + ext2
            # print(new_AnnotationName)
            save_image(augmented_image, os.path.join(save_folder, folderImages), new_filename)
            save_annoation(Annotation, os.path.join(save_folder, folderAnnotations), new_AnnotationName)
        # print(name, Annotation)






