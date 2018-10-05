//Name:         Central
//Description:  This program measures tank level (cm), ambient temperature (°C) and relative humidity (%) and has 2 controller:
//              A water level ON/OFF controller with SRF05 sensor.
//              A CO2 ON/OFF controller.
//              It also gathers the date and time info from the RTC sensor and reports it
//              The total current of the prototype is acquired through a WCS1800 sensor. (New Implementation V10)
//              All the sensors information is cleaned through a moving average of 20 samples
//Author:       David Velasquez
//Version:      11.0
//Date:         17/08/2018

//Library definition
#include "DHT.h"
#include <SPI.h>
//#include <SD.h>
#include <Wire.h>
#include "DS1307.h"

//I/O pin definition
#define SPCMAX 10 //Setpoint de corriente de temperatura para el maxthermo
#define CO2SEN 2  //CO2 sensor on pin A2
#define WCS1800C 6 //Current sensor for central on A6
#define WCS1800P2 0//Current sensor for plant 2 on A0
#define WCS1800P1 7 //Current sensor for plant 1 on A7
#define DHTPIN 9  //DHT sensor on pin digital 9
#define ENCFANIN 18 //Hall sensor for Fan In on digital pin D3
#define ENCFANOUT 19 //Hall sensor for Fan Out on digital pin D18
#define FANIN 5 //Fan In on pin digital 30
#define FANOUT 4  //Fan Out on pin digital 31
#define WLECHO 6  //Ultrasonic water level on pin D4
#define WLTRIG 7 //Ultrasonic water level trig on pin D24
#define WTPIN 3  //Water temperature on pin A3
#define WLINVALVE 26 //Water level in valve in pin D26
#define WTOUT 31 //Water resistor SSR out
#define chipSelect 9 //CS pin for microSD Shield on pin D8

//Library definitions and variables
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
DS1307 clock;//define a object of DS1307 class

//Constants definition
const float DC_GAIN = 8.5;  //define the DC gain of amplifier CO2 sensor
//const float ZERO_POINT_VOLTAGE = 0.4329; //define the output of the sensor in volts when the concentration of CO2 is 400PPM
const float ZERO_POINT_VOLTAGE = 0.265; //define the output of the sensor in volts when the concentration of CO2 is 400PPM
const float REACTION_VOLTAGE = 0.059;   //define the “voltage drop” of the sensor when move the sensor from air into 1000ppm CO2
const unsigned long tsamr = 3000; //Sampling time 3 secs for reading
const float wtadc_min = 50; //Minimum ADC for temperature
const float wtadc_max = 920; //Maximum ADC for temperature
const float wto_min = 20; //Minimum temperature
const float wto_max = 35;  //Maximum temperature
const float wli_min = 20; //Minimum water level
const float wli_max = 41; //Maximum water level
const float wlo_min = 0; //Minimum water level
const float wlo_max = 25; //Maximum water level
const int numReadings = 40;
const int numReadingsCO2 = 5;

