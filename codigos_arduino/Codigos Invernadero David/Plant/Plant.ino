//Name:         Plant
//Description:  This program samples data from LDR (lux), DFRobot Soil Moisture Sensor (%), Sunlight Sensor (ir and uv) and LM35 (°C). It integrates two (2) closed loop controllers:
//              A PID controller for light intensity: Depending on the luminous flux (lux), it powers the LEDs to control its light.
//              An ON/OFF controlller for soil humidity: Depending on the soil moisture % it automatically dispenses water to the soil using a Hydraulic Electrovalve.
//              It also gathers the date and time info from the RTC sensor and reports it.
//              All the sensors information is cleaned through a moving average of 20 samples
//              ph sensor added and transmission added
//Author:       David Velasquez
//Version:      10.0
//Date:         17/09/2018

#include <math.h> //Libreria de matematicas para la función logaritmo
#include <PID_v1.h> //Libreria de PID para control de iluminacion
#include <SPI.h>  //Libreria para comunicacion SPI con la tarjeta SD (no se necesita SISE)
#include <SD.h> //Libreria para SD (no SISE)
#include <Wire.h> //Libreria para comunicarse DS1307 reloj de tiempo real
#include "DS1307.h" //Libreria del RTC
#include "Arduino.h" //Se añade en el ejemplo del sunlight sensor
#include "SI114X.h" //Libreria del Sunlight Sensor (uv ir y Visible SEEED Studio Groove)

//I/O pin definition
#define LDR 0 //Fotocelda para control de iluminacion visible blanca
#define SOIL 1  //Sensor de humedad de la tierra
#define PH 2 //Sensor de ph
#define LM35 3  //Sensor de temperatura del suelo
#define WVALVE 4  //Electrovalvula para irrigacion
#define IRLED 5  //LEDs infrarojos en pin 5
#define PLED 6  //LEDs blancos de potencia
#define CS 8 //CS pin for microSD Shield on pin D8 (no SISE)

//Constants definition
const unsigned long TSAMR = 3000; //Periodo de muestreo para almacenar variables
const float SOIL_MADC_AIR = 520; //Valor ADC en el aire para calibrar el sensor de humedad del suelo
const float SOIL_MADC_WATER = 286; //Valor ADC en el agua para calibrar el sensor de humedad del suelo
const float SOIL_MO_AIR = 0; //Valor minimo (seco o al aire) deseado escalado de humedad del suelo (0%)
const float SOIL_MO_WATER = 100;  //Valor maximo (humedo) deseado esacalado de humedad del suelo (100%)
double CONSKP = 0.02, CONSKI = 1, CONSKD = 0.00001; //Constantes proporcional, integral y derivativa para el PID de iluminacion
const float OFFSET = 0.00;  //OFFSET de voltaje para el sensor de ph
const unsigned long MAXTPH = 20;  //Tiempo de sampling del sensor de ph (min 20 ms)
const int NUMREADS = 40;  //Maximo de muestras a medir para la media movil (longitud del vector)

//Variable definitions
float l = 0;  //Variable para almacenar la iluminacion visible (blanca)
float ir = 0; //Variable de luz infraroja
float uv = 0; //Luz UV
float ph = 0; //Valor del ph
float m = 0; //Valor de humedad del suelo (moisture)
float t = 0;  //Temperatura del suelo (medida con el LM35)
float el = 0; //Error de iluminacion
float esh = 0; //Error de humedad de la tierra
unsigned long grad = 0; //Define grad as unsigned long for storing the servo degree position (no SISE)
float spl = 300;  //Set-point (Valor deseado) de iluminacion blanca (default en 300 lux)
float spm = 50;  //Set-point (Valor deseado) de humedad del suelo (default 50%)
unsigned int hrs = 0; //Variable para almacenar las horas (HOUR) del reloj de tiempo real
double setpointLight = 600, inputLight = 0, inputm = 0, inputTemp = 0, inputPH = 0, InputIR = 0, outputLightPID = 0; //Variables para media movil y PID
unsigned int wvalstate = 0; //Variable para almacenar el estado de la válvula de irrigacion
//Variables para media movil
int readingsl[NUMREADS] = {0}, readingsm[NUMREADS] = {0}, readingst[NUMREADS] = {0}, readingsPH[NUMREADS] = {0}, readingsIR[NUMREADS] = {0};
int readIndexl = 0, readIndexm = 0, readIndext = 0, readIndexPH = 0, readIndexIR = 0;
long totall = 0, totalm = 0, totalt = 0, totalPH = 0, totalIR = 0;;
char readbuffer[4] = {'\0'};  //Vector recibido desde el computador (para mejorar SISE desde RaspberryPI)

