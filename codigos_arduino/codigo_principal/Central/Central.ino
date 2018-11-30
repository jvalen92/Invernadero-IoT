/**
   Este código es para el monitoreo y control de los sensores y actuadores del arduino central en el inverdanero iot que está desarrollando el semillero de investigación en sistemas embebidos.
   Este código se basa en el código del profesor David Velasquez, del departamento de ingeniería de sistemas de la universidad Eafit.
   Fecha: Noviembre 2018
   Convenciones:
   - sp = setPoitnt(valor deseado). P.e: spLuzBlanca = valor deseado de luz Blanca
   - Se omite el uso de las preposiciones como "de", "del", "de la", en los nombres de las variables.
     P.e: luzInfrarrojaDeEntrada es en nuestro código luzInfrarrojaEntrada.
*/

//Definción de librerías
#include "DHT.h"
#include <PID_v1.h> //Libreria de PID para control de iluminacion
#include <Wire.h>   //Libreria para comunicarse DS1307 reloj de tiempo real
#include "DS1307.h" //Libreria del reloj de tiempo real (RTC)

//Definición de pines
#define SPCMAX 10     //Setpoint de corriente de temperatura para el maxthermo
#define nivelCo2SEN 2 //Sensor de nivelCo2

#define WCS1800C 6  //Current sensor for central on A6 ?
#define WCS1800P2 0 //Current sensor for plant 2 on A0 ?
#define WCS1800P1 7 //Current sensor for plant 1 on A7 ?

#define DHTPIN 9 //Sensor de temperatura y humedad

#define ENCVENTILADOR_ENTRADA 18 //Hall sensor for Fan In on digital pin D3 ?
#define ENCVENTILADOR_SALIDA 19  //Hall sensor for Fan Out on digital pin D18 ?

#define VENTILADOR_ENTRADA 5
#define VENTILADOR_SALIDA 4

#define WLECHO 6    //Echo para ultrasonido utilizado para medir el nivel del agua
#define WLTRIG 7    //Trigger para ultrasonido utilizado para medir el nivel del agua
#define TEMP_AGUA 3 //Temperatura del agua

#define WTOUT 31 //Water resistor SSR out ?

#define WLINVALVE 26 //Water level in valve in pin D26 ?

//Variables para las librerias y definción de constantes
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

//Definición de constantes
const float DC_GAIN = 8.5; //define the DC gain of amplifier nivelCo2 sensor
//const float ZERO_POINT_VOLTAGE = 0.4329; //define the output of the sensor in volts when the concentration of nivelCo2 is 400PPM
const float ZERO_POINT_VOLTAGE = 0.265; //define the output of the sensor in volts when the concentration of nivelCo2 is 400PPM
const float REACTION_VOLTAGE = 0.059;   //define the “voltage drop” of the sensor when move the sensor from air into 1000ppm nivelCo2
const unsigned long tsamr = 3000;       //Sampling time 3 secs for reading
const float wtadc_min = 50;             //Minimum ADC for temperature
const float wtadc_max = 920;            //Maximum ADC for temperature
const float wto_min = 20;               //Minimum temperature
const float wto_max = 35;               //Maximum temperature
const float nivelAguai_min = 20;        //Minimum water level
const float nivelAguai_max = 41;        //Maximum water level
const float nivelAguao_min = 0;         //Minimum water level
const float nivelAguao_max = 25;        //Maximum water level
const int numReadings = 40;
const int numReadingsnivelCo2 = 5;
const int TOTAL_VALORES_RASP = 5;

/*
  Maxthermo tabla de datos
  Nombre     Pin Arduino
  DIN -       4
  SCLK -      3
  CS -        2
  Vcc -       5v
  Gnd -       0v
  Out -       Salida
*/
//Variabes para el maxthermo
const byte CS = 2;
const byte CLOCK = 3;
const byte DATA = 4;
const byte HALF_CLOCK_PERIOD = 2; //2 uS of clock period

