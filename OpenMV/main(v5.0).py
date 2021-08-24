import pyb
from pyb import LED
import sensor, image, time, math
#from button_test import Button_Test
import json
from pyb import UART
import json

uart = UART(1, 9600)
red_led = LED(1)
green_led = LED(2)
print("ready")
green_led.on()

coeff_tag0 = 100/17
coeff_tag1 = 100/17
gates_length = 1
#запуск по кнопке

#button = Button_Test()
#pressed_button = button.wait_for_button_pressing()
#if pressed_button == 1:
#    print ("go")

#трешхолды на мяч (L Min, L Max, A Min, A Max, B Min, B Max)
thresholds = [(36, 72, 57, 96, -6, 72)]
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

#коэффицент размера мяча, расстояния от мяча до центральной линии и отступ до мяча
K_ball_area = 841
b = 54
b1 = 20
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
        Lm = (self.blob.w()+self.blob.h())/2
        length = K_ball_area/Lm
        self.length = int(math.sqrt(length*length-361))
    def ang(self):
        angle = -((blob.cx()-60)*30/60)
        self.angle = int(angle)
    def counting(self):
        c = self.length
        a = math.sqrt(c*c - b*b)
        beta = math.asin(b / c)
        beta = math.degrees(beta)
        b_2 = b - b1
        c_2 = math.sqrt(a*a + b_2 * b_2)
        beta_2 = math.asin(b_2 / c_2)
        angle_2 = beta - beta_2
        #angle_2 = int(math.degrees(angle_2))
        c_2 = int(c_2)
        self.length_2 = a
        self.angle_2 = beta
    def draw(self, blob, img):
        img.draw_edges(blob.min_corners(), color=(255,0,0))
        img.draw_line(blob.major_axis_line(), color=(0,255,0))
        img.draw_line(blob.minor_axis_line(), color=(0,0,255))
        img.draw_rectangle(blob.rect(), color=(255, 255, 255))
        img.draw_cross(blob.cx(), blob.cy())
        img.draw_line(60, 0, 60, 80, color=(255, 255, 0))
        img.draw_keypoints([(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=5)

def move(value):
    if value >= 15:
        message ="{\"type\":\"move\",\"value\":" + str(int(10 * value - 50)) + "} \n"
    else:
        message ="{\"type\":\"move\",\"value\":" + str(int(10 * value)) + "} \n"

    uart.write(message)
    time.sleep(abs(int(value * 30)))
    print("move ", value)
    '''
   uart.write("{\"type\":\"move\",\"value\":" + str(0) + "} \n")
    time.sleep(200)
    try:
        arduino_json = uart.readline().decode()
        if arduino_json != None:
            # print(arduino_json)
            json_data = json.loads(arduino_json)
            arduino_status = json_data["value"]
            if int(arduino_status) == -2:
                # print(arduino_status)
                time.sleep(50)
            if int(arduino_status) == 0:
                # uart.write(message)
                pass
        else:
            json_data = json.loads(arduino_json)
    except:
        # print('error arduino')
        time.sleep(50)
    # uart.write(message)'''

def rotate(value):


    '''if value >=90:
        value -= 0
    elif value >=50:
        value  -=
'''
    message = "{\"type\":\"rotate\",\"value\":" + str(int(value) ) + "} \n"
    uart.write(message)

    print("sleep rotate", abs(int(value / 360 *1000 * 3)))
    time.sleep(abs(int(value / 360 *1000 * 3)))
    print("rotate: ", value)
    '''try:
        arduino_json = uart.readline().decode()
        if arduino_json != None:
            # print(arduino_json)
            json_data = json.loads(arduino_json)
            arduino_status = json_data["value"]
            if int(arduino_status) == -2:
                # print(arduino_status)
                time.sleep(1000)
            if int(arduino_status) == 0:
                uart.write(message)
    except:
        # print('error arduino')
        time.sleep(50)'''

def kick():
    message ="{\"type\":\"status\",\"value\":7} \n "
    uart.write(message)


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

ball_found = False
flag0 = True
flag1 = True
flag2 = True
flag3 = True
flag4 = False


while(True):
    clock.tick()
    img = sensor.snapshot()
    time.sleep(100)
    while ball_found == False:
        img = sensor.snapshot()
        for blob in img.find_blobs(thresholds, pixels_threshold=10, area_threshold=10):
            if blob.roundness() >  0.1:
                ball = ball_and_Commands(blob)
                ball.draw(blob, img)
            try:
                ball.distance()
                ball_found = True
                ball.ang()
                ball.counting()
            except:
                print("math_error_distance")
        if not(ball_found):
            rotate(20)
        #print("length = ", ball.length, "angle = ", ball.angle, "angle 2 = ", ball.angle_2, "length2 = ", ball.length_2)
    print("ball detected wait 1 sec")
    time.sleep(100)
    img = sensor.snapshot()
    ball = ball_and_Commands(blob)
    ball.distance()
    ball.ang()
    ball.counting()

    print("length = ", ball.length, "angle = ", ball.angle, "angle 2 = ", ball.angle_2, "length2 = ", ball.length_2)
    time.sleep(1000)

    if ball_found and flag1:
       try:
           rotate(ball.angle)
       except:
           print("ball not found")
       flag1 = False
       print("done")
       time.sleep(100)
    print("ball fitting wait 1 sec")
    print(ball.angle_2)
    time.sleep(1000)

    if flag2 and not(flag1):
        rotate(ball.angle_2)
        print("rotate_angle2 ", (ball.angle_2))
        time.sleep(100)
        print("rotate to angle2 and wait 1 sec")
        time.sleep(1000)
        move(ball.length_2)
        print("move to length2 and wait 1 sec")
        time.sleep(1000)
        rotate(-90)
        print("rotate to 90 and wait 1 sec")
        time.sleep(1000)
        flag2 = False
        #message = "{\"type\":\"kick\",\"value\":" + str(2000) + "} \n"
        time.sleep(100)

    if flag3 and not(flag2):
        img = sensor.snapshot()
        for blob in img.find_blobs(thresholds, pixels_threshold=10, area_threshold=10):
            if blob.roundness() >  0.1:
                ball = ball_and_Commands(blob)
                ball.draw(blob, img)
            try:
                ball.distance()
                ball_found = True
                ball.ang()
                ball.counting()
            except:
                print("math_error_distance")
        rotate(ball.angle)
        time.sleep(1000)
        move(ball.length-10)
        time.sleep(2000)
        kick()
        print("kick and wait 10 sec")
        time.sleep(10000)
        print("finish")
    break