//Variable definitions
volatile unsigned long halfRevFanIn = 0;  //Variable for counting pulses for Fan In Hall sensor
volatile unsigned long halfRevFanOut = 0; //Variable for counting pulses for Fan Out Hall sensor
unsigned int rpmFanIn = 0;  //Variable for storing rpm for Fan In
unsigned int rpmFanOut = 0; //Variable for storing rpm for Fan Out
unsigned int rpmFanInF = 0;  //Variable for storing rpm filtered for Fan In
unsigned int rpmFanOutF = 0; //Variable for storing rpm filtered for Fan Out
float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))}; //Line curve with 2 points
int CO2 = 0; //Define co2 as int for CO2 concentration in ppm
float wl = 0;  //Define wl as float for water level in cm
float t = 25;  //Define t as float for ambient temperature in °C
float tp = 25;  //Define tp as float for previous ambient temperature in °C
float wt = 0; //Define wt as float for water temperature in °C
float h = 30;  //Define h as float for humidity in %
float hp = 30;  //Define hp as float for previous humidity in %
float ewl = 0; //Define ewl as float for water level error
float IC = 0; //Define IC as float for Central current in Amps
float IP1 = 0;  //Define IP1 as float for Plant 1 current in Amps
float IP2 = 0;  //Define IP2 as float for Plant 2 current in Amps
int eCO2 = 0; //Define et as int for CO2 concentration error
float spwl = 0;  //Define spwl as float for water level setpoint and initialize it on 15 cm
float spwt = 30;  //Define spwt as float for water temperature setpoint and initialize it on 30°C
int spCO2 = 600; //Define spCO2 as int for CO2 concentration initialize it on 1000 ppm
double InputFANIN = 0, InputFANOUT = 0, InputCO2 = 0, Inputwl = 0, Inputt = 0, Inputwt = 0, Inputh = 0, InputIC = 0, InputIP1 = 0, InputIP2 = 0;
unsigned int WLINSTATE = 0, WTOUTSTATE = 0, FANINSTATE = 0, FANOUTSTATE = 0;
//Vars for smoothing
int readingsFANIN[numReadings] = {0}, readingsFANOUT[numReadings] = {0}, readingsCO2[numReadingsCO2] = {0}, readingswl[numReadings] = {0}, readingst[numReadings] = {0}, readingswt[numReadings] = {0}, readingsh[numReadings] = {0}, readingsIC[numReadings] = {0}, readingsIP1[numReadings] = {0}, readingsIP2[numReadings] = {0};
int readIndexFANIN = 0, readIndexFANOUT = 0, readIndexCO2 = 0, readIndexwl = 0, readIndext = 0, readIndexwt = 0, readIndexh = 0, readIndexIC = 0, readIndexIP1 = 0, readIndexIP2 = 0;
long totalFANIN = 0, totalFANOUT = 0, totalCO2 = 0, totalwl = 0, totalt = 0, totalwt = 0, totalh = 0, totalIC = 0, totalIP1 = 0, totalIP2 = 0;
char readbuffer[5] = {'\0'};

//Variable time definitions
//tinir and tstatr for the sampling time of the sensors
unsigned long tact = 0;
unsigned long tinir = 0;
unsigned long tstatr = 0;
unsigned long tiniFanIn = 0;
unsigned long tFanIn = 0;
unsigned long tiniFanOut = 0;
unsigned long tFanOut = 0;
unsigned long tiniCO2 = 0;
unsigned long tCO2 = 0;
unsigned long tiniwl = 0;
unsigned long twl = 0;

//Subroutines and functions
float flmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return constrain((float)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min), out_min, out_max);
}

unsigned int smooth(int meas, long &total, int *readings, int &readIndex, char type) {
  // subtract the last reading
  total = total - readings[readIndex];
  // read from the sensor:
  if (type == 'A') {
    readings[readIndex] = analogRead(meas);
  }
  else if (type == 'M') {
    readings[readIndex] = meas;
  }
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  return total / numReadings;
}

unsigned int smoothCO2(int pin, long &tot, int *reads, int &readInd) {
  // subtract the last reading
  tCO2 = tact - tiniCO2;
  if (tCO2 >= 20) {
    tot = tot - reads[readInd];
    // read from the sensor:
    reads[readInd] = analogRead(pin);
    // add the reading to the total:
    tot = tot + reads[readInd];
    // advance to the next position in the array:
    readInd = readInd + 1;

    // if we're at the end of the array...
    if (readInd >= numReadingsCO2) {
      // ...wrap around to the beginning:
      readInd = 0;
    }
    tiniCO2 = millis();
  }
  // calculate the average:
  return tot / numReadingsCO2;
}

float sultra(byte echop, byte trigp) {
  twl = tact - tiniwl;
  if (twl >= 50) {
    digitalWrite(trigp, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigp, LOW);
    tiniwl = millis();
    return pulseIn(echop, HIGH) / 58.0;
  }
  return Inputwl;
}

void countRevFanIn() {
  halfRevFanIn++;
}

