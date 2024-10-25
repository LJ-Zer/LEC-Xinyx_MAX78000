import serial
import numpy as np
import matplotlib.pyplot as plt


received = []

dim = 32

ser = serial.Serial('/dev/cu.usbmodem1434102', 115200)#timeout=1)  # open serial port
ser.flush()
while True:
    print(ser.name)         # check which port was really used
    # ser.write(b'hello')     # write a string
    temp = ser.readline().strip()
    received = (np.fromstring(temp, sep=','))/255.0
    print(received)
    # received = ser.readline()

    img = cv2.imread(join(data_path, data_class, img), 0) #,0 in case grayscale

    plt.imshow(np.reshape(np.array(received), (dim, dim)))
    plt.show()

ser.close()             # close port