//Definición de variables
float nivelCo2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))}; //Line curve with 2 points
int nivelCo2 = 0;                                                                       //Define co2 as int for nivelCo2 concentration in ppm
float nivelAgua = 0;                                                                    //Define nivelAgua as float for water level in cm
float t = 25;                                                                           //Define t as float for ambient temperature in °C
float tp = 25;                                                                          //Define tp as float for previous ambient temperature in °C
float wt = 0;                                                                           //Define wt as float for water temperature in °C
float h = 30;                                                                           //Define h as float for humidity in %
float hp = 30;                                                                          //Define hp as float for previous humidity in %
float IC = 0;                                                                           //Define IC as float for Central current in Amps
float IP1 = 0;                                                                          //Define IP1 as float for Plant 1 current in Amps
float IP2 = 0;                                                                          //Define IP2 as float for Plant 2 current in Amps
int enivelCo2 = 0;                                                                      //Define et as int for nivelCo2 concentration error
float valoresRasp[TOTAL_VALORES_RASP] = {0};                                            //Arreglo que guardara los valores recibidos desde la raspberry

//Variables para control
unsigned int entradaAire = 0;
unsigned int salidaAire = 0;
unsigned int modoManual = 0;

//Set points (valores deseados)
float spTempAgua = 30;
int spNivelCo2 = 600;

double InputVENTILADOR_ENTRADA = 0, InputVENTILADOR_SALIDA = 0, InputiempoRelativoNivelCo2 = 0, InputiempoRelativoNivelAgua = 0, Inputt = 0, Inputwt = 0, Inputh = 0, InputIC = 0, InputIP1 = 0, InputIP2 = 0;
unsigned int WLINSTATE = 0, WTOUTSTATE = 0, VENTILADOR_ENTRADASTATE = 0, VENTILADOR_SALIDASTATE = 0;

//Variables para la media movil
int readingsnivelCo2[numReadingsnivelCo2] = {0}, readingsnivelAgua[numReadings] = {0}, readingst[numReadings] = {0}, readingswt[numReadings] = {0}, readingsh[numReadings] = {0}, readingsIC[numReadings] = {0}, readingsIP1[numReadings] = {0}, readingsIP2[numReadings] = {0};

int readIndexnivelCo2 = 0,
    readIndexnivelAgua = 0,
    readIndext = 0,
    readIndexwt = 0,
    readIndexh = 0,
    readIndexIC = 0,
    readIndexIP1 = 0,
    readIndexIP2 = 0;

long totalnivelCo2 = 0, totalnivelAgua = 0, totalt = 0, totalwt = 0, totalh = 0, totalIC = 0, totalIP1 = 0, totalIP2 = 0;
char readbuffer[5] = {'\0'};

//Variables para el tiempo
unsigned long tiempoActual = 0;
//tiempoInicial y tiempoRelativo para el tiempo de muestreo de los sensores
unsigned long tiempoInicial = 0;
unsigned long tiempoRelativo = 0;

unsigned long tiempoInicialNivelCo2 = 0;
unsigned long tiempoRelativoNivelCo2 = 0;

unsigned long tiempoInicialNivelAgua = 0;
unsigned long tiempoRelativoNivelAgua = 0;

//Subroutines and functions
float flmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return constrain((float)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min), out_min, out_max);
}

unsigned int smooth(int meas, long &total, int *readings, int &readIndex, char type)
{
  // subtract the last reading
  total = total - readings[readIndex];
  // read from the sensor:
  if (type == 'A')
  {
    readings[readIndex] = analogRead(meas);
  }
  else if (type == 'M')
  {
    readings[readIndex] = meas;
  }
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings)
  {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  return total / numReadings;
}

