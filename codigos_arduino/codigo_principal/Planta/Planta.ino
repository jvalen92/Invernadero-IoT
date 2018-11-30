/*
   Este código es para el monitoreo y control de los sensores y actuadores de una planta en el inverdanero iot que está desarrollando el semillero de investigación en sistemas embebidos.
   Este código se basa en el código del profesor David Velasquez, del departamento de ingeniería de sistemas de la universidad Eafit.
   Fecha: Septiembre 2018
   Convenciones:
   - sp = setPoitnt(valor deseado). P.e: spLuzBlanca = valor deseado de luz Blanca
   - Se omite el uso de las preposiciones como "de", "del", "de la", en los nombres de las variables.
     P.e: luzInfrarrojaDeEntrada es en nuestro código luzInfrarrojaEntrada.
*/

//Definición de librerías
#include <math.h> //Libreria de matematicas para la función logaritmo
#include <PID_v1.h> //Libreria de PID para control de iluminacion
#include <Wire.h> //Libreria para comunicarse DS1307 reloj de tiempo real
#include "DS1307.h" //Libreria del reloj de tiempo real (RTC)
#include "Arduino.h" //Se añade en el ejemplo del sunlight sensor
#include "SI114X.h" //Libreria del Sunlight Sensor (uv,  luzInfrarrojay Visible SEEED Studio Groove)

//Definicón de pines
#define LDR 0 //Fotocelda para control de iluminacion visible blanca

#define SOIL 1  //Sensor de humedad de la tierra

#define PH 2 //Sensor de ph

#define LM35 3  //Sensor de temperatura del suelo

#define VALVULA_IRRIGACION 7  //Electrovalvula para irrigacion 
#define VALVULA_GOTEO 8  //Electrovalvula para goteo 
#define MOTOPIN 9 //La motobomba 
#define UVLED 13 // FALTA POR IMPLEMENTAR
#define IRLED 12  //LEDs infrarojos en pin 5
#define PLED 11  //LEDs blancos de potencia


//Definición de constantes
const unsigned long TSAMR = 3000; //Periodo de muestreo para almacenar variables
const float SOIL_MADC_AIR = 0; //Valor ADC en el aire para calibrar el sensor de humedad del suelo
const float SOIL_MADC_WATER = 700; //Valor ADC en el agua para calibrar el sensor de humedad del suelo
const float SOIL_MO_AIR = 0; //Valor minimo (seco o al aire) deseado escalado de humedad del suelo (0%)
const float SOIL_MO_WATER = 100;  //Valor maximo (humedo) deseado esacalado de humedad del suelo (100%)
double CONSKP = 0.02, CONSKI = 1, CONSKD = 0.00001; //Constantes proporcional, integral y derivativa para el PID de iluminacion
const float OFFSET = 0.00;  //OFFSET de voltaje para el sensor de ph
const unsigned long MAXTPH = 20;  //Tiempo de sampling del sensor de ph (min 20 ms)
const int NUMREADS = 40;  //Maximo de muestras a medir para la media movil (longitud del vector)
const int TOTAL_VALORES_RASP = 8;


//Definición de variables
float valoresRasp[TOTAL_VALORES_RASP] = {0}; //Arreglo que guardara los valores recibidos desde la raspberry

float luzBlanca = 0,  //Luz blanca (luz visible)
      luzInfrarroja = 0, //Luz infraroja
      luzUltravioleta = 0, //Luz UV
      ph = 0, //Valor del ph
      humedadSuelo = 0, //Valor de humedad del suelo (moisture)
      temperaturaSuelo = 0,  //Temperatura del suelo (medida con el LM35)

      errorIluminacion = 0, //Error de iluminacion
      errorHumedadSuelo = 0, //Error de humedad de la tierra

      //Set points
      spLuzBlanca = 300,  //Set-point (Valor deseado) de iluminacion blanca (valor por defecto en 300 lux)
      spLuzUv = 300,
      spLuzIR = 300,
      spHumedadSuelo = 50;  //Set-point (Valor deseado) de humedad del suelo (valor por defeto en 50%)

unsigned int hrs = 0; //Variable para almacenar las horas (HOUR) del reloj de tiempo real

