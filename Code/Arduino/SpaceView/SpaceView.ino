#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

#include <Adafruit_GPS.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>

//Pin definitions
#define tonePin 6
#define chipSelect 10
#define ledPin 13

//Config options
#define DEBUG 1
#define HOUR_OFFSET -5
#define ERROR_FREQ 440

typedef enum {
  SENSOR_INIT = 1,
  SD_CARD_INIT = 2,
  FILE_INIT = 3,
  LOG_WRITE = 4
  //List other errors here
} Error;

typedef uint16_t pitch;

/* Assign a unique ID to the sensors */
Adafruit_LSM303_Accel_Unified* accel;
Adafruit_LSM303_Mag_Unified* mag;
Adafruit_BMP085_Unified* bmp;
Adafruit_L3GD20_Unified* gyro;

//Learn how to set this up with Mega
SoftwareSerial mySerial(8, 7);
Adafruit_GPS gps(&mySerial);

File logfile;
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;

void setup() {
  // Sensor initialization:
  accel = new Adafruit_LSM303_Accel_Unified(30301);
  mag = new Adafruit_LSM303_Mag_Unified(30302);
  bmp = new Adafruit_BMP085_Unified(18001);
  gyro = new Adafruit_L3GD20_Unified(20);

  if (!accel->begin() || !mag->begin() || !bmp->begin() || !gyro->begin()) {
    /* There was a problem detecting the LSM303 ... check your connections */
    if (DEBUG) Serial.println("Error initializing sensors!");
    error(SENSOR_INIT);
  }

  if (DEBUG) displaySensorDetails();

  // SD card initialization:
  if (!SD.begin(chipSelect)) {
    if (DEBUG) Serial.println("Error initializing SD card!");
    error(SD_CARD_INIT);
  }

  // Opening file for log
  String filename = "LOG00.csv";
  for (uint8_t i = 0; i < 100; i++) {
    filename[3] = '0' + i / 10;
    filename[4] = '0' + i % 10;
    // create if does not exist, do not open existing, write, sync after write
    if (!SD.exists(filename)) break;
  }
  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    if (DEBUG) {
      Serial.print("Couldnt create ");
      Serial.println(filename);
    }
    error(FILE_INIT);
  }

  //Write header to logfile
  logfile.println("Time (HH:MM:SS),Latitude (°),Longitude (°),Altitude (m),Pressure (hPa),Temperature (°C),Acceleration_x (gs),Acceleration_y (gs),Acceleration_z (gs),Gyro_x (rad/s),Gyro_y (rad/s),Gyro_z (rad/s)");

  //GPS Setup
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); //Minimum updates, only updates position and time
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // Use a 1 hz, once a second, update rate.
  enableGPSInterrupt(); // Enable the interrupt to parse GPS data.

  //setup_complete
  setup_complete();
}

//Read the sensors and writing to the logfile in lines
void loop() {
  sensors_event_t event;
  int hours, minutes, seconds;
  float latitude, longitude, altitude, pressure, temp, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z;
  char lat, lon;

  //BMP085 sensor read
  bmp->getEvent(&event);
  temp = event.temperature;
  pressure = event.pressure;
  altitude = bmp->pressureToAltitude(seaLevelPressure, pressure);

  //LSM303 sensor read
  accel->getEvent(&event);
  accel_x = event.acceleration.x;
  accel_y = event.acceleration.y;
  accel_z = event.acceleration.z;
  //accel_mag = sqrt( pow(accel_x, 2) + pow(accel_y, 2) + pow(accel_z, 2) );
  //I'm thinking we should calculate this in Matlab post experiment?

  //L3GD20 sensor read
  gyro->getEvent(&event);
  gyro_x = event.gyro.x;
  gyro_y = event.gyro.y;
  gyro_z = event.gyro.z;

  //GPS sensor read
  if (gps.newNMEAreceived()) {
    gps.parse(gps.lastNMEA());
  }
  latitude = gps.latitude; lat = gps.lat;
  longitude = gps.longitude; lat = gps.lon;
  hours = gps.hour + HOUR_OFFSET;
  if (hours < 0) // Handle when UTC + offset wraps around to a negative or > 23 value.
    hours = 24 + hours;
  if (hours > 23)
    hours = 24 - hours;
  minutes = gps.minute;
  seconds = gps.seconds;

  //Write to file
  logfile.print(hours); logfile.print(":");
  logfile.print(minutes); logfile.print(":");
  logfile.print(seconds); logfile.print(",");
  logfile.print(latitude); logfile.print(" "); logfile.print(lat); logfile.print(",");
  logfile.print(longitude); logfile.print(" "); logfile.print(lon); logfile.print(",");
  logfile.print(longitude); logfile.print(",");
  logfile.print(altitude); logfile.print(",");
  logfile.print(pressure); logfile.print(",");
  logfile.print(temp); logfile.print(",");
  logfile.print(accel_x); logfile.print(",");
  logfile.print(accel_y); logfile.print(",");
  logfile.print(accel_z); logfile.print(",");
  logfile.print(gyro_x); logfile.print(",");
  logfile.print(gyro_y); logfile.print(",");
  logfile.println(gyro_z); //Last row entry

}

void setup_complete(void) {
  //TODO: use the tone library?
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

  while (1);
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

SIGNAL(TIMER0_COMPA_vect) {
  // Use a timer interrupt once a millisecond to check for new GPS data.
  // This piggybacks on Arduino's internal clock timer for the millis()
  // function.
  gps.read();
}

void enableGPSInterrupt() {
  // Function to enable the timer interrupt that will parse GPS data.
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function above
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}
