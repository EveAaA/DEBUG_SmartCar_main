# Untitled - By: G15 - 周六 12月 23 2023
from machine import UART
import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

clock = time.clock()

uart = UART(2, 115200)
send_num = 150

def limit(num):
    if num >= 127:
        return 127
    elif num < -128:
        return -128
    else:
        return num

while(True):
    clock.tick()
    send_num = limit(send_num)
    print(int(send_num).to_bytes(1, "little"))
    uart.write(int(send_num).to_bytes(1, "little"))
    uart_num = uart.any()       # 获取当前串口数据数量
    print(uart_num)
    if uart_num:
        data = uart.read(uart_num)
        print("获取到的数据:",data)



