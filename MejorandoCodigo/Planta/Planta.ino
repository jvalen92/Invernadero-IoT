/*
   Este código es para el monitoreo y control de los sensores y actuadores de una planta en el inverdanero iot que está desarrollando el semillero de investigación en sistemas embebidos.
   Este códog se basa en el código del profesor David Velasquez, del departamento de ingeniería de sistemas de la universidad Eafit.
   Fecha: Septiembre 2018
   Convenciones:
   - sp = setPoitnt(valor deseado). P.e: spLuzBlanca = valor deseado de luz Blanca
   - Se omite el uso de las preposiciones como "de", "del", "de la", en los nombres de las variables. 
     P.e: luzInfrarrojaDeEntrada es en nuestro código luzInfrarrojaEntrada.
*/

//Definición de librerías
#include <math.h> //Libreria de matematicas para la función logaritmo
#include <PID_v1.h> //Libreria de PID para control de iluminacion
#include <SPI.h>  //Libreria para comunicacion SPI con la tarjeta SD (no se necesita SISE)
#include <SD.h> //Libreria para SD (no SISE)
#include <Wire.h> //Libreria para comunicarse DS1307 reloj de tiempo real
#include "DS1307.h" //Libreria del reloj de tiempo real (RTC)
#include "Arduino.h" //Se añade en el ejemplo del sunlight sensor
#include "SI114X.h" //Libreria del Sunlight Sensor (uv,  luzInfrarrojay Visible SEEED Studio Groove)

//Definicón de pines
#define LDR 0 //Fotocelda para control de iluminacion visible blanca
#define SOIL 1  //Sensor de humedad de la tierra
#define PH 2 //Sensor de ph
#define LM35 3  //Sensor de temperatura del suelo
#define WVALVE 4  //Electrovalvula para irrigacion
#define IRLED 5  //LEDs infrarojos en pin 5
#define PLED 6  //LEDs blancos de potencia
#define CS 8 //CS pin for microSD Shield on pin D8 (no SISE)
#define UVLED 18 // FALTA POR IMPLEMENTAR

//Definición de constantes
const unsigned long TSAMR = 3000; //Periodo de muestreo para almacenar variables
const float SOIL_MADC_AIR = 520; //Valor ADC en el aire para calibrar el sensor de humedad del suelo
const float SOIL_MADC_WATER = 286; //Valor ADC en el agua para calibrar el sensor de humedad del suelo
const float SOIL_MO_AIR = 0; //Valor minimo (seco o al aire) deseado escalado de humedad del suelo (0%)
const float SOIL_MO_WATER = 100;  //Valor maximo (humedo) deseado esacalado de humedad del suelo (100%)
double CONSKP = 0.02, CONSKI = 1, CONSKD = 0.00001; //Constantes proporcional, integral y derivativa para el PID de iluminacion
const float OFFSET = 0.00;  //OFFSET de voltaje para el sensor de ph
const unsigned long MAXTPH = 20;  //Tiempo de sampling del sensor de ph (min 20 ms)
const int NUMREADS = 40;  //Maximo de muestras a medir para la media movil (longitud del vector)

//Definición de variables
float luzBlanca = 0,  //Luz blanca (luz visible)
      luzInfrarroja = 0, //Luz infraroja
      luzUltravioleta = 0, //Luz UV
      ph = 0, //Valor del ph
      humedadSuelo = 0, //Valor de humedad del suelo (moisture)
      temperaturaSuelo = 0,  //Temperatura del suelo (medida con el LM35)
      
      errorIluminacion = 0, //Error de iluminacion
      errorHumedadSuelo = 0, //Error de humedad de la tierra

      spLuzBlanca = 300,  //Set-point (Valor deseado) de iluminacion blanca (valor por defecto en 300 lux)
      spHumedadSuelo = 50;  //Set-point (Valor deseado) de humedad del suelo (valor por defeto en 50%)

unsigned int hrs = 0; //Variable para almacenar las horas (HOUR) del reloj de tiempo real
unsigned long grad = 0; //Define grad as unsigned long for storing the servo degree position (no SISE)