void countRevFanOut() {
  halfRevFanOut++;
}

void RPMIn() {
  attachInterrupt(digitalPinToInterrupt(ENCFANIN), countRevFanIn, FALLING);
  tFanIn = tact - tiniFanIn;
  if (tFanIn >= 1000) {
    detachInterrupt(digitalPinToInterrupt(ENCFANIN));
    rpmFanIn = (halfRevFanIn / 2.0) * 60.0; //(hall sensor revolutions / 2) * 60 (secs to mins factor)
    halfRevFanIn = 0; //Reset counter
    tiniFanIn = millis();  //Reset timing
  }
}

void RPMOut() {
  attachInterrupt(digitalPinToInterrupt(ENCFANOUT), countRevFanOut, FALLING);
  tFanOut = tact - tiniFanOut;
  if (tFanOut >= 1000) {
    detachInterrupt(digitalPinToInterrupt(ENCFANOUT));
    rpmFanOut = (halfRevFanOut / 2.0) * 60.0; //(hall sensor revolutions / 2) * 60 (secs to mins factor)
    halfRevFanOut = 0; //Reset counter
    tiniFanOut = millis();  //Reset timing
  }
}

int readCO2(double ADCCO2, float *pcurve) {
  float volts = ADCCO2 * 5.0 / 1023.0;  //Convert CO2 ADC to volts
  if ((volts / DC_GAIN) >= ZERO_POINT_VOLTAGE) {
    return -1;
  }
  else {
    return pow(10, ((volts / DC_GAIN) - pcurve[1]) / pcurve[2] + pcurve[0]);
  }
}