unsigned int smoothnivelCo2(int pin, long &tot, int *reads, int &readInd)
{
  // subtract the last reading
  tiempoRelativoNivelCo2 = tiempoActual - tiempoInicialNivelCo2;
  if (tiempoRelativoNivelCo2 >= 20)
  {
    tot = tot - reads[readInd];
    // read from the sensor:
    reads[readInd] = analogRead(pin);
    // add the reading to the total:
    tot = tot + reads[readInd];
    // advance to the next position in the array:
    readInd = readInd + 1;

    // if we're at the end of the array...
    if (readInd >= numReadingsnivelCo2)
    {
      // ...wrap around to the beginning:
      readInd = 0;
    }
    tiempoInicialNivelCo2 = millis();
  }
  // calculate the average:
  return tot / numReadingsnivelCo2;
}
//Sensor ultrasonido
float sultra(byte echop, byte trigp)
{
  tiempoRelativoNivelAgua = tiempoActual - tiempoInicialNivelAgua;
  if (tiempoRelativoNivelAgua >= 50)
  {
    digitalWrite(trigp, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigp, LOW);
    tiempoInicialNivelAgua = millis();
    return pulseIn(echop, HIGH) / 58.0;
  }
  return InputiempoRelativoNivelAgua;
}

int readnivelCo2(double ADCnivelCo2, float *pcurve)
{
  float volts = ADCnivelCo2 * 5.0 / 1023.0; //Convert nivelCo2 ADC to volts
  if ((volts / DC_GAIN) >= ZERO_POINT_VOLTAGE)
  {
    return -1;
  }
  else
  {
    return pow(10, ((volts / DC_GAIN) - pcurve[1]) / pcurve[2] + pcurve[0]);
  }
}

void readsens()
{ //Read sensors information and store it in variables
  InputiempoRelativoNivelCo2 = smoothnivelCo2(nivelCo2SEN, totalnivelCo2, readingsnivelCo2, readIndexnivelCo2);
  //InputiempoRelativoNivelAgua = smooth(sultra(WLECHO, WLTRIG), totalnivelAgua, readingsnivelAgua, readIndexnivelAgua, 'M');
  InputiempoRelativoNivelAgua = sultra(WLECHO, WLTRIG);
  //Inputt = smooth(WLPIN, totall, readingsl, readIndexl);
  Inputwt = smooth(TEMP_AGUA, totalwt, readingswt, readIndexwt, 'A');
  InputIC = smooth(WCS1800C, totalIC, readingsIC, readIndexIC, 'A');
  InputIP1 = smooth(WCS1800P1, totalIP1, readingsIP1, readIndexIP1, 'A');
  InputIP2 = smooth(WCS1800P2, totalIP2, readingsIP2, readIndexIP2, 'A');
  tiempoRelativo = tiempoActual - tiempoInicial;
  if (tiempoRelativo >= tsamr)
  {
    if (digitalRead(WTOUT) == HIGH)
    {
      WTOUTSTATE = 1;
    }
    else
    {
      WTOUTSTATE = 0;
    }
    nivelCo2 = constrain(readnivelCo2(InputiempoRelativoNivelCo2, nivelCo2Curve), 400, 10000);
    nivelAgua = flmap(InputiempoRelativoNivelAgua, nivelAguai_max, nivelAguai_min, nivelAguao_min, nivelAguao_max);
    t = dht.readTemperature();
    wt = flmap(Inputwt, wtadc_min, wtadc_max, wto_min, wto_max);
    h = dht.readHumidity();
    IC = constrain(0.0952 * InputIC - 49.732, 0, 30);
    IP1 = constrain(0.0952 * InputIP1 - 49.732, 0, 30);
    IP2 = constrain(0.088 * InputIP2 - 44.972, 0, 30);

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t))
    {
      t = tp;
      //Serial.println("Failed to read from DHT");
    }
    else if (isnan(h))
    {
      h = hp;
    }
    else
    {
      tp = t;
      hp = h;
    }
    String datalog = ""; // Recoge las lecturas finales de los sensores y las envia por serial a la raspberry
    datalog += String(nivelCo2, DEC);
    datalog += ",";
    datalog += String(wt, 4);
    datalog += ",";
    datalog += String(h, 4);
    datalog += ",";
    datalog += String(nivelAgua, 4);
    datalog += ",";
    datalog += String(t, 4);
    Serial.println(datalog);

    tiempoInicial = millis();
  }
}

