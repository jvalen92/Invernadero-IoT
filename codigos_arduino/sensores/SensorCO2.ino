const int MQ_PIN = 2;
const int MQ_DELAY = 2000;
 
void setup()
{
  Serial.begin(9600);
}
 
 
void loop()
{
  bool state= digitalRead(MQ_PIN);
 
  if (!state)
  {
    Serial.println("Deteccion");
  }
  else
  {
    Serial.println("No detectado");
  }
  delay(MQ_DELAY);
}


/* Lectura Analoga

const int MQ_PIN = A0;
const int MQ_DELAY = 2000;
 
void setup()
{
  Serial.begin(9600);
}
 
void loop() 
{
  int raw_adc = analogRead(MQ_PIN);
  float value_adc = raw_adc * (5.0 / 1023.0);
 
  Serial.print("Raw:");
  Serial.print(raw_adc);
  Serial.print("    Tension:");
  Serial.println(value_adc);
 
  delay(MQ_DELAY);
}
*/