void readsens() { //Read sensors information and store it in variables
  RPMIn();
  RPMOut();
  InputFANIN = smooth(rpmFanIn, totalFANIN, readingsFANIN, readIndexFANIN, 'M');
  InputFANOUT = smooth(rpmFanOut, totalFANOUT, readingsFANOUT, readIndexFANOUT, 'M');
  InputCO2 = smoothCO2(CO2SEN, totalCO2, readingsCO2, readIndexCO2);
  //Inputwl = smooth(sultra(WLECHO, WLTRIG), totalwl, readingswl, readIndexwl, 'M');
  Inputwl = sultra(WLECHO, WLTRIG);
  //Inputt = smooth(WLPIN, totall, readingsl, readIndexl);
  Inputwt = smooth(WTPIN, totalwt, readingswt, readIndexwt, 'A');
  //Inputh = smooth(WLPIN, totall, readingsl, readIndexl);
  InputIC = smooth(WCS1800C, totalIC, readingsIC, readIndexIC, 'A');
  InputIP1 = smooth(WCS1800P1, totalIP1, readingsIP1, readIndexIP1, 'A');
  InputIP2 = smooth(WCS1800P2, totalIP2, readingsIP2, readIndexIP2, 'A');
  tstatr = tact - tinir;
  if (tstatr >= tsamr) {
    if (digitalRead(WTOUT) == HIGH) {
      WTOUTSTATE = 1;
    }
    else {
      WTOUTSTATE = 0;
    }
    rpmFanInF = InputFANIN;
    rpmFanOutF = InputFANOUT;
    CO2 = constrain(readCO2(InputCO2, CO2Curve), 400, 10000);
    wl = flmap(Inputwl, wli_max, wli_min, wlo_min, wlo_max);
    //wl = 25.0-(constrain(Inputwl, wlmin, wlmax) - wlmin) * 25.0 / (wlmax - wlmin);
    //wl = abs(844 - constrain(Inputwl, 0, 844)) * 25.0 / 844.0;
    t = dht.readTemperature();
    wt = flmap(Inputwt, wtadc_min, wtadc_max, wto_min, wto_max);
    //wt = (float)((((Inputwt - vtmin) * (wtmax - wtmin)) / (vtmax - vtmin)) + wtmin);
    //wt = analogRead(WTPIN);
    h = dht.readHumidity();
    IC = constrain(0.0952 * InputIC - 49.732, 0, 30);
    IP1 = constrain(0.0952 * InputIP1 - 49.732, 0, 30);
    IP2 = constrain(0.088 * InputIP2 - 44.972, 0, 30);

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t)) {
      t = tp;
      //Serial.println("Failed to read from DHT");
    }
    else if (isnan(h)) {
      h = hp;
    }
    else {
      tp = t;
      hp = h;
    }
    //printsens();
    //RTC get time
    clock.getTime();

    //SD Logging
    String datalog = ""; //Define datalog as string for storing data in SD as text
    datalog += String(clock.hour, DEC);
    datalog += ":";
    datalog += String(clock.minute, DEC);
    datalog += ":";
    datalog += String(clock.second, DEC);
    datalog += ",";
    datalog += String(clock.dayOfMonth, DEC);
    datalog += "/";
    datalog += String(clock.month, DEC);
    datalog += "/";
    datalog += String(clock.year, DEC);
    datalog += ",";
    datalog += String(spwl, 4);
    datalog += ",";
    datalog += String(wl, 4);
    datalog += ",";
    datalog += String(WLINSTATE, DEC);
    datalog += ",";
    datalog += "30";
    datalog += ",";
    datalog += String(wt, 4);
    datalog += ",";
    datalog += String(WTOUTSTATE, DEC);
    datalog += ",";
    datalog += String(t, 4);
    datalog += ",";
    datalog += String(h, 4);
    datalog += ",";
    datalog += String(spCO2, DEC);
    datalog += ",";
    datalog += String(CO2, DEC);
    datalog += ",";
    datalog += String(FANINSTATE, DEC);
    datalog += ",";
    datalog += String(FANOUTSTATE, DEC);
    datalog += ",";
    datalog += String(rpmFanInF, DEC);
    datalog += ",";
    datalog += String(rpmFanOutF, DEC);
    datalog += ",";
    datalog += String(IC, 4);
    datalog += ",";
    datalog += String(IP1, 4);
    datalog += ",";
    datalog += String(IP2, 4);
    //    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    //    // if the file is available, write to it:
    //    if (dataFile) {
    //      dataFile.println(datalog);
    //      dataFile.close();
    //      // print to the serial port too:
    //      //Serial.println("Success logging data");
    //    }
    //    // if the file isn't open, pop up an error:
    //    else {
    //      //Serial.println("error opening datalog.txt");
    //    }
    tinir = millis();
  }
}

void printsens() { //Prints sensors information on Serial monitor
  Serial.print(clock.hour, DEC);
  Serial.print(':');
  Serial.print(clock.minute, DEC);
  Serial.print(':');
  Serial.print(clock.second, DEC);
  Serial.print(' ');
  Serial.print(clock.dayOfMonth, DEC);
  Serial.print('/');
  Serial.print(clock.month, DEC);
  Serial.print('/');
  Serial.print(clock.year, DEC);
  Serial.print(" Water Level: ");
  Serial.print(wl);
  Serial.print(" cm Ambient Temperature: ");
  Serial.print(t);
  Serial.print(" *C Water Temperature: ");
  Serial.print(wt);
  Serial.print(" *C Relative Humidity: ");
  Serial.print(h);
  Serial.print(" % WTOUTSTATE: ");
  Serial.print(WTOUTSTATE);
  Serial.print(" CO2: ");
  Serial.print(CO2);
  Serial.print(" ppm Fan In state: ");
  Serial.print(FANINSTATE);
  Serial.print(" Fan Out state: ");
  Serial.print(FANOUTSTATE);
  Serial.print(" RPM Fan In: ");
  Serial.print(rpmFanInF);
  Serial.print(" RPM Fan Out: ");
  Serial.print(rpmFanOutF);
  Serial.print(" Central Current: ");
  Serial.print(IC);
  Serial.print(" A Plant 1 Current: ");
  Serial.print(IP1);
  Serial.print(" A Plant 2 Current: ");
  Serial.print(IP2);
  Serial.println(" A");
}