void printsens()
{ //Prints sensors information on Serial monitor
  Serial.print(" Water Level: ");
  Serial.print(nivelAgua);
  Serial.print(" cm Ambient Temperature: ");
  Serial.print(t);
  Serial.print(" *C Water Temperature: ");
  Serial.print(wt);
  Serial.print(" *C Relative Humidity: ");
  Serial.print(h);
  Serial.print(" % WTOUTSTATE: ");
  Serial.print(WTOUTSTATE);
  Serial.print(" nivelCo2: ");
  Serial.print(nivelCo2);
  Serial.print(" ppm Fan In state: ");
  Serial.print(VENTILADOR_ENTRADASTATE);
  Serial.print(" Fan Out state: ");
  Serial.print(VENTILADOR_SALIDASTATE);
  Serial.print(" Central Current: ");
  Serial.print(IC);
  Serial.print(" A Plant 1 Current: ");
  Serial.print(IP1);
  Serial.print(" A Plant 2 Current: ");
  Serial.print(IP2);
  Serial.println(" A");
}

void nivelCo2ctrl()
{ //nivelCo2 Controller
  enivelCo2 = spNivelCo2 - nivelCo2;
  if (enivelCo2 < -20)
  { //If the nivelCo2 error is less than -100 ppm, turn off VENTILADOR_ENTRADA turn on VENTILADOR_SALIDA
    digitalWrite(VENTILADOR_ENTRADA, LOW);
    digitalWrite(VENTILADOR_SALIDA, HIGH);
    VENTILADOR_ENTRADASTATE = 0;
    VENTILADOR_SALIDASTATE = 1;
  }
  else if (enivelCo2 > 100)
  { //else If the nivelCo2 error is greater than 100 ppm, turn on VENTILADOR_ENTRADA turn off VENTILADOR_SALIDA
    digitalWrite(VENTILADOR_ENTRADA, HIGH);
    digitalWrite(VENTILADOR_SALIDA, LOW);
    VENTILADOR_ENTRADASTATE = 1;
    VENTILADOR_SALIDASTATE = 0;
  }
  else
  { //In other case, keep the fans off
    digitalWrite(VENTILADOR_ENTRADA, LOW);
    digitalWrite(VENTILADOR_SALIDA, LOW);
    VENTILADOR_ENTRADASTATE = 0;
    VENTILADOR_SALIDASTATE = 0;
  }
}

void getValoresRasp()
{ //Recibe los datos desde la raspberry y los asigna a las variables correspondientes
  if (Serial.available())
  {
    String recibido = Serial.readString();
    int r = 0;
    int t = 0;
    for (int i = 0; i < recibido.length(); i++)
    {
      if (recibido.charAt(i) == ',')
      {
        valoresRasp[t] = recibido.substring(r, i).toFloat();
        r = (i + 1);
        t++;
      }
    }
  }
  //Los siguientes condicionales son porque en la parte de control solo podemos garantizar un control hasta el 60 % del valor
  if (valoresRasp[4] <= 6000)
  {
    spNivelCo2 = valoresRasp[4];
  }
  else
  {
    spNivelCo2 = 6000;
  }
  if (valoresRasp[1] <= 45)
  {
    //No hay control
    spTempAgua = valoresRasp[1];
  }
  else
  {
    spTempAgua = 45;
  }

  modoManual = valoresRasp[2];
  entradaAire = valoresRasp[0];
  salidaAire = valoresRasp[3];
}

void funcionarManual() {
  if (entradaAire) {
    digitalWrite(VENTILADOR_ENTRADA, HIGH);
  } else {
    digitalWrite(VENTILADOR_ENTRADA, LOW);
  }

  if (salidaAire) {
    digitalWrite(VENTILADOR_SALIDA, HIGH);
  } else {
    digitalWrite(VENTILADOR_SALIDA, LOW);
  }

  tempAguactrl();

  delay(1000);

}

