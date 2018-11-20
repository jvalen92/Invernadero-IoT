/*
   Convertir en una sola funcion que lee los datos por serial, los subidivide
   y llama a otras funciones que hagan cambios de variables y eso

*/

float valores[7];


void setup() {

  Serial.begin(9600);
}

void loop() {
  Serial.println("inicio");
  getValues();
  Serial.println(valores[0]);
  delay(1000);
  Serial.println(valores[1]);
  delay(1000);
  Serial.println(valores[2]);
  delay(1000);
  Serial.println(valores[3]);
  delay(1000);
  Serial.println(valores[4]);
  delay(1000);
  Serial.println(valores[5]);
  delay(1000);
  Serial.println(valores[6]);
  delay(1000);
}

// Sacado de https://stackoverflow.com/questions/9072320/split-string-into-string-array
// Recibe el string desde raspberry y divide los valores en posiciones de un arreglo
// y los convierte en float.
void getValues() {
  int r = 0;
  int t = 0;

  if (Serial.available()) {
    String string = Serial.readString();
    for (int i = 0; i < string.length(); i++) {
      if (string.charAt(i) == ',') {
        valores[t] = string.substring(r, i).toFloat();
        r = (i + 1);
        t++;
      }
    }
  }
}

