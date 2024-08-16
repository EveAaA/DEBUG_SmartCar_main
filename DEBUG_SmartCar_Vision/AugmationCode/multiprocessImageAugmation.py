import cv2
import os
import numpy as np
from tqdm import tqdm
from multiprocessing import Process, cpu_count, Manager
import random
import time

# 常量定义
BRIGHTNESS_DELTA = (-60, 60)  # 亮度调整范围
CONTRAST_RANGE = (0.4, 1.7)   # 对比度调整范围
SATURATION_RANGE = (0.8, 1.6) # 饱和度调整范围
COLOR_SHIFT = 30.0            # 颜色偏移幅度
COLOR_SHIFT_PROB = 0.40       # 应用颜色偏移的概率
TRANSLATE_RATIO = 0.03        # 平移的最大比例（基于图像尺寸）

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
    ksize = int(6 * sigma + 1) | 1
    return cv2.GaussianBlur(img, (ksize, ksize), sigma)

def random_mask(image, mask, cnt=1):
    N, M, _ = image.shape
    blob = np.zeros((N, M))
    for _ in range(cnt):
        i = random.randint(0, N - 1)
        j = random.randint(0, M - 1)
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
    image = image.astype(np.float32)
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    hsv[:, :, 1] = np.clip(hsv[:, :, 1] * random.uniform(SATURATION_RANGE[0], SATURATION_RANGE[1]), 0, 255)
    return cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)

def random_saturation_mask(image):
    mask = random_saturation(image)
    return random_mask(image, mask)

def random_color(image):
    if random.random() < COLOR_SHIFT_PROB:  # 判断是否应用颜色偏移
        shift = np.array([random.uniform(-COLOR_SHIFT, COLOR_SHIFT) for _ in range(3)])
        return np.clip(image + shift, 0, 255)
    return image

def random_color_mask(image):
    mask = random_color(image)
    return random_mask(image, mask)