//Variables para control
unsigned int estadoValvula = 0; //1 = irrigación, 0 = goteo
unsigned int modoManual = 0;
unsigned int motobomba = 0; //Para saber si se prende o se apaga la motobomba

//Variables para media movil y control PID
double spLuz = 600,
       spUv = 600,
       spIR = 600,
       luzEntrada = 0,
       humedadSueloEntrada = 0,
       temperaturaEntrada = 0,
       phEntrada = 0,
       luzInfrarrojaEntrada = 0,
       luzUvEntrada = 0,
       luzSalida = 0,
       luzInfrarrojaSalida = 0,
       luzUvSalida = 0;


//Variables para media movil
int lecturaLuz[NUMREADS] = {0},
                           lecturaHumedadSuelo[NUMREADS] = {0},
                               lecturaTemperatura[NUMREADS] = {0},
                                   lecturaPh[NUMREADS] = {0},
                                       lecturaInfrarroja[NUMREADS] = {0},
                                           lecturaUv[NUMREADS] = {0};

int indiceLuz = 0,
    indiceHumedadSuelo = 0,
    indiceTemperatura = 0,
    indicePh = 0,
    indiceInfrarroja = 0,
    indiceUv = 0;

long totalLuz = 0,
     totalHumedadSuelo = 0,
     totalTemperatura = 0,
     totalPH = 0,
     totalIR = 0,
     totalUv = 0;


//Variables para el tiempo
unsigned long tiempoActual = 0; //Tiempo actual de todo millis()
//tiempoInicial y tiempoRelativo para el tiempo de muestreo de los sensores
unsigned long tiempoInicial = 0;  //Tiempo inicial para procesamiento (conversion) de las variables de sensores
unsigned long tiempoRelativo = 0; //Tiempo relativo para procesamiento (conversion) de las variables de sensores
unsigned long tiempoInicialPh = 0; //Tiempo inicial de pH
unsigned long tiempoRelativoPh = 0;  //Tiempo relativo de muestreo del sensor pH

//Definiciones e inicializaciones para librerías
PID PIDBlanca(&luzEntrada, &luzSalida, &spLuz, CONSKP, CONSKI, CONSKD, DIRECT);
PID PIDIR(&luzInfrarrojaEntrada, &luzInfrarrojaSalida, &spIR, CONSKP, CONSKI, CONSKD, DIRECT);
PID PIDUV(&luzUvEntrada, &luzUvSalida, &spUv, CONSKP, CONSKI, CONSKD, DIRECT);
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

unsigned long smoothUV(long &total, int *readings, int &readIndex) {
  // subtract the last reading
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = SI1145.ReadUV();
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
  luzInfrarrojaEntrada = smoothIR(totalIR, lecturaInfrarroja, indiceInfrarroja);
  luzUvEntrada = smoothUV(totalUv, lecturaUv, indiceUv);
  humedadSueloEntrada = smooth(SOIL, totalHumedadSuelo, lecturaHumedadSuelo, indiceHumedadSuelo);
  temperaturaEntrada = smooth(LM35, totalTemperatura, lecturaTemperatura, indiceTemperatura);
  phEntrada = smoothPH(PH, totalPH, lecturaPh, indicePh);
  tiempoRelativo = tiempoActual - tiempoInicial;
  if (tiempoRelativo >= TSAMR) {
    luzUltravioleta = luzUvEntrada;
    luzInfrarroja = luzInfrarrojaEntrada;
    luzBlanca  = luzEntrada * 5.0 / 1023.0;  //Convierto a voltios la iluminacion blanca
    luzBlanca  = 13.788 * exp(1.3413 *   luzBlanca ); //Convierto o proceso la iluminacion en lumens
    humedadSuelo = flmap(humedadSueloEntrada, SOIL_MADC_AIR, SOIL_MADC_WATER, SOIL_MO_AIR, SOIL_MO_WATER);
    //m = (float)((((humedadSueloEntrada - vtmin) * (mmax - mmin)) / (vtmax - vtmin)) + mmin);
    humedadSuelo = constrain(humedadSuelo, 0, 100);
    //m = analogRead(SOIL);
    temperaturaSuelo = temperaturaEntrada * 500.0 / 1023.0;
    ph = phEntrada * 5.0 * 3.5 / 1023.0 + OFFSET;
    
    String datalog = "";
    datalog += String(luzBlanca, 4);
    datalog += ",";
    datalog += String(ph, 4);
    datalog += ",";
    datalog += String(luzInfrarroja, 4);
    datalog += ",";
    datalog += String(humedadSuelo, 4);
    datalog += ",";
    datalog += String(luzUltravioleta, 4);
    datalog += ",";
    datalog += String(temperaturaSuelo, 4);
    Serial.println(datalog);


    tiempoInicial = millis();
  }
}

