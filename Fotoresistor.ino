void setup()
{
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
}
void loop()
{
      
      double constante=0.00488759;
      double val =analogRead(0)*constante;   //connect grayscale sensor to Analog 0
      double val2=val*1200;
      Serial.print(val2);//print the value to serial        
      Serial.println(" LUX ");
      delay(100);
}
