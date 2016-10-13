#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

#include <Adafruit_GPS.h>
#include <Adafruit_10DOF.h>

#define tonePin 6
#define errorFrequency 440 //Concert A
#define chipSelect 10
#define ledPin 13


#define debug 1

typedef enum {
  SENSOR_INIT = 1,
  SD_CARD_INIT = 2,
  FILE_INIT = 3,
  LOG_WRITE = 4
  //List other errors here
} Error;

/* Assign a unique ID to the sensors */
Adafruit_10DOF* dof;
Adafruit_LSM303_Accel_Unified* accel;
Adafruit_LSM303_Mag_Unified* mag;
Adafruit_BMP085_Unified* bmp;
Adafruit_L3GD20_Unified* gyro;

SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);

File logfile;
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;

void setup() {
  // Sensor initialization:
    dof = new Adafruit_10DOF();
    accel = new Adafruit_LSM303_Accel_Unified(30301);
    mag = new Adafruit_LSM303_Mag_Unified(30302);
    bmp = new Adafruit_BMP085_Unified(18001);
    gyro = new Adafruit_L3GD20_Unified(20);

    if(!accel->begin() || !mag->begin() || !bmp->begin() || !gyro->begin()) {
    /* There was a problem detecting the LSM303 ... check your connections */
      if (debug) Serial.println("Error initializing sensors!");
      error(SENSOR_INIT);
    }

    if (debug) displaySensorDetails();
    
  // SD card initialization:
    if (!SD.begin(chipSelect)) {
      if (debug) Serial.println("Error initializing SD card!");
      error(SD_CARD_INIT);
    }
    
  // Opening file for log
    char filename[13] = "GPSLOG00.csv";
    for (uint8_t i = 0; i < 100; i++) {
      filename[6] = '0' + i/10;
      filename[7] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (!SD.exists(filename)) break;
    }
    logfile = SD.open(filename, FILE_WRITE);
    if(!logfile) {
      if (debug) { Serial.print("Couldnt create ");   Serial.println(filename); }
      error(FILE_INIT);
    }

    
    
}

void loop() {
  char c = GPS.read();
  if (debug && c) Serial.print(c);

  
  
  



  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    char *stringptr = GPS.lastNMEA();
    
    if (!GPS.parse(stringptr))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another

    // Sentence parsed! 
    if (!GPS.fix && debug) {
      Serial.print("No Fix");
      return;
    }

    uint8_t stringsize = strlen(stringptr);
    if (stringsize != logfile.write((uint8_t *)stringptr, stringsize)) error(LOG_WRITE);
    if (strstr(stringptr, "RMC") || strstr(stringptr, "GGA")) logfile.flush();
  }
  
}

void error(uint8_t errno) {
    tone(tonePin, 440, 1000);
    delay(500);
    for (uint8_t i = 0; i < errno; i++) {
      digitalWrite(ledPin, HIGH);
      tone(tonePin, 440, 500);
      delay(500);
      digitalWrite(ledPin, LOW);
    }

    while(1);
}

void displaySensorDetails(void) {
  sensor_t* sensor;
  
  accel->getSensor(sensor);
  Serial.println(F("----------- ACCELEROMETER ----------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor->name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor->version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor->sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor->max_value); Serial.println(F(" m/s^2"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor->min_value); Serial.println(F(" m/s^2"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor->resolution); Serial.println(F(" m/s^2"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));

  gyro->getSensor(sensor);
  Serial.println(F("------------- GYROSCOPE -----------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor->name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor->version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor->sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor->max_value); Serial.println(F(" rad/s"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor->min_value); Serial.println(F(" rad/s"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor->resolution); Serial.println(F(" rad/s"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  
  mag->getSensor(sensor);
  Serial.println(F("----------- MAGNETOMETER -----------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor->name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor->version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor->sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor->max_value); Serial.println(F(" uT"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor->min_value); Serial.println(F(" uT"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor->resolution); Serial.println(F(" uT"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));

  bmp->getSensor(sensor);
  Serial.println(F("-------- PRESSURE/ALTITUDE ---------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor->name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor->version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor->sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor->max_value); Serial.println(F(" hPa"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor->min_value); Serial.println(F(" hPa"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor->resolution); Serial.println(F(" hPa"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  
  delay(500);
}