void printsens() { //Prints sensors information on Serial monitor
  Serial.print(" Luminous flux: ");
  Serial.print(luzBlanca);
  Serial.print(" *C luzInfrarroja Sensor: ");
  Serial.print(luzInfrarroja);
  Serial.print(" lux luzUltravioleta Sensor: ");
  Serial.print(luzUltravioleta);
  Serial.print(" lux Soil Moisture: ");
  Serial.print(humedadSuelo);
  Serial.print(" % Valve: ");
  Serial.print(motobomba);
  Serial.print(" Soil Temperature: ");
  Serial.print(temperaturaSuelo);
  Serial.print(" ph Sensor: ");
  Serial.println(ph);
}

void lightctrl() {  //Light controller subroutine
  spLuz = constrain(151.04 * log(spLuzBlanca) - 390.08, 0, 1023); //Pasa de lumens a valores analogos
  spIR = constrain(151.04 * log(spLuzIR) - 390.08, 0, 1023);
  spUv = constrain(151.04 * log(spLuzUv) - 390.08, 0, 1023);
  PIDBlanca.Compute(); //Calcula que tanto se debe prender la luz blanca
  PIDIR.Compute(); //Calcula que tanto se debe prender la luz IR
  PIDUV.Compute(); //Calcula que tanto se debe prender la luz UV
  analogWrite(PLED, luzSalida);
  analogWrite(IRLED, luzInfrarrojaSalida);
  analogWrite(UVLED, luzUvSalida);
}

void shumidctrl() { //Soil Humidity Controller
  errorHumedadSuelo = spHumedadSuelo - humedadSuelo;
  if (errorHumedadSuelo < -2) {  //If the soil moisture is above set point (neg error), turn off all valves
    digitalWrite(MOTOPIN, LOW);
    digitalWrite(VALVULA_GOTEO, LOW);
    digitalWrite(VALVULA_IRRIGACION, LOW);

    motobomba = 0;
  }
  else if (errorHumedadSuelo > 2 ) { //else if the soil moisture is below set point (pos error), turn on water valve
    if (estadoValvula) {
      digitalWrite(VALVULA_GOTEO, LOW);
      digitalWrite(VALVULA_IRRIGACION, HIGH);
    } else {
      digitalWrite(VALVULA_GOTEO, HIGH);
      digitalWrite(VALVULA_IRRIGACION, LOW);
    }
    digitalWrite(MOTOPIN, HIGH);
    motobomba = 1;
  }
  else { //if the error is between -10% and 10%, keep the water valve off
    digitalWrite(MOTOPIN, LOW);
    digitalWrite(VALVULA_GOTEO, LOW);
    digitalWrite(VALVULA_IRRIGACION, LOW);
    motobomba = 0;
  }
}

void MeasInitialize() {
  for (unsigned int i = 0; i < 80; i++) {
    luzEntrada = smooth(LDR, totalLuz, lecturaLuz, indiceLuz);
    luzInfrarrojaEntrada = smoothIR(totalIR, lecturaInfrarroja, indiceInfrarroja);
    luzUvEntrada = smoothUV(totalUv, lecturaUv, indiceUv);
    humedadSueloEntrada = smooth(SOIL, totalHumedadSuelo, lecturaHumedadSuelo, indiceHumedadSuelo);
    temperaturaEntrada = smooth(LM35, totalTemperatura, lecturaTemperatura, indiceTemperatura);
    phEntrada = smooth(PH, totalPH, lecturaPh, indicePh);
  }
  luzBlanca  = luzEntrada * 5.0 / 1023.0;
  luzBlanca  = 13.788 * exp(1.3413 * luzBlanca);
  luzInfrarroja = luzInfrarrojaEntrada;
  luzUltravioleta = luzUvEntrada;
  humedadSuelo = flmap(humedadSueloEntrada, SOIL_MADC_AIR, SOIL_MADC_WATER, SOIL_MO_AIR, SOIL_MO_WATER);
  //m = constrain(humedadSueloEntrada * 100.0 / 845.0, 0, 100);
  temperaturaSuelo = temperaturaEntrada * 500.0 / 1023.0;
  ph = phEntrada * 5.0 * 3.5 / 1023.0 + OFFSET;
}

