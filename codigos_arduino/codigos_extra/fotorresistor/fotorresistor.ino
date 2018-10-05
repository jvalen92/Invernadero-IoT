
#define foto A0//se define el pin donde esta conectado la fotorresistencia


void leerFoto() {//El metodo leerFoto se encarga de mostrar los valores recolectados por la fotorresistencia
  int valorFoto = analogRead(foto);
  Serial.println("La intensidad de la luz es de:");
  Serial.println(valorFoto);
  delay(1000);
}
void setup() {
  pinMode(foto, INPUT);
  Serial.begin(9600);

}

void loop() {
  leerFoto();
}
