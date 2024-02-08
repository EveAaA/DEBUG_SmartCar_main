# Untitled - By: G15 - 周六 1月 6 2024

import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)

clock = time.clock()

BackGround = [(0, 33, -128, 127, -128, -6)]

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

while(True):
    clock.tick()
    img = sensor.snapshot()
    BackBlob = img.find_blobs(BackGround, pixels_threshold = 400, area_threshold = 400, margin = 1,merge = True, invert = 0)
    b = find_max(BackBlob)
    img.draw_edges(b.corners())
    print(clock.fps())