void getValoresRasp() { //Recibe los datos desde la raspberry y los asigna a las variables correspondientes
  if (Serial.available()) {
    String recibido = Serial.readString();
    int r = 0;
    int t = 0;
    for (int i = 0; i < recibido.length(); i++) {
      if (recibido.charAt(i) == ',') {
        String value = recibido.substring(r, i);
        if (value == "True") {
          valoresRasp[t] = 1;
        } else if (value == "False") {
          valoresRasp[t] = 0;
        } else {
          valoresRasp[t] = recibido.substring(r, i).toFloat();
        }

        r = (i + 1);
        t++;
      }
    }
  }

  //Los siguientes condicionales son porque en la parte de control solo podemos garantizar un control hasta el 60 % del valor
  if (valoresRasp[8] <= 600) {
    spLuzBlanca = valoresRasp[8];
  } else {
    spLuzBlanca = 600;
  }
  if (valoresRasp[7] <= 600) {
    spLuzUv = valoresRasp[7];
  } else {
    spLuzUv = 600;
  }
  if (valoresRasp[6] <= 600) {
    spLuzIR = valoresRasp[6];
  } else {
    spLuzIR = 600;
  }
  if (valoresRasp[3] <= 60) {
    spHumedadSuelo = valoresRasp[3];
  } else {
    spHumedadSuelo = 60;
  }
  motobomba = valoresRasp[2];
  estadoValvula = valoresRasp[1];
  modoManual = valoresRasp[0];
  //Aqui va la parte de modificar las variables de los actuadores necesarios.
}

void funcionarManual() {
  if (motobomba) {
    digitalWrite(MOTOPIN, HIGH);
    if (estadoValvula) {
      digitalWrite(VALVULA_IRRIGACION, HIGH);
      digitalWrite(VALVULA_GOTEO, LOW);
    } else {
      digitalWrite(VALVULA_GOTEO, HIGH);
      digitalWrite(VALVULA_IRRIGACION, LOW);
    }
  } else {
    digitalWrite(MOTOPIN, LOW);
    digitalWrite(VALVULA_GOTEO, LOW);
    digitalWrite(VALVULA_IRRIGACION, LOW);
  }
  analogWrite(UVLED, spLuzUv * (255.0 / 1000.0));
  analogWrite(IRLED, spLuzIR * (255.0 / 1000.0));
  analogWrite(PLED, spLuzBlanca * (255.0 / 1000.0));

  delay(1000);
}

void setup() {
  //Pin configuration
  pinMode(VALVULA_IRRIGACION, OUTPUT);
  pinMode(VALVULA_GOTEO, OUTPUT);
  pinMode(PLED, OUTPUT);
  pinMode(IRLED, OUTPUT);
  pinMode(UVLED, OUTPUT);
  pinMode(MOTOPIN, OUTPUT);


  pinMode(LED_BUILTIN, OUTPUT);


  //luzSalida cleaning
  digitalWrite(VALVULA_IRRIGACION, LOW);
  digitalWrite(VALVULA_GOTEO, LOW);
  estadoValvula = 0;
  digitalWrite(PLED, LOW);
  digitalWrite(UVLED, LOW);
  digitalWrite(IRLED, LOW);

  //Communications
  Serial.begin(9600);
  PIDBlanca.SetMode(AUTOMATIC);
  PIDIR.SetMode(AUTOMATIC);
  PIDUV.SetMode(AUTOMATIC);
  while (!SI1145.Begin());
  MeasInitialize();

  tiempoInicial = millis();
  tiempoInicialPh = millis();
}

void loop() {
  tiempoActual = millis();
  leerSensores();
  getValoresRasp();
  if (modoManual) {
    //Manual
    funcionarManual();
  } else {
    //Automatico
    lightctrl();  //Activate light controller
    shumidctrl(); //Activate soil humidity controller
    //analogWrite(5, 200);
  }
}
