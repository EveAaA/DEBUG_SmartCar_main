
import sensor, image, time, tf, gc
from machine import UART
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
# sensor.set_brightness(100)
sensor.set_auto_exposure(True, 100)
clock = time.clock()                # Create a clock object to track the FPS.

#设置模型路径
border_detect = '/sd/findBesideCard.tflite'
net2 = '/sd/Debug_VisionPlace.tflite'
labels = [line.rstrip("\n") for line in open("/sd/label.txt")] + ["none"]
#载入模型
net = tf.load(border_detect)
UART_TAIl = b"\x80"
UART_DIR_STRAIGHT = b"\x12"
UART_DIR_LEFT = b"\x10"
UART_DIR_RIGHT = b"\x11"
UART_BORDER_FIND = b"\x01"
UART_NOT_BORDER_FIND = b"\x00"
NO_OBJ = 0
IS_OBJ = 0
TARGET_LINE = 180
uart = UART(1, 115200)
sharpen_kernel = (0, -1, 0, -1, 5, -1, 0, -1, 0)
def classifyAlphaBet(img, minRect):
    img1 = img.copy()
    img1.rotation_corr(corners=minRect)
    inputImage = sensor.alloc_extra_fb(96, 96, sensor.GRAYSCALE)
    inputImage.draw_image(img1, 0, 0,x_scale=0.3, y_scale=0.4)
    inputImage.invert()
    inputImage.morph(1, sharpen_kernel, 1)
    his = inputImage.get_histogram()
    inputImage.binary([[his.get_threshold().value(), 255]], invert = True) # 突出图像特征
    inputImage.erode(1)
    model = tf.load(net2, load_to_fb=True)
    obj = tf.classify(model, inputImage)[0]
    tf.free_from_fb()
    res = obj.output()
    m = max(res)
    result_index = res.index(m)
    #print("%s: %f" % (labels[result_index], m))
    # print(res)
    if m >= 0.95:
        result_index = res.index(m)
        print("%s: %f" % (labels[result_index], m))
    else:
        print("Unknow!")
    img.draw_image(inputImage, 0, 0)
    sensor.dealloc_extra_fb()

while(True):
    clock.tick()
    img = sensor.snapshot()
    img1 = img.copy(x_scale = 0.75)
    #使用模型进行识别
    obj = tf.detect(net, img1)
    if obj:
        x1,y1,x2,y2,label,scores = obj[0] # 获取置信度最高的一项
        if(scores > 0.70):
            NO_OBJ = 0
            IS_OBJ += 1
            w = x2- x1
            h = y2 - y1
            x1 = int((x1)*img.width())
            y1 = int(y1*img.height())
            w = int(w*img.width())
            h = int(h*img.height())
            cx = int(x1+w/2)
            cy = int(y1+h/2)
            minRect = [(x1, y1), ((x1 + w), y1), ((x1 + w), (y1 + h)), (x1, (y1 + h))]
            #classifyAlphaBet(img, tuple(minRect))
            if cy >= TARGET_LINE and IS_OBJ >= 7:
                dx = int(cx - img.width()/2)
                if cx < int(img.width() / 2):
                    print("LEFT")
                    uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_LEFT + UART_TAIl)
                    # print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                else:
                    print("RIGHT")
                    uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_RIGHT + UART_TAIl)
                     # print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                img.draw_line(0, TARGET_LINE, sensor.width(), TARGET_LINE, color = (255, 0, 0), thickness = 3)
            else:
                dx = int(cx - img.width()/2)
                print("GO AHEAD")
                uart.write(UART_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
                img.draw_line(0, TARGET_LINE,  sensor.width(), TARGET_LINE, color = (255, 255, 0), thickness = 3)


            img.draw_rectangle((x1,y1,w,h),thickness=2, color = (0, 255, 0))
        else:
            NO_OBJ += 1
            IS_OBJ = 0
            dx = 0
            uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
            # print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
    else:
        NO_OBJ += 1
        IS_OBJ = 0
        dx = 0
        uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
    if NO_OBJ > 5:
        dx = 0
        print("None")
        uart.write(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
        # print(UART_NOT_BORDER_FIND + dx.to_bytes(1, "little") + UART_DIR_STRAIGHT + UART_TAIl)
