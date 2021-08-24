# AprilTags Example
#
# This example shows the power of the OpenMV Cam to detect April Tags
# on the OpenMV Cam M7. The M4 versions cannot detect April Tags.

import sensor, image, time, math

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA) # we run out of memory if the resolution is much bigger...
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False)  # must turn this off to prevent image washout...
sensor.set_auto_whitebal(False)  # must turn this off to prevent image washout...
clock = time.clock()

input_smoothing_dist_z = [100,100,100,100,100,100,100,100,100,100]
y_angle_array = [0,0,0,0,0,0,0,0,0,0]
num_of_measures = 0

f_x = (2.8 / 3.984) * 160 # find_apriltags defaults to this if not set
f_y = (2.8 / 2.952) * 120 # find_apriltags defaults to this if not set
c_x = 160 * 0.5 # find_apriltags defaults to this if not set (the image.w * 0.5)
c_y = 120 * 0.5 # find_apriltags defaults to this if not set (the image.h * 0.5)

def degrees(radians):
    return (180 * radians) / math.pi

def unitsToCm(units):
    return units * 8

def medium(input2):
    summ = 0
    for i in input2:
        summ += i
    return summ / 10

while(True):
    clock.tick()
    img = sensor.snapshot()
    for tag in img.find_apriltags(fx=f_x, fy=f_y, cx=c_x, cy=c_y): # defaults to TAG36H11
        img.draw_rectangle(tag.rect(), color = (255, 0, 0))
        img.draw_cross(tag.cx(), tag.cy(), color = (0, 255, 0))
        print_args = (tag.x_translation(), tag.y_translation(), tag.z_translation(), \
            degrees(tag.x_rotation()), degrees(tag.y_rotation()), degrees(tag.z_rotation()))
        #print("Tx: %f, Ty %f, Tz %f, Rx %f, Ry %f, Rz %f" % print_args)

        ''' input_smoothing_dist_z.insert(0,-tag.z_translation())
        input_smoothing_dist_z.pop()
        summ_z = 0
        for i in input_smoothing_dist_z:
            summ_z += i

        input_smoothing_angle_x.insert(0,tag.y_rotation())
        input_smoothing_angle_x.pop()
        summ_x = 0
        for i in input_smoothing_angle_x:
            summ_x += i  '''
        y_angle_array[num_of_measures] = tag.x_translation()
        num_of_measures += 1
        if num_of_measures >= 10:
            print(medium(y_angle_array))
            num_of_measures = 0

        #print(unitsToCm(summ_z / 10))
        #print(degrees(summ_x / 10))
    #print(clock.fps())