//Variables para media movil y control PID
double spLuz = 600,
       luzEntrada = 0,
       humedadSueloEntrada = 0,
       temperaturaEntrada = 0,
       phEntrada = 0,
       luzInfrarrojaEntrada = 0,
       luzSalida = 0;

unsigned int estadoValvula = 0; //Variable para almacenar el estado de la válvula de irrigacion

//Variables para media movil
int lecturaLuz[NUMREADS] = {0},
    lecturaHumedadSuelo[NUMREADS] = {0}, 
    lecturaTemperatura[NUMREADS] = {0}, 
    lecturaPh[NUMREADS] = {0}, 
    lecturaInfrarroja[NUMREADS] = {0};
    
int indiceLuz = 0, 
    indiceHumedadSuelo = 0, 
    indiceTemperatura = 0, 
    indicePh = 0, 
    indiceInfrarroja = 0;

long totalLuz = 0,
     totalHumedadSuelo = 0, 
     totalTemperatura = 0, 
     totalPH = 0, 
     totalIR = 0;

char lectura[4] = {'\0'};  //Vector recibido desde el computador (para mejorar SISE desde RaspberryPI)

//Variables para el tiempo
unsigned long tiempoActual = 0; //Tiempo actual de todo millis()
//tiempoInicial y tiempoRelativo para el tiempo de muestreo de los sensores
unsigned long tiempoInicial = 0;  //Tiempo inicial para procesamiento (conversion) de las variables de sensores
unsigned long tiempoRelativo = 0; //Tiempo relativo para procesamiento (conversion) de las variables de sensores
unsigned long tiempoInicialPh = 0; //Tiempo inicial de pH
unsigned long tiempoRelativoPh = 0;  //Tiempo relativo de muestreo del sensor pH

//Definiciones e inicializaciones para librerías
PID myPID(&luzEntrada, &luzSalida, &spLuz, CONSKP, CONSKI, CONSKD, DIRECT);
DS1307 clock;//Objeto para el uso de los métodos del RTC
SI114X SI1145 = SI114X(); //Objeto para el sensor de luz

//Métodos y subrutinas

float flmap(float x, float in_min, float in_max, float out_min, float out_max) { //Sirve para mapeo lineal de variables (recordar semejanza de triangulos para escalar)
  return constrain((float)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min), out_min, out_max);
}

