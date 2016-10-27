# Record all values from BMp180/LSM303/L3GD20H every 5 seconds
# Author: John Barton 
# Last Modified 10/21/2016 by Erica Murphy


'''

  GPS:                 RPI:
  TX------------------RX
  RX------------------TX
 VIN------------------5V
 GND------------------GND
 
#Terminal Commands once USB is connected to raspberry pi
ls /dev/ttyUSB*
sudo apt-get install gpsd gpsd-clients python-gps
sudo systemctl stop gpsd.socket
sudo systemctl disable gpsd.socket
sudo gpsd /dev/ttyUSB0 -F /var/run/gpsd.sock # change '/dev/ttyUSB0' to proper destination
cgps -s
'''

import gps
import time
from datetime import datetime
import os.path
import csv
from math import sqrt, pow
from Adafruit_BMP import BMP085
import Adafruit_LSM303 as LSM303


# Constants
debug = 1
override_log = 1

# Sensor initialization
accelSensor = LSM303.LSM303()
tempSensor = BMP085.BMP085()

# Open a new file to store our info in
filename = "LOG00.csv"
if not override_log:
	filename_l = list(filename)
	for n in range (0, 100):
		filename_l[3] = str(0 + n/10)
		filename_l[4] = str(0 + n%10)
		s = "".join(filename_l)
		if not os.path.exists(s):
			break
	filename = "".join(filename_l)
logfile = open(filename, "w")
if debug:
	print "Log file created! Log name: ", filename


# Write header to logfile
csv_writer = csv.writer(logfile)
csv_writer.writerow(['Time','Latitude','Longitude','Altitude','Pressure','Temperature','Acceleration','Acceleration (x)', 'Acceleration (y)', 'Acceleration (z)'])

t0 = clock()

'''
while True:
    try:
    	report = session.next()
		# Wait for a 'TPV' report and display the current time
		# To see all report data, uncomment the line below
		# print report
        if report['class'] == 'TPV':
            if hasattr(report, 'time'):
                print report.time
                print('Latitude (knots):', GPS.latitude)
                print('Longitude (knots):', GPS.longitude)
    except KeyError:
		pass
    except KeyboardInterrupt:
		quit()
    except StopIteration:
		session = None
		print "GPSD has terminated"
    global GPS.lat
'''

# Read data from sensors, write to file
while True:
	latitude = GPS.latitude
	longitude = GPS.longitude
	timeString = time.strftime('%H:%M:%S', time.localtime())

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

	csv_writer.writerow([timeString,latitude,longitude,altitude,pressure,temperature,accel_mag,accel_x,accel_y,accel_z])

	time.sleep(1)

# Listen on port 2947 (gpsd) of localhost
session = gps.gps("localhost", "2947")
session.stream(gps.WATCH_ENABLE | gps.WATCH_NEWSTYLE)
 
logfile.close()
