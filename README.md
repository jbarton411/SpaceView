# SpaceView
This repository is for a project at Penn State University, led by four honors students in attempt to get honors credit for our computer engineering class. This is where we will keep all of our source code, project goals, and other important files. 

Note: You may have to enter your Github credentials in order to perform the following operations.

To clone:
cd ~/Documents/
git clone https://github.com/jbarton411/SpaceView.git

To commit:
cd ~/Documents/SpaceView/
git commit -am <message>

To push:
cd ~/Documents/SpaceView/
git push 

To install the Python libraries:

Accelerometer:
cd ~/Documents/SpaceView/Code/Python/lib
git clone https://github.com/adafruit/Adafruit_Python_LSM303.git
cd Adafruit_Python_LSM303
sudo python setup.py install

Temp/Baro/Alt:
cd ~/Documents/SpaceView/Code/Python/lib
git clone https://github.com/adafruit/Adafruit_Python_BMP.git
cd Adafruit_Python_BMP
sudo python setup.py install

In order to run SpaceView.ino, you must install the free Arduino IDE:
https://www.arduino.cc/en/Main/Software

Also, you must download the necessary libraries:
Arduino->Sketch->Include Library->Manage Libraries...

Search and install the following libraries:
    Adafruit BMP085 Unified
    Adafruit L3GD20 Unified
    Adafruit LSM303 Unified
    Adafruit Sensor
    


