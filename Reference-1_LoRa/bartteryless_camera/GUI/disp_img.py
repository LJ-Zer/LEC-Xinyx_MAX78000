import serial
import numpy as np
import matplotlib.pyplot as plt
from os.path import join
import cv2

ser = serial.Serial('/dev/cu.usbmodem143303', 115200)#timeout=1)  # open serial port
ser.flush()

print(ser.name)         # check which port was really used

temp = ser.readline().strip().decode("utf-8") 
print(temp)
img = cv2.imread("./" + temp + ".jpg") #,0 in case grayscale

b_temp = img[:,:,1]
r_temp = img[:,:,2]
img[:,:,0] = r_temp
img[:,:,2] = b_temp

plt.imshow(img)
plt.show()

ser.close()             # close port
