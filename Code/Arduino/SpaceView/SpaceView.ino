#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

#include <Adafruit_GPS.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
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

typedef uint16_t pitch;

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
    String filename = "LOG00.csv";
    for (uint8_t i = 0; i < 100; i++) {
      filename[3] = '0' + i/10;
      filename[4] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (!SD.exists(filename)) break;
    }
    logfile = SD.open(filename, FILE_WRITE);
    if(!logfile) {
      if (debug) { Serial.print("Couldnt create ");   Serial.println(filename); }
      error(FILE_INIT);
    }

    //Write header to logfile
    logfile.println("Time,Latitude,Longitude,Altitude,Pressure,Temperature,Acceleration,Acceleration (x),Acceleration (y),Acceleration (z)");
    
    //setup_complete
    setup_complete();
}

//Read the sensors and writing to the logfile in lines
void loop() {

    sensors_event_t event;      
    float accel_x, accel_y, accel_z, accel_mag, pressure, pressure1, altitude, temp, Time, latitude, longitude, gyro_x, gyro_y, gyro_z;
       
    while(1){

      //BMP085 sensor read
      bmp->getEvent(&event);
      temp = event.temperature;
      pressure = event.pressure;
      altitude = (1000000*(pow(pressure/101325,1/5.25588) + 1))/(2.25577);

      //LSM303 sensor read
      accel->getEvent(&event);
      accel_x = event.acceleration.x;
      accel_y = event.acceleration.y;
      accel_z = event.acceleration.z;
      accel_mag = sqrt( pow(accel_x, 2) + pow(accel_y, 2) + pow(accel_z, 2) );

      //L3GD20 sensor read
      gyro->getEvent(&event);
      gyro_x = event.gyro.x;
      gyro_y = event.gyro.y;
      gyro_z = event.gyro.z;
      
      //Write to file
      logfile.print(Time, latitude);
      logfile.print(latitude);
      logfile.print(longitude);
      logfile.print(altitude);
      logfile.print(pressure);
      logfile.print(temp);
      logfile.print(accel_mag);
      logfile.print(accel_x);
      logfile.print(accel_y);
      logfile.println(accel_z);
   
    }
  
}

void setup_complete(void) {

  pitch d5  = 587;
  pitch c5  = 523;
  pitch as4 = 466;
  pitch f4  = 349;
  pitch f5  = 698;
  pitch ds5 = 622;
  pitch cs5 = 554;
  pitch gs4 = 415;
  pitch gs5 = 831;
  pitch as5 = 932;

  int melody[] = {
  d5,  d5,  c5, as4,  f4,  d5,
  f5,  f5, ds5, cs5, gs4, ds5,
  f5, gs5, as5
    };

  int noteDurations[] = {
    4, 3, 1, 2, 4, 2,
    4, 3, 1, 2, 4, 2,
    14, 2, 9
  };

  for (int thisTone = 0; thisTone < 15; thisTone++) {
    int noteDuration = 250 * noteDurations[thisTone];
    tone(tonePin, melody[thisTone], noteDuration);

    int pauseBetweenNotes = noteDuration / 2;
    delay(pauseBetweenNotes);
    noTone(tonePin);
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