unsigned long smoothPH(int pin, long &total, int *readings, int &readIndex) {
  tiempoRelativoPh = tiempoActual - tiempoInicialPh;
  if (tiempoRelativoPh >= MAXTPH) {
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
    tiempoInicialPh = millis();
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

void leerSensores() { //Read sensors information and store it in variables
  luzEntrada = smooth(LDR, totalLuz, lecturaLuz, indiceLuz);
  humedadSueloEntrada = smooth(SOIL, totalHumedadSuelo, lecturaHumedadSuelo, indiceHumedadSuelo);
  temperaturaEntrada = smooth(LM35, totalTemperatura, lecturaTemperatura, indiceTemperatura);
  phEntrada = smoothPH(PH, totalPH, lecturaPh, indicePh);
  luzInfrarrojaEntrada = smoothIR(totalIR, lecturaInfrarroja, indiceInfrarroja);
  tiempoRelativo = tiempoActual - tiempoInicial;
  if (tiempoRelativo >= TSAMR) {
    //luzEntrada = analogRead(LDR);
    luzUltravioleta = SI1145.ReadUV() / 100.0; //Para que tome nuevamente los valores de UV IR
    luzInfrarroja = luzInfrarrojaEntrada;
    luzBlanca  = luzEntrada * 5.0 / 1023.0;  //Convierto a voltios la iluminacion blanca
    luzBlanca  = 13.788 * exp(1.3413 *   luzBlanca ); //Convierto o proceso la iluminacion en lumens
    //m = constrain(humedadSueloEntrada * 100.0 / 845.0, 0, 100);
    humedadSuelo = flmap(humedadSueloEntrada, SOIL_MADC_AIR, SOIL_MADC_WATER, SOIL_MO_AIR, SOIL_MO_WATER);
    //m = (float)((((humedadSueloEntrada - vtmin) * (mmax - mmin)) / (vtmax - vtmin)) + mmin);
    humedadSuelo = constrain(humedadSuelo, 0, 100);
    //m = analogRead(SOIL);
    temperaturaSuelo = temperaturaEntrada * 500.0 / 1023.0;
    ph = phEntrada * 5.0 * 3.5 / 1023.0 + OFFSET;
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
    datalog += String(spLuzBlanca, 4);
    datalog += ",";
    datalog += String(luzBlanca, 4);
    datalog += ",";
    datalog += String(luzSalida, DEC);
    datalog += ",";
    datalog += String(spHumedadSuelo, 4);
    datalog += ",";
    datalog += String(humedadSuelo, 4);
    datalog += ",";
    datalog += String(estadoValvula, DEC);
    datalog += ",";
    datalog += String(temperaturaSuelo, 4);
    datalog += ",";
    datalog += String(luzUltravioleta, 4);
    datalog += ",";
    datalog += String(luzInfrarroja, 4);
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
    tiempoInicial = millis();
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
  Serial.print(luzBlanca);
  Serial.print(" lux Soil Moisture: ");
  Serial.print(humedadSuelo);
  Serial.print(" % Valve: ");
  Serial.print(estadoValvula);
  Serial.print(" Soil Temperature: ");
  Serial.print(temperaturaSuelo);
  Serial.print(" *C luzInfrarroja Sensor: ");
  Serial.print(luzInfrarroja);
  Serial.print(" lux luzUltravioleta Sensor: ");
  Serial.print(luzUltravioleta);
  Serial.print(" ph Sensor: ");
  Serial.println(ph);
}

void lightctrl() {  //Light controller subroutine
  spLuz = constrain(151.04 * log(spLuzBlanca) - 390.08, 0, 1023);
  myPID.Compute();
  analogWrite(PLED, luzSalida);
  analogWrite(IRLED, luzSalida * 200.0 / 255.0);
}

void shumidctrl() { //Soil Humidity Controller
  errorHumedadSuelo = spHumedadSuelo - humedadSuelo;
  if (errorHumedadSuelo < -2) {  //If the soil moisture is above set point (neg error), turn off water valve
    digitalWrite(WVALVE, LOW);
    estadoValvula = 0;
  }
  else if (errorHumedadSuelo > 2) { //else if the soil moisture is below set point (pos error), turn on water valve
    digitalWrite(WVALVE, HIGH);
    estadoValvula = 1;
  }
  else { //if the error is between -10% and 10%, keep the water valve off
    digitalWrite(WVALVE, LOW);
    estadoValvula = 0;
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
    luzEntrada = smooth(LDR, totalLuz, lecturaLuz, indiceLuz);
    humedadSueloEntrada = smooth(SOIL, totalHumedadSuelo, lecturaHumedadSuelo, indiceHumedadSuelo);
    temperaturaEntrada = smooth(LM35, totalTemperatura, lecturaTemperatura, indiceTemperatura);
    phEntrada = smooth(PH, totalPH, lecturaPh, indicePh);
  }
  luzBlanca  = luzEntrada * 5.0 / 1023.0;
  luzBlanca  = 13.788 * exp(1.3413 * luzBlanca);
  humedadSuelo = flmap(humedadSueloEntrada, SOIL_MADC_AIR, SOIL_MADC_WATER, SOIL_MO_AIR, SOIL_MO_WATER);
  //m = constrain(humedadSueloEntrada * 100.0 / 845.0, 0, 100);
  temperaturaSuelo = temperaturaEntrada * 500.0 / 1023.0;
  ph = phEntrada * 5.0 * 3.5 / 1023.0 + OFFSET;
}

void setup() {
  //Pin configuration
  pinMode(WVALVE, OUTPUT);
  pinMode(PLED, OUTPUT);
  pinMode(IRLED, OUTPUT);

  //luzSalida cleaning
  digitalWrite(WVALVE, LOW);
  estadoValvula = 0;
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
  tiempoInicial = millis();
  tiempoInicialPh = millis();
}

void loop() {
  tiempoActual = millis();
  leerSensores();
  lightctrl();  //Activate light controller
  shumidctrl(); //Activate soil humidity controller
  //analogWrite(5, 200);
}