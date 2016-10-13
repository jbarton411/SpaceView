# Record all values from BMp180/LSM303/L3GD20H every 5 seconds
# Author: John Barton 
# Last Modified 10/10/2016

import time
import os.path
from Adafruit_BMP import BMP085
import Adafruit_LSM303 as LSM303

lsm303 = LSM303.LSM303()
bmp085 = BMP085.BMP085()

filename = "LOG00.csv"
for n in range (0, 100):
	filename[3] = '0' + n/10
	filename[4] = '0' + n%10
	if !os.path.exists(file_path):
		break
	
logfile = open(filename, "w")

print("SUCCESS")