void MeasInitialize()
{
  for (unsigned int i = 0; i < 80; i++)
  {
    constrain(readnivelCo2(InputiempoRelativoNivelCo2, nivelCo2Curve), 400, 10000);
    InputiempoRelativoNivelAgua = sultra(WLECHO, WLTRIG);
    Inputwt = smooth(TEMP_AGUA, totalwt, readingswt, readIndexwt, 'A');
    InputIC = smooth(WCS1800C, totalIC, readingsIC, readIndexIC, 'A');
    InputIP1 = smooth(WCS1800P1, totalIP1, readingsIP1, readIndexIP1, 'A');
    InputIP2 = smooth(WCS1800P2, totalIP2, readingsIP2, readIndexIP2, 'A');
  }
  nivelCo2 = constrain(readnivelCo2(InputiempoRelativoNivelCo2, nivelCo2Curve), 400, 10000);
  nivelAgua = flmap(InputiempoRelativoNivelAgua, nivelAguai_max, nivelAguai_min, nivelAguao_min, nivelAguao_max);
  wt = flmap(Inputwt, wtadc_min, wtadc_max, wto_min, wto_max);
  IC = constrain(0.0952 * InputIC - 49.732, 0, 30);
  IP1 = constrain(0.0952 * InputIP1 - 49.732, 0, 30);
  IP2 = constrain(0.088 * InputIP2 - 44.972, 0, 30);
}
// Método para establecer el voltaje al maxthermo (writeValue)
void escribirValorMaxthermo(uint16_t value)
{
  digitalWrite(CS, LOW); //start of 12 bit data sequence
  digitalWrite(CLOCK, LOW);
  value = value << 2;
  for (int i = 11; i >= 0; i--) //send the 12 bit sample data
  {
    digitalWrite(DATA, (value & (1 << i)) >> i); //DATA ready
    delayMicroseconds(HALF_CLOCK_PERIOD);
    digitalWrite(CLOCK, HIGH); //DAC get DATA at positive edge
    delayMicroseconds(HALF_CLOCK_PERIOD);
    digitalWrite(CLOCK, LOW);
  }
  digitalWrite(CS, HIGH); //end 12 bit data sequence
}

void setup()
{
  //Configuración de pines
  pinMode(WLINVALVE, OUTPUT);
  pinMode(WLECHO, INPUT);
  pinMode(WLTRIG, OUTPUT);
  pinMode(WTOUT, INPUT);
  pinMode(VENTILADOR_ENTRADA, OUTPUT);
  pinMode(VENTILADOR_SALIDA, OUTPUT);
  pinMode(SPCMAX, OUTPUT);

  //Output cleaning
  digitalWrite(WLTRIG, LOW);
  digitalWrite(WLINVALVE, LOW);
  digitalWrite(VENTILADOR_ENTRADA, LOW);
  digitalWrite(VENTILADOR_SALIDA, LOW);
  WLINSTATE = 0;
  VENTILADOR_ENTRADASTATE = 0;
  VENTILADOR_SALIDASTATE = 0;

  //DAC
  pinMode(DATA, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  digitalWrite(DATA, LOW);
  digitalWrite(CLOCK, LOW);

  //Communications
  Serial.begin(9600);

  dht.begin();
  MeasInitialize();

  tiempoInicial = millis();
  tiempoInicialNivelCo2 = millis();
  tiempoInicialNivelAgua = millis();
}

void tempAguactrl() {
  //El control lo hace el propio maxthermo
  escribirValorMaxthermo(spTempAgua * (1023.0 / 50.0));
}

void loop()
{
  tiempoActual = millis();
  readsens();          //Read sensors
  getValoresRasp();
  if (modoManual) {
    //Manual
    funcionarManual();
  } else {
    //Automatico
    nivelCo2ctrl();
    tempAguactrl();
  }
  analogWrite(SPCMAX, 0);
}
