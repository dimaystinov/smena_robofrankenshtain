import pyb
from pyb import LED
import sensor, image, time, math
from button_test import Button_Test
import json
from pyb import UART
uart = UART(1, 9600)
red_led = LED(1)
green_led = LED(2)
print("ready")
green_led.on()
#запуск по кнопке

#button = Button_Test()
#pressed_button = button.wait_for_button_pressing()
#if pressed_button == 1:
#    print ("go")

#трешхолды на мяч (L Min, L Max, A Min, A Max, B Min, B Max)
thresholds = [(31, 73, 37, 83, -11, 83)]
f_x = (2.8 / 3.984) * 160 # find_apriltags defaults to this if not set
f_y = (2.8 / 2.952) * 120 # find_apriltags defaults to this if not set
c_x = 160 * 0.5 # find_apriltags defaults to this if not set (the image.w * 0.5)
c_y = 120 * 0.5 # find_apriltags defaults to this if not set (the image.h * 0.5)

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.HQQVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
clock = time.clock()

#коэффицент размера мяча и расстояния от мяча до центральной линии
K = 2500
b = 10
#class gates:
#    def __init__(self):
#        self.
class ball_and_Commands:
    def __init__(self, blob):
        self.blob = blob
        self.length = 0
        self.angle = 0
        self.length_2 = 0
        self.angle_2 = 0
    def distance(self):
        Lm = (self.blob[2]+self.blob[3])/2
        length = K/Lm
        self.length = int(math.sqrt(length*length-361))
    def ang(self):
        angle = -((blob.cx()-60)*30/60)
        self.angle = int(angle)
    def counting(self):
        c = self.length
        a = math.sqrt(c*c - b*b)
        beta = math.asin(b / c)
        b_2 = b - 7
        c_2 = math.sqrt(a*a + b_2 * b_2)
        beta_2 = math.asin(b_2 / c_2)
        angle_2 = beta - beta_2
        angle_2 = int(math.degrees(angle_2))
        c_2 = int(c_2)
        self.length_2 = c_2
        self.angle_2 = angle_2
    def draw(self, blob, img):
        img.draw_edges(blob.min_corners(), color=(255,0,0))
        img.draw_line(blob.major_axis_line(), color=(0,255,0))
        img.draw_line(blob.minor_axis_line(), color=(0,0,255))
        img.draw_rectangle(blob.rect(), color=(255, 255, 255))
        img.draw_cross(blob.cx(), blob.cy())
        img.draw_line(60, 0, 60, 80, color=(255, 255, 0))
        img.draw_keypoints([(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=5)

def move(value):
    message ="{\"type\":\"move\",\"value\":" + str(value * 10 - 340) + "} \n"
    arduino_status = uart.readline()
    print(arduino_status)
    uart.write(message)

def rotate(value):
    message = "{\"type\":\"rotate\",\"value\":" + str(value) + "} \n"
    uart.write(message)

def degrees(radians):
    return (180 * radians) / math.pi

tag_families = 0
tag_families |= image.TAG16H5 # comment out to disable this family
tag_families |= image.TAG25H7 # comment out to disable this family
tag_families |= image.TAG25H9 # comment out to disable this family
tag_families |= image.TAG36H10 # comment out to disable this family
tag_families |= image.TAG36H11 # comment out to disable this family (default family)
tag_families |= image.ARTOOLKIT # comment out to disable this family

def family_name(tag):
    if(tag.family() == image.TAG16H5):
        return "TAG16H5"
    if(tag.family() == image.TAG25H7):
        return "TAG25H7"
    if(tag.family() == image.TAG25H9):
        return "TAG25H9"
    if(tag.family() == image.TAG36H10):
        return "TAG36H10"
    if(tag.family() == image.TAG36H11):
        return "TAG36H11"
    if(tag.family() == image.ARTOOLKIT):
        return "ARTOOLKIT"

flag1 = True
flag2 = True
flag3 = True

while(True):
    clock.tick()
    move(10)
    '''
    img = sensor.snapshot()
    #time.sleep(100)
    for blob in img.find_blobs(thresholds, pixels_threshold=10, area_threshold=10):
        #if blob.roundness() >  0.5:
        ball = ball_and_Commands(blob)
        ball.draw(blob, img)
        try:
            ball.distance()
        except:
            print("math_error_distance")
        ball.ang()
        #try:
        #    ball.counting()
        #except:
        #    print("math error2")
        print("length = ", ball.length, "angle = ", ball.angle, "angle 2 = ", ball.angle_2, "length2 = ", ball.length_2)
        #time.sleep(100)
        if flag2:
            rotate(-(ball.angle))
            flag2 = False
        #time.sleep(100)
        if flag1:
            move(ball.length)
            flag1 = False
        #print(message)


    try:
        red_led.off()
        for tag in img.find_apriltags(families=tag_families, fx=f_x, fy=f_y, cx=c_x, cy=c_y):
            img.draw_rectangle(tag.rect(), color = (255, 255, 0))
            img.draw_cross(tag.cx(), tag.cy(), color = (100, 0, 100))
            angle2 = -((tag.cx()-60)*30/60)
            print_args = (family_name(tag), tag.id(), (180 * tag.rotation()) / math.pi)
            print_args_2 = (tag.x_translation(), tag.y_translation(), tag.z_translation(), degrees(tag.x_rotation()), degrees(tag.y_rotation()), degrees(tag.z_rotation()))
            print("Tag Family %s, Tag ID %d, rotation %f (degrees)" % print_args)
            print("Tx: %f, Ty %f, Tz %f, Rx %f, Ry %f, Rz %f" % print_args_2)
    except:
        red_led.on()
        print("framesize_error")
    if flag3:
        rotate(-(angle2))
        flag3 = False
    move(100)
print("end")
'''
