# Record all values from BMp180/LSM303/L3GD20H every 5 seconds
# Author: John Barton 
# Last Modified 10/10/2016
 
import time
from datetime import datetime
import os.path
import csv
from math import sqrt, pow
from Adafruit_BMP import BMP085
import Adafruit_LSM303 as LSM303

# Constants
DEBUG = 1
OVERRIDE_LOG = 1 # captalize?

# Sensor initialization
accelSensor = LSM303.LSM303()
tempSensor = BMP085.BMP085()

# Open a new file to store our info in
filename = "LOG00.csv"
if not OVERRIDE_LOG:
	filename_l = list(filename)
	for n in range (0, 100):
		filename_l[3] = str(0 + n/10)
		filename_l[4] = str(0 + n%10)
		s = "".join(filename_l)
		if not os.path.exists(s):
			break
	filename = "".join(filename_l)
logfile = open(filename, "w")
if DEBUG:
	print "Log file created! Log name: ", filename


# Write header to logfile
csv_writer = csv.writer(logfile)
csv_writer.writerow(['Time','Latitude','Longitude','Altitude','Pressure','Temperature','Acceleration','Acceleration (x)', 'Acceleration (y)', 'Acceleration (z)'])


# Read data from sensors, write to file
while True:
	now = datetime.now().strftime('%H:%M:%S') # eventually read from GPS
	latitude = 'DDMMSS W' # eventually read from GPS
	longitude = 'DDDMMSS N' # eventually read from GPS

	# BMP180 sensor reads
	altitude = tempSensor.read_altitude()
	pressure = tempSensor.read_pressure()
	temperature = tempSensor.read_temperature()
	
	# LSM303 sensor reads
	accel, tmp = accelSensor.read()
	accel_x, accel_y, accel_z = accel
	accel_x = float(accel_x) / 1000
	accel_y = float(accel_y) / 1000
	accel_z = float(accel_z) / 1000
	accel_mag = sqrt( pow(accel_x, 2) + pow(accel_y, 2) + pow(accel_z, 2) )

	csv_writer.writerow([now,latitude,longitude,altitude,pressure,temperature,accel_mag,accel_x,accel_y,accel_z])

	time.sleep(1)

logfile.close()
