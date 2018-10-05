//el codigo se encarga de mostrar la fecha y la hora actual por medio del RTC

#include <Wire.h>
#include "RTClib.h"

// RTC_DS1307 rtc;
RTC_DS3231 rtc;

String daysOfTheWeek[7] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };//se crea un arreglo para almacenar los dias de la semana
String monthsNames[12] = { "Enero", "Febrero", "Marzo", "Abril", "Mayo",  "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre" }; //se crea un arreglo para almacenar los meses

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
}

void printDate(DateTime date) {//este metodo sse encarga de imprimir hora, dia, mes y año
  Serial.print(date.year(), DEC);
  Serial.print('/');
  Serial.print(date.month(), DEC);
  Serial.print('/');
  Serial.print(date.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[date.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(date.hour(), DEC);
  Serial.print(':');
  Serial.print(date.minute(), DEC);
  Serial.print(':');
  Serial.print(date.second(), DEC);
  Serial.println();
}

void loop() {
  // Obtener fecha actual y mostrar por Serial
  DateTime now = rtc.now();
  printDate(now);

  delay(3000);
}