void wlevelctrl() { //Water Level Controller
  ewl = spwl - wl;
  if (ewl < -0.5) {  //If the water level error is less than -2 cm, turn off water valve
    digitalWrite(WLINVALVE, LOW);
    WLINSTATE = 0;
  }
  else if (ewl > 0.5) { //else if the water level is greater than 4 cm, turn on water in valve
    digitalWrite(WLINVALVE, HIGH);
    WLINSTATE = 1;
  }
  else { //In other case, keep the water in valve off
    digitalWrite(WLINVALVE, LOW);
    WLINSTATE = 0;
  }
}

void CO2ctrl() { //CO2 Controller
  eCO2 = spCO2 - CO2;
  if (eCO2 < -20) {  //If the CO2 error is less than -100 ppm, turn off FANIN turn on FANOUT
    digitalWrite(FANIN, LOW);
    digitalWrite(FANOUT, HIGH);
    FANINSTATE = 0;
    FANOUTSTATE = 1;
  }
  else if (eCO2 > 100) { //else If the CO2 error is greater than 100 ppm, turn on FANIN turn off FANOUT
    digitalWrite(FANIN, HIGH);
    digitalWrite(FANOUT, LOW);
    FANINSTATE = 1;
    FANOUTSTATE = 0;
  }
  else { //In other case, keep the fans off
    digitalWrite(FANIN, LOW);
    digitalWrite(FANOUT, LOW);
    FANINSTATE = 0;
    FANOUTSTATE = 0;
  }
}

//void SDinitialize() {
//  //Serial.print("Initializing SD card...");
//
//  // see if the card is present and can be initialized:
//  if (!SD.begin(chipSelect)) {
//    //Serial.println("Card failed, or not present");
//    // don't do anything more:
//    return;
//  }
//  //Serial.println("card initialized.");
//}

void MeasInitialize() {
  for (unsigned int i = 0; i < 80; i++) {
    InputFANIN = smooth(rpmFanIn, totalFANIN, readingsFANIN, readIndexFANIN, 'M');
    InputFANOUT = smooth(rpmFanOut, totalFANOUT, readingsFANOUT, readIndexFANOUT, 'M');
    constrain(readCO2(InputCO2, CO2Curve), 400, 10000);
    Inputwl = sultra(WLECHO, WLTRIG);
    //Inputwl = smooth(sultra(WLECHO, WLTRIG), totalwl, readingswl, readIndexwl, 'M');
    Inputwt = smooth(WTPIN, totalwt, readingswt, readIndexwt, 'A');
    InputIC = smooth(WCS1800C, totalIC, readingsIC, readIndexIC, 'A');
    InputIP1 = smooth(WCS1800P1, totalIP1, readingsIP1, readIndexIP1, 'A');
    InputIP2 = smooth(WCS1800P2, totalIP2, readingsIP2, readIndexIP2, 'A');
  }
  rpmFanInF = InputFANIN;
  rpmFanOutF = InputFANOUT;
  CO2 = constrain(readCO2(InputCO2, CO2Curve), 400, 10000);
  wl = flmap(Inputwl, wli_max, wli_min, wlo_min, wlo_max);
  //wl = 25.0-(constrain(Inputwl, wlmin, wlmax) - wlmin) * 25.0 / (wlmax - wlmin);
  //wl = abs(844 - constrain(Inputwl, 0, 844)) * 25.0 / 844.0;
  wt = flmap(Inputwt, wtadc_min, wtadc_max, wto_min, wto_max);
  //wt = (float)((((Inputwt - vtmin) * (wtmax - wtmin)) / (vtmax - vtmin)) + wtmin);
  IC = constrain(0.0952 * InputIC - 49.732, 0, 30);
  IP1 = constrain(0.0952 * InputIP1 - 49.732, 0, 30);
  IP2 = constrain(0.088 * InputIP2 - 44.972, 0, 30);
}