def random_rotate(image):
    angle = random.randint(0, 360)
    h, w = image.shape[:2]
    M = cv2.getRotationMatrix2D((w // 2, h // 2), angle, 1)
    return cv2.warpAffine(image, M, (w, h), borderValue=(192, 78, 65))

def random_scaling_mask(image):
    h, w = image.shape[:2]
    small = cv2.resize(image, (w // 2, h // 2), interpolation=cv2.INTER_AREA)
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

def random_translate_crop(image):
    h, w = image.shape[:2]
    max_dx = int(TRANSLATE_RATIO * w)
    max_dy = int(TRANSLATE_RATIO * h)
    dx = random.randint(-max_dx, max_dx)
    dy = random.randint(-max_dy, max_dy)
    M = np.float32([[1, 0, dx], [0, 1, dy]])
    translated_image = cv2.warpAffine(image, M, (w, h), borderValue=(0, 0, 0))

    # 平移后裁剪图像，使其返回到原始大小
    start_x = max(-dx, 0)
    start_y = max(-dy, 0)
    end_x = start_x + w
    end_y = start_y + h
    translated_cropped_image = translated_image[start_y:end_y, start_x:end_x]

    # 将裁剪后的图像调整回原始尺寸
    translated_cropped_image = cv2.resize(translated_cropped_image, (w, h), interpolation=cv2.INTER_LINEAR)
    return translated_cropped_image

def rotation_corr(image):
    width = image.shape[1]
    height = image.shape[0]
    pts1 = np.float32([[0, 0], [width, 0], [width, height], [0, height]])
    pts2 = np.float32([[0, 0], [320, 0], [320, 240], [0, 240]])
    M = cv2.getPerspectiveTransform(pts1, pts2)
    dst = cv2.warpPerspective(image, M, (320, 240))
    return dst

def add_noise(image, noise_level=5, prob=0.4):
    if random.random() < prob:  # 控制噪声的概率
        noise = np.random.randn(*image.shape) * noise_level
        noisy_image = image + noise
        return np.clip(noisy_image, 0, 255).astype(np.uint8)
    return image

def cutout(image, max_mask_size=20, prob=0.3):
    h, w = image.shape[:2]
    if random.random() < prob:  # 控制遮挡的概率
        mask_size = random.randint(5, max_mask_size)  # 随机选择遮挡黑块的大小
        y = np.random.randint(h)
        x = np.random.randint(w)
        y1 = np.clip(y - mask_size // 2, 0, h)
        y2 = np.clip(y + mask_size // 2, 0, h)
        x1 = np.clip(x - mask_size // 2, 0, w)
        x2 = np.clip(x + mask_size // 2, 0, w)
        image[y1:y2, x1:x2] = 0
    return image

def mixup(image1, image2, alpha=0.2):
    lam = np.random.beta(alpha, alpha)
    return lam * image1 + (1 - lam) * image2

# 数据增强的主要函数
def augment(image, additional_image=None):
    image = image.astype(np.float32)
    image = random_rotate(image)          # 随机旋转
    image = random_translate_crop(image)  # 应用平移裁剪
    image = random_brightness(image)      # 随机亮度调整
    image = random_color(image)           # 随机颜色偏移
    image = random_contrast(image)        # 随机对比度调整
    image = random_brightness_mask(image) # 随机亮度遮罩
    image = random_contrast_mask(image)   # 随机对比度遮罩
    image = random_saturation_mask(image) # 随机饱和度遮罩
    if random.random() < COLOR_SHIFT_PROB:  # 控制颜色偏移的概率
        image = random_color_mask(image)
    image = random_blur_mask(image)       # 随机模糊遮罩
    #  image = random_crop(image)            # 随机裁剪
    # image = random_scaling_mask(image)    # 随机缩放
    image = cutout(image, max_mask_size=40, prob=0.4)  # 应用 cutout 操作
    # 如果提供了额外的图像，则进行 mixup
    if additional_image is not None:
        image = mixup(image, additional_image)
    image = add_noise(image)              # 添加噪声
    return image.astype(np.uint8)

# 保存图像到指定路径
def save_image(image, folder, filename):
    if not os.path.exists(folder):
        os.makedirs(folder)
    cv2.imwrite(os.path.join(folder, filename), image)

# 处理单个图像
def process_image(image_path, save_folder, filename, num_aug=10):
    image = cv2.imread(image_path)
    original_image = augment(image)
    save_image(original_image, save_folder, filename)
    for i in range(num_aug):
        augmented_image = augment(image)
        name, ext = os.path.splitext(filename)
        new_filename = name + "_aug_" + str(i + 1) + ext
        save_image(augmented_image, save_folder, new_filename)

# 处理一个类别文件夹中的所有图像
def process_category(category_folder, save_folder, num_aug, progress_dict):
    files = os.listdir(category_folder)
    total_files = len(files)
    category_name = os.path.basename(category_folder)
    for i, filename in enumerate(tqdm(files, desc=f'Processing {category_name}')):
        image_path = os.path.join(category_folder, filename)
        process_image(image_path, save_folder, filename, num_aug)
        # 更新此类别的进度
        progress_dict[category_name] = (i + 1) / total_files * 100

# 处理根文件夹中的所有类别文件夹
def process_folders(root_folder, save_folder, num_aug=10, num_processes=15):
    categories = [os.path.join(root_folder, folder) for folder in os.listdir(root_folder) if
                  os.path.isdir(os.path.join(root_folder, folder))]
    manager = Manager()
    progress_dict = manager.dict()
    processes = []

    for i, category in enumerate(categories):
        if i >= num_processes:
            break
        save_subfolder = os.path.join(save_folder, os.path.basename(category))
        progress_dict[os.path.basename(category)] = 0  # 初始化每个类别的进度
        p = Process(target=process_category, args=(category, save_subfolder, num_aug, progress_dict))
        p.start()
        processes.append(p)

    try:
        while any(p.is_alive() for p in processes):
            time.sleep(1)
            # 打印每个类别的进度
            for category, progress in progress_dict.items():
                print(f'Category {category} progress: {progress:.2f}%')
    finally:
        # 确保所有进程都完成
        for p in processes:
            p.join()

if __name__ == '__main__':
    # 设置路径
    root_folder = "D:\\aaaaWordSpace\\Embedded_Development\\DEBUG_Visual_Part\\Debug_Tensorflow_GPU_2.9\\ALPHABETS_DATA\\newPlace\\train"
    save_folder = "D:\\aaaaWordSpace\\Embedded_Development\\DEBUG_Visual_Part\\Debug_Tensorflow_GPU_2.9\\ALPHABETS_DATA\\newPlace2"
    num_aug = 200

    # 多进程数据增强
    process_folders(root_folder, save_folder, num_aug)