//Variable time definitions
//tinir and tstatr for the sampling time of the sensors
unsigned long tact = 0; //Tiempo actual de todo millis()
unsigned long tinir = 0;  //Tiempo inicial para procesamiento (conversion) de las variables de sensores
unsigned long tstatr = 0; //Tiempo relativo para procesamiento (conversion) de las variables de sensores
unsigned long tiniPH = 0; //Tiempo inicial de pH
unsigned long tPH = 0;  //Tiempo relativo de muestreo del sensor pH

//Library definitions
PID myPID(&inputLight, &outputLightPID, &setpointLight, CONSKP, CONSKI, CONSKD, DIRECT);
DS1307 clock;//define a object of DS1307 class
SI114X SI1145 = SI114X(); //Sunlight sensor class

//Subroutines and subfunctions
float flmap(float x, float in_min, float in_max, float out_min, float out_max) { //Sirve para mapeo lineal de variables (recordar semejanza de triangulos para escalar)
  return constrain((float)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min), out_min, out_max);
}

void readSun() {  // ir = SI1145.ReadIR();
  uv = SI1145.ReadUV() / 100.0;
}

unsigned long smoothPH(int pin, long &total, int *readings, int &readIndex) {
  tPH = tact - tiniPH;
  if (tPH >= MAXTPH) {
    // subtract the last reading
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = analogRead(pin);
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    // if we're at the end of the array...
    if (readIndex >= NUMREADS) {
      // ...wrap around to the beginning:
      readIndex = 0;
    }
    tiniPH = millis();
  }
  // calculate the average:
  return total / NUMREADS;
}

unsigned long smooth(int pin, long &total, int *readings, int &readIndex) {
  // subtract the last reading
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(pin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= NUMREADS) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  return total / NUMREADS;
}

unsigned long smoothIR(long &total, int *readings, int &readIndex) {
  // subtract the last reading
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = SI1145.ReadIR();
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= NUMREADS) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  return total / NUMREADS;
}

void readsens() { //Read sensors information and store it in variables
  inputLight = smooth(LDR, totall, readingsl, readIndexl);
  inputm = smooth(SOIL, totalm, readingsm, readIndexm);
  inputTemp = smooth(LM35, totalt, readingst, readIndext);
  inputPH = smoothPH(PH, totalPH, readingsPH, readIndexPH);
  InputIR = smoothIR(totalIR, readingsIR, readIndexIR);
  tstatr = tact - tinir;
  if (tstatr >= TSAMR) {
    //inputLight = analogRead(LDR);
    readSun(); //Para que tome nuevamente los valores de UV IR
    ir = InputIR;
    l = inputLight * 5.0 / 1023.0;  //Convierto a voltios la iluminacion blanca
    l = 13.788 * exp(1.3413 * l); //Convierto o proceso la iluminacion en lumens
    //m = constrain(inputm * 100.0 / 845.0, 0, 100);
    m = flmap(inputm, SOIL_MADC_AIR, SOIL_MADC_WATER, SOIL_MO_AIR, SOIL_MO_WATER);
    //m = (float)((((inputm - vtmin) * (mmax - mmin)) / (vtmax - vtmin)) + mmin);
    m = constrain(m, 0, 100);
    //m = analogRead(SOIL);
    t = inputTemp * 500.0 / 1023.0;
    ph = inputPH * 5.0 * 3.5 / 1023.0 + OFFSET;
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
    datalog += String(spl, 4);
    datalog += ",";
    datalog += String(l, 4);
    datalog += ",";
    datalog += String(outputLightPID, DEC);
    datalog += ",";
    datalog += String(spm, 4);
    datalog += ",";
    datalog += String(m, 4);
    datalog += ",";
    datalog += String(wvalstate, DEC);
    datalog += ",";
    datalog += String(t, 4);
    datalog += ",";
    datalog += String(uv, 4);
    datalog += ",";
    datalog += String(ir, 4);
    datalog += ",";
    datalog += String(ph, 4);
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(datalog);
      dataFile.close();
      // print to the serial port too:
      //Serial.println("Success logging data");
    }
    // if the file isn't open, pop up an error:
    else {
      //Serial.println("error opening datalog.txt");
    }
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
  Serial.print(" Luminous flux: ");
  Serial.print(l);
  Serial.print(" lux Soil Moisture: ");
  Serial.print(m);
  Serial.print(" % Valve: ");
  Serial.print(wvalstate);
  Serial.print(" Soil Temperature: ");
  Serial.print(t);
  Serial.print(" *C ir Sensor: ");
  Serial.print(ir);
  Serial.print(" lux uv Sensor: ");
  Serial.print(uv);
  Serial.print(" ph Sensor: ");
  Serial.println(ph);
}

