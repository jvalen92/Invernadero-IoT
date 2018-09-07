/************************Pines y variables para el sensor de humedad************************************/
#define SensorPin A2          //pH meter Analog output to Arduino Analog Input 0
#define D_ph 13               //pin de salida digital
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10], temp;

/************************Libreria y pines para el sensor de humedad DHT11************************************/
#include <dht11.h>
dht11 DHT;
#define DHT11_PIN 4

/************************Hardware Related Macros************************************/
#define         MG_PIN                       (A0)     //define which analog input channel you are going to use
#define         BOOL_PIN                     (2)
#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier

/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in 
//normal operation

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
#define         ZERO_POINT_VOLTAGE           (0.220) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTGAE             (0.030) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

/*****************************Globals***********************************************/
float           CO2Curve[3]  =  {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE / (2.602 - 3))};
//two points are taken from the curve.
//with these two points, a line is formed which is
//"approximately equivalent" to the original curve.
//data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280)
//slope = ( reaction voltage ) / (log400 –log1000)



/*****************************  MGRead *********************************************
  Input:   mg_pin - analog channel
  Output:  output of SEN-000007
  Remarks: This function reads the output of SEN-000007
************************************************************************************/
float MGRead(int mg_pin)
{
  int i;
  float v = 0;

  for (i = 0; i < READ_SAMPLE_TIMES; i++) {
    v += analogRead(mg_pin);
    delay(READ_SAMPLE_INTERVAL);
  }
  v = (v / READ_SAMPLE_TIMES) * 5 / 1024 ;
  return v;
}


/*****************************  MQGetPercentage **********************************
  Input:   volts   - SEN-000007 output measured in volts
         pcurve  - pointer to the curve of the target gas
  Output:  ppm of the target gas
  Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
         of the line could be derived if y(MG-811 output) is provided. As it is a
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
         value.
************************************************************************************/
int  MGGetPercentage(float volts, float *pcurve)
{
  if ((volts / DC_GAIN ) >= ZERO_POINT_VOLTAGE) {
    return -1;
  } else {
    return pow(10, ((volts / DC_GAIN) - pcurve[1]) / pcurve[2] + pcurve[0]);
  }
}

void setup()
{
  Serial.begin(9600);                              //UART setup, baudrate = 9600bps
  pinMode(BOOL_PIN, INPUT);                        //set pin to input
  digitalWrite(BOOL_PIN, HIGH);                    //turn on pullup resistors
  Serial.print("MG-811 Demostration\n");

  //sensor de humedad DTH11
  //revisar los prints
  /*Serial.begin(9600);
    Serial.println("DHT TEST PROGRAM ");
    Serial.print("LIBRARY VERSION: ");
    Serial.println(DHT11LIB_VERSION);
    Serial.println();
    Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
  */
  //Sensor de ph
  pinMode(D_ph, OUTPUT);
  Serial.println("PH sensor, Ready");    //Test the serial monitor
}

void getSensor_C02() {
  int percentage;
  float volts;
  /*
    volts = MGRead(MG_PIN);
    Serial.print( "SEN0159:" );
    Serial.print(volts);
    Serial.print( "V           " );
  */
  percentage = MGGetPercentage(volts, CO2Curve);
  Serial.print("CO2:");
  if (percentage == -1) {
    Serial.print( "<400" );
  } else {
    Serial.print(percentage);
  }

  Serial.print( "ppm" );
  Serial.print("\n");
  /*
    if (digitalRead(BOOL_PIN) ) {
      Serial.print( "=====BOOL is HIGH======" );
    } else {
      Serial.print( "=====BOOL is LOW======" );
    }
  */
  Serial.print("\n");

  delay(500);
}
void getSensor_DHT11() {

  int chk;
  Serial.print("DHT11, \t");
  chk = DHT.read(DHT11_PIN);    // READ DATA
  switch (chk) {
    case DHTLIB_OK:
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT: 0
        Serial.print("Time out error,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }
  // DISPLAT DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);

  delay(2000);

}

void getSensor_HumedadSuelo() {
  /*
    # Example code for the moisture sensor
    # Editor     : Lauren
    # Date       : 13.01.2012
    # Version    : 1.0
    # Connect the sensor to the A0(Analog 0) pin on the Arduino board

    # the sensor value description
    # 0  ~300     dry soil
    # 300~700     humid soil
    # 700~950     in water
  */
  double val = (analogRead(A0) * 100) / 950; //porcentaje
  Serial.print("Moisture Sensor Value:");
  Serial.println(analogRead(A0));
  delay(1000);
}
void setSensor_LDR() {
  double constante = 0.00488759;
  double val = analogRead(A0) * constante; //connect grayscale sensor to Analog 0
  double val2 = val * 1200;
  Serial.print(val2);//print the value to serial
  Serial.println(" LUX ");
  delay(500);
}

void getSensor_Ph()
{
  for (int i = 0; i < 10; i++) //Get 10 sample value from the sensor for smooth the value
  {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++) //sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)               //take the average value of 6 center sample
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  phValue = 3.5 * phValue;                  //convert the millivolt into pH value
  Serial.print("    pH:");
  Serial.print(phValue, 2);
  Serial.println(" ");
  digitalWrite(13, HIGH);
  delay(800);
  digitalWrite(13, LOW);

}

void loop()
{
  //getSensor_HumedadSuelo();
  //getSensor_DHT11();
  //getSensor_C02();
  //setSensor_LDR();
  //getSensor_Ph();
}
