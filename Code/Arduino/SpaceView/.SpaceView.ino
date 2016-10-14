
// Arduino includes
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

// Our includes
#include <Adafruit_GPS.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_10DOF.h>

// Constants: pins
#define tonePin 6
#define chipSelect 10
#define ledPin 13

// Constants: config
#define debug 1
#define override_log = 1

// Constants: other
#define errorFrequency 440 //Concert A

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
Adafruit_GPS gps(&mySerial);

File logfile;
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;

String now, latitude, longitude, altitude, pressure, temperature, accel, accel_x, accel_y, accel_z;

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
    String filename = "LOG00.csv";
    for (uint8_t i = 0; i < 100; i++) {
      filename[3] = '0' + i/10;
      filename[4] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (!SD.exists(filename)) break;
    }
    logfile = SD.open(filename, FILE_WRITE);
    if (debug) { Serial.print("Log file created! Log name: "); Serial.println(filename); }
    if(!logfile) {
      if (debug) { Serial.print("Couldnt create ");   Serial.println(filename); }
      error(FILE_INIT);
    }

  // Write header to logfile
    String header = "Time,Latitude,Longitude,Altitude,Pressure,Temperature,Acceleration,Acceleration (x),Acceleration (y),Acceleration (z)";
    logfile.println(header);

    now = latitude = longitude = altitude = pressure = temperature = accel = accel_x = accel_y = accel_z = String();
}

void loop() {
  now = latitude = longitude = altitude = pressure = temperature = accel = accel_x = accel_y = accel_z = "";
  sensors_event_t* accel_event;
  sensors_event_t* mag_event;
  sensors_event_t* bmp_event;
  sensors_event_t* gyro_event;

  accel.getEvent(accel_event);
  mag.getEvent(mag_event);
  bmp.getEvent(bmp_event);
  gyro.getEvent(gyro_event);
  
  char c = gps.read();
  if (debug && c) Serial.print(c);

  if (gps.newMEAreceived()) {
    gps.parse(gps.lastNMEA());
  }


  int hours = gps.hour + HOUR_OFFSET;
  if (hours < 0)
    hours = 24+hours;
  if (hours > 23)
    hours = 24-hours;
  int minutes = gps.minute;
  int seconds = gps.seconds;

  // HH:MM:SS
  now += hours; now += ':'; now += minutes; now += ':'; now += seconds;
  latitude = "DDMMSS W";
  longitude = "DDDMMSS N";
  altitude = String(bmp->pressureToAltitude(seaLevelPressure, bmp_event->pressure), 6);
  pressure = String(bmp_event->pressure, 6);
  temperature = String(bmp_event->temperature, 6);
  accel_x = String(accel_event->acceleration->x, 6);
  accel_y = String(accel_event->acceleration->y, 6);
  accel_z = String(accel_event->acceleration->z, 6);
  accel = sqrt( (accel_x ^ 2) + (accel_y ^ 2) + (accel_z ^ 2) );


  /*
  // if a sentence is received, we can check the checksum, parse it...
  if (gps.newNMEAreceived()) {
    char *stringptr = gps.lastNMEA();
    
    if (!gps.parse(stringptr))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another

    // Sentence parsed! 
    if (!gps.fix && debug) {
      Serial.print("No Fix");
      return;
    }

    uint8_t stringsize = strlen(stringptr);
    if (stringsize != logfile.write((uint8_t *)stringptr, stringsize)) error(LOG_WRITE);
    if (strstr(stringptr, "RMC") || strstr(stringptr, "GGA")) logfile.flush();
  }
  */
  
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