void lightctrl() {  //Light controller subroutine
  setpointLight = constrain(151.04 * log(spl) - 390.08, 0, 1023);
  myPID.Compute();
  analogWrite(PLED, outputLightPID);
  analogWrite(IRLED, outputLightPID * 200.0 / 255.0);
}

void shumidctrl() { //Soil Humidity Controller
  esh = spm - m;
  if (esh < -2) {  //If the soil moisture is above set point (neg error), turn off water valve
    digitalWrite(WVALVE, LOW);
    wvalstate = 0;
  }
  else if (esh > 2) { //else if the soil moisture is below set point (pos error), turn on water valve
    digitalWrite(WVALVE, HIGH);
    wvalstate = 1;
  }
  else { //if the error is between -10% and 10%, keep the water valve off
    digitalWrite(WVALVE, LOW);
    wvalstate = 0;
  }
}

void SDinitialize() {
  //Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(CS)) {
    //Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  //Serial.println("card initialized.");
}

void MeasInitialize() {
  for (unsigned int i = 0; i < 80; i++) {
    inputLight = smooth(LDR, totall, readingsl, readIndexl);
    inputm = smooth(SOIL, totalm, readingsm, readIndexm);
    inputTemp = smooth(LM35, totalt, readingst, readIndext);
    inputPH = smooth(PH, totalPH, readingsPH, readIndexPH);
  }
  l = inputLight * 5.0 / 1023.0;
  l = 13.788 * exp(1.3413 * l);
  m = flmap(inputm, SOIL_MADC_AIR, SOIL_MADC_WATER, SOIL_MO_AIR, SOIL_MO_WATER);
  //m = constrain(inputm * 100.0 / 845.0, 0, 100);
  t = inputTemp * 500.0 / 1023.0;
  ph = inputPH * 5.0 * 3.5 / 1023.0 + OFFSET;
}

void labviewcomm() {
  char writebuffer[9] = {inputLight * 255.0 / 1023.0, outputLightPID, m * 255.0 / 100.0, wvalstate * 255, inputTemp * 255.0 / 1023.0, ir * 255.0 / 10000.0, constrain(uv, 0, 11) * 255.0 / 11.0, constrain(ph, 0, 14) * 255.0 / 14.0, '\0'};
  //char writebuffer[8] = {Lumens, Ouput, Moisture, wvalstate, inputTemp, ir, uv, ph, '\n'};
  if (Serial.available() > 0) {
    Serial.readBytesUntil('\n', readbuffer, 4);
    Serial.flush();
    if (readbuffer[0] == 'A') {
      spl = (byte)readbuffer[1] * (1000 / 255);
      setpointLight = constrain(151.04 * log(spl) - 390.08, 0, 1023);
      spm = (byte)readbuffer[2] * (100.0 / 255.0);
      for (unsigned int i = 0; i < 8; i++) {
        if ((byte)writebuffer[i] == 10 || (byte)writebuffer[i] == 0)writebuffer[i] = writebuffer[i] + 1;
      }
      Serial.println(writebuffer);
      readbuffer[0] = '\0';
    }
  }
}

void setup() {
  //Pin configuration
  pinMode(WVALVE, OUTPUT);
  pinMode(PLED, OUTPUT);
  pinMode(IRLED, OUTPUT);

  //outputLightPID cleaning
  digitalWrite(WVALVE, LOW);
  wvalstate = 0;
  digitalWrite(PLED, LOW);

  //Communications
  Serial.begin(9600);
  myPID.SetMode(AUTOMATIC);
  SDinitialize();
  clock.begin();
  while (!SI1145.Begin());
  MeasInitialize();
  //  clock.fillByYMD(2016, 3, 29); //Jan 19,2013
  //  clock.fillByHMS(16, 37, 15); //15:28 30"
  //  clock.fillDayOfWeek(THU);//Saturday
  //  clock.setTime();//write time to the RTC chip
  //tini initialization
  tinir = millis();
  tiniPH = millis();
}

void loop() {
  tact = millis();
  readsens(); //Read sensors
  labviewcomm();
  lightctrl();  //Activate light controller
  shumidctrl(); //Activate soil humidity controller
  //analogWrite(5, 200);
}