void labviewcomm() {
  char writebuffer[15] = {flmap(Inputwl, wli_max, wli_min, wlo_min, wlo_max) * 255.0 / 25.0, WLINSTATE * 255, Inputwt * 255.0 / 1023.0, WTOUTSTATE * 255, t * 255.0 / 50.0, h * 255.0 / 100.0, constrain(readCO2(InputCO2, CO2Curve), 400, 10000) * 255.0 / 10000.0, FANINSTATE * 255, FANOUTSTATE * 255, constrain(InputFANIN * 255.0 / 3000.0, 0, 255), constrain(InputFANOUT * 255.0 / 3000.0, 0, 255), InputIC * 255.0 / 1023.0, InputIP1 * 255.0 / 1023.0, InputIP2 * 255.0 / 1023.0,'\0'};
  //   writebuffer[12] = {WATER LEVEL,WATER VALVE IN STATE, WATER TEMPERATURE, SSR RESISTOR STATE, AMBIENT TEMPERATURE, RELATIVE HUMIDITY, CO2, FAN IN STATE, FAN OUT STATE, RPM FAN IN, RPM FAN OUT, END OF LINE};
  if (Serial.available() > 0) {
    Serial.readBytesUntil('\n', readbuffer, 5);
    Serial.flush();
    if (readbuffer[0] == 'B') {
      spwl = (byte)readbuffer[1] * (25.0 / 255.0);
      analogWrite(SPCMAX,(byte)readbuffer[2]);
      spwt = (byte)readbuffer[2] * (100.0 / 255.0);
      spCO2 = (byte)readbuffer[3] * (10000.0 / 255.0);
      for (unsigned int i = 0; i < 14; i++) {
        if ((byte)writebuffer[i] == 10 || (byte)writebuffer[i] == 0)writebuffer[i] = writebuffer[i] + 1;
      }
      Serial.println(writebuffer);
      readbuffer[0] == '\0';
    }
  }
}

void setup() {
  //Pin configuration
  pinMode(ENCFANIN, INPUT_PULLUP);
  pinMode(ENCFANOUT, INPUT_PULLUP);
  pinMode(WLINVALVE, OUTPUT);
  pinMode(WLECHO, INPUT);
  pinMode(WLTRIG, OUTPUT);
  pinMode(WTOUT, INPUT);
  pinMode(FANIN, OUTPUT);
  pinMode(FANOUT, OUTPUT);
  pinMode(SPCMAX, OUTPUT);

  //Output cleaning
  digitalWrite(WLTRIG, LOW);
  digitalWrite(WLINVALVE, LOW);
  digitalWrite(FANIN, LOW);
  digitalWrite(FANOUT, LOW);
  WLINSTATE = 0;
  FANINSTATE = 0;
  FANOUTSTATE = 0;

  //Communications
  Serial.begin(9600);
  //Serial.println("DHTxx test!");

  //SDinitialize();
  clock.begin();
  dht.begin();
  MeasInitialize();
  //  clock.fillByYMD(2016, 5, 5); //Jan 19,2013
  //  clock.fillByHMS(15, 50, 30); //15:28 30"
  //  clock.fillDayOfWeek(THU);//Saturday
  //  clock.setTime();//write time to the RTC chip

  //Interrupciones
  attachInterrupt(digitalPinToInterrupt(ENCFANOUT), countRevFanOut, FALLING); //Interrupt #4 (Rising edge on pin 19) for Fan Out hall sensor
  attachInterrupt(digitalPinToInterrupt(ENCFANIN), countRevFanIn, FALLING);  //Interrupt #5 (Rising edge on pin 18) for Fan In hall sensor

  //Reseteo tini
  tiniFanOut = millis();
  tiniFanIn = millis();
  tinir = millis();
  tiniCO2 = millis();
  tiniwl = millis();
}

void loop() {
  tact = millis();
  readsens(); //Read sensors
  labviewcomm();
  wlevelctrl(); //Activate water level controller
  CO2ctrl(); //Activate temperature controller
  analogWrite(SPCMAX, 0);
}
