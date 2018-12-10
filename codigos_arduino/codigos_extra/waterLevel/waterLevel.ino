/*
   waterLevel: el programa se encarga de medir el nivel de agua que hay en un tanque  alertar los diferentes cambios
*/

//definir entradas y salidas
#define Echo 27//Pin que recibe la señal
#define Trigger 26//Pin que dispara la señal


//constantes
unsigned int tanLleno = 5; //Distancia que tiene que hay entre el agua y el sensor indicando que el tanque esta lleno
unsigned int alerta = 90;// alera de que es tanque esta por vaciarce
unsigned int tanVacio = 100;//Distancia que tiene que hay entre el agua y el sensor indicando que el tanque esta vacio

int ping(int TriggerPin, int EchoPin) {// el metodo ping utiliza el sensor de ultra sonido para saber la distancia que hay entre el censor y algun punto
  long duration, distanceCm;
  digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);

  duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos

  distanceCm = duration * 10 / 292 / 2;  //convertimos a distancia, en cm
  return distanceCm;
}//Fin del metodo ping

void info () {//El metodo info anuncia los diferentes cambios que hay en el nivel del agua
  int cm = ping(Trigger, Echo);
  if (cm >= tanLleno && cm < alerta) {
    Serial.println("EL nivel del agua es aceptable");
    Serial.println(cm);
  }
  if (cm >= alerta && cm < tanVacio) {
    Serial.println("El nivel del agua esta bajo");
    Serial.println(cm);
  }
  if (cm >= tanVacio) {
    Serial.println("El tanque vacio");
    Serial.println(cm);
  }
}//Fin del metodo info

void setup() {
  Serial.begin(9600);
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
}

void loop() {
  info();
  Serial.println("Running");
  delay(1000);
}
