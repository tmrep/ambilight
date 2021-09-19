import mss
import numpy as np
import serial
import math
import time

# params to set
num_at_bottom_left = 12
num_at_left = 16
num_at_top = 29
num_at_right = num_at_left
num_at_bottom_right = num_at_bottom_left

sct = mss.mss()
# Get information of monitor 1
monitor_number = 1
mon = sct.monitors[monitor_number]

# coordinates of LEDs
dist_to_edge = 25 # px
dist = (mon["width"]-2*dist_to_edge)/num_at_top
x = np.hstack((np.linspace(mon["left"]+dist_to_edge+dist*num_at_bottom_left,mon["left"]+dist_to_edge,num_at_bottom_left),
            np.ones(num_at_left)*(mon["left"]+dist_to_edge),
            np.linspace(mon["left"]+dist_to_edge,mon["width"]-dist_to_edge,num_at_top),
            np.ones(num_at_right)*(mon["width"]-dist_to_edge),
            np.linspace(mon["width"]-dist_to_edge,mon["width"]-dist_to_edge-dist*num_at_bottom_right,num_at_bottom_right)))
y = np.hstack((np.ones(num_at_bottom_left)*(mon["height"]-dist_to_edge),
            np.linspace(mon["height"]-dist_to_edge,mon["top"]+dist_to_edge,num_at_left),
            np.ones(num_at_top)*(mon["top"]+dist_to_edge),
            np.linspace(mon["top"]+dist_to_edge,mon["height"]-dist_to_edge,num_at_right),
            np.ones(num_at_bottom_right)*(mon["height"]-dist_to_edge)))
orient = [['h']*num_at_bottom_left,['v']*num_at_left,['h']*num_at_top,['v']*num_at_right,['h']*num_at_bottom_right]
orient = [item for sublist in orient for item in sublist] # flatten the list
# bboxes of screen parts to capture
bbox_width = 200 # px
bbox_height = 400 # px

bboxes = []
for i in range(0,len(x)):
    if orient[i] == 'h':
        width = bbox_width
        height = bbox_height
    else:
        width = bbox_height
        height = bbox_width
    top = max(math.floor(y[i]-height/2),mon["top"])
    left = max(math.floor(x[i]-width/2),mon["left"])
    width = int(width/2+dist_to_edge)
    height = int(height/2+dist_to_edge)
    bboxes.append([top, left, width, height])
bboxes = np.array(bboxes)

values = np.array([0,0,0]*len(bboxes))

def go_fast(img,values):
    for i,bbox in enumerate(bboxes):
        cutout = img[bbox[0]:(bbox[0]+bbox[3]),bbox[1]:(bbox[1]+bbox[2]),:3]
        values[i*3] = int(np.mean(cutout[...,2]))
        values[i*3+1] = int(np.mean(cutout[...,1]))
        values[i*3+2] = int(np.mean(cutout[...,0]))

# jit the code
img = np.array(sct.grab(mon)) # Get raw pixels (bgra) from the screen, save it to a Numpy array
go_fast(img,values)

# with serial.Serial('/dev/ttyACM0') as ser:
with serial.Serial('COM3') as ser:
    while "Screen capturing":
        # last_time = time.time()
        img = np.array(sct.grab(mon)) # Get raw pixels (bgra) from the screen, save it to a Numpy array
        go_fast(img,values)
        ser.write(bytearray([0,0,0,0,0,0]+values.tolist())) # write a string
        # print(values)
        # print("fps: {}".format(1 / (time.time() - last_time)))

# while "Screen capturing":
#     last_time = time.time()
#     img = np.array(sct.grab(mon)) # Get raw pixels (bgra) from the screen, save it to a Numpy array
#     go_fast(img,values)
#     print("fps: {}".format(1 / (time.time() - last_time)))