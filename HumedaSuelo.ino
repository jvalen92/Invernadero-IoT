
const int sensorPin = A0;
 
void setup() {
   Serial.begin(9600);
}
 
void loop() 
{
   int humedad = analogRead(sensorPin);
   Serial.print(humedad);
  
   if(humedad < 500)
   {
      Serial.println("Encendido");  
      //hacer las acciones necesarias
   }
   delay(1000);
}

//

/* 
Si estamos empleando la señal digital, empleamos una entrada digital para leer el estado.
 En el ejemplo mostramos un mensaje por la pantalla, 
 pero igualmente en un caso real ejecutaríamos las acciones oportunas.
 
const int sensorPin = 10;
 
void setup()
{
   Serial.begin(9600);
   pinMode(sensorPin, INPUT);
}
 
void loop()
{
   int humedad = digitalRead(sensorPin);
 
   //mandar mensaje a puerto serie en función del valor leido
   if (humedad == HIGH)
   {
      Serial.println("Encendido");   
      //aquí se ejecutarían las acciones
   }
   delay(1000);
}
*/