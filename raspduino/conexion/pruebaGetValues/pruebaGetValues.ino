 /*
   Convertir en una sola funcion que lee los datos por serial, los subidivide
   y llama a otras funciones que hagan cambios de variables y eso

*/




void setup() {

  Serial.begin(9600);
}

void loop() {
  if (Serial.available()){
    String cadena= Serial.readString();
    delay(1);
    Serial.print(cadena);
    
  }
